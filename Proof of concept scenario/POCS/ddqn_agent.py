#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import random
import gym
import numpy as np
from collections import deque
from keras.models import Sequential
from keras.layers import Dense
from keras.optimizers import Adam
from keras import backend as K

import tensorflow as tf
import pdb
import tensorflow.contrib.slim as slim
import matplotlib as mpl                  #for plotting
import matplotlib.pyplot as plt
from tensorflow import keras
from ns3gym import ns3env                 #for interfacing NS-3 with RL
from keras.layers import Dense, Dropout, Activation
import csv
EPISODES = 200
port=1122 # Should be consistent with NS-3 simulation port
stepTime=0.2
startSim=0
Usersnum=41
seed=3
simArgs = {}
debug=True
step_CIO=3 # CIO value step in the discrete set {-6, -3, 0, 3, 6}
Result_row=[]
Rew_ActIndx=[]
MCS2CQI=np.array([1,2,3,3,3,4,4,5,5,6,6,6,7,7,8,8,8,9,9,9,10,10,10,11,11,12,12,13,14])# To map MCS indexes to CQI

max_env_steps = 50  #Maximum number of steps in every episode
class DDQNAgent:
    def __init__(self, state_size, action_size):
        self.state_size = state_size
        self.action_size = action_size
        self.memory = deque(maxlen=20000)
        self.gamma = 0.95    # Discount rate
        self.epsilon = 1.0  # At the begining
        self.epsilon_min = 0.01
        self.epsilon_decay = 0.9995
        self.learning_rate = 0.001
        self.Prev_Mean=0 #initial mean of the targets (for target normalization)
        self.Prev_std=1 #initial std of the targets (for target normalization)
        self.model = self._build_model()
        self.target_model = self._build_model()
        self.update_target_model()
        #self.Find_Best_Action(self, next_state,a_level,a_num)


    def _huber_loss(self, y_true, y_pred, clip_delta=1.0):
        error = y_true - y_pred
        cond  = K.abs(error) <= clip_delta

        squared_loss = 0.5 * K.square(error)
        quadratic_loss = 0.5 * K.square(clip_delta) + clip_delta * (K.abs(error) - clip_delta)

        return K.mean(tf.where(cond, squared_loss, quadratic_loss))

    def _build_model(self):
        # Neural Net for Deep-Q learning Model
        model = Sequential()
        model.add(Dense(24, input_dim=self.state_size, activation='relu'))
        model.add(Dense(24, activation='relu'))
        model.add(Dense(self.action_size, activation='linear'))
        model.compile(loss=self._huber_loss,
                      optimizer=Adam(lr=self.learning_rate))
        return model

    def update_target_model(self):
        # copy weights from the CIO selection network to target network
        self.target_model.set_weights(self.model.get_weights())
   
    def remember(self, state, action, reward, next_state, done):
        self.memory.append((state, action, reward, next_state, done))

    def act(self, state):
        act_values = self.model.predict(state)
        #print("Predicted action for this state is: {}".format(np.argmax(act_values[0])))
        if np.random.rand() <= self.epsilon:
            return random.randrange(self.action_size)
        return np.argmax(act_values[0])  # returns action

    def replay(self, batch_size):
        
        minibatch = random.sample(self.memory, batch_size)

        target_A=[]#for batch level target normalization

        for state, action, reward, next_state, done in minibatch:#calculate the target array
            a = self.model.predict(next_state)[0]
            t = self.target_model.predict(next_state)[0]
            b =t[np.argmax(a)]#needs de_normalization
            b *=self.Prev_std
            b =+self.Prev_Mean
            target_A.append(reward + self.gamma * b)
        
        mean_MB= np.mean(np.asarray(target_A), axis=0) #mean of the targets in this mini-batch
        std_MB= np.std(np.asarray(target_A), axis=0) # std of the targets in this mini-batch

        for state, action, reward, next_state, done in minibatch:
            target = self.model.predict(state)
            if done:
                tg = reward
            else:
                a = self.model.predict(next_state)[0]
                t = self.target_model.predict(next_state)[0] # DDQN feature

                b =t[np.argmax(a)]#needs de_normalization
                b *=self.Prev_std # denormalize the future reward by the mean and std of the previous mini-batch
                b =+self.Prev_Mean # denormalized future reward
                tg = reward + self.gamma * b  # 
                tg -= mean_MB
                tg /= std_MB #normalized target
                self.Prev_std = std_MB
                self.Prev_Mean= mean_MB
            target[0][action]=tg
            history = self.model.fit(state, target, epochs=1, verbose=0)# training
        loss = history.history['loss'][0]
        if self.epsilon > self.epsilon_min: #To balance the exploration and exploitation
            self.epsilon *= self.epsilon_decay
        return loss


    def load(self, name):
        self.model.load_weights(name)

    def save(self, name):
        self.model.save_weights(name)


if __name__ == "__main__":
    env = ns3env.Ns3Env(port=port, stepTime=stepTime, startSim=startSim, simSeed=seed, simArgs=simArgs, debug=debug)
    env._max_episode_steps = max_env_steps
    ac_space = env.action_space               #Getting the action space
    state_size = 12
    a_level=int(ac_space.high[0]) # CIO levels          
    a_num=int(ac_space.shape[0]) # number of required relative CIOs
    action_size = a_level**a_num
    agent = DDQNAgent(state_size, action_size)
    #agent.load("./LTE-DDQN.h5") 
    done = False
    batch_size = 32
    rew_history = []
    reward1_list= []
    reward2_list= []
    reward3_list= []
    AVG_CQI_list=[]


    for e in range(EPISODES):
        state = env.reset()
        state1 = np.reshape(state['rbUtil'], [3, 1])
        state2 = np.reshape(state['dlThroughput'],[3,1])
        state2_norm=state2/18 # normalize with the maximium expected throughput (min-max normalization)
        state3 = np.reshape(state['UserCount'], [3, 1])
        state3_norm=state3/Usersnum # normalize with the total number of users
        MCS_t=np.array(state['MCSPen'])
        state4=np.sum(MCS_t[:,:10], axis=1)
        state4=np.reshape(state4,[3, 1])
        # To report other rewards
        R_rewards = np.reshape(state['rewards'], [3, 1])
        R_rewards =[j for sub in R_rewards for j in sub]
        
        state  = np.concatenate((state1,state2_norm,state3_norm,state4),axis=None)
        #print("RB utilization:{}".format(np.transpose(state1)))
        #print("Cell throuhput:{}".format(np.transpose(state2)))
        #print("Users count:{}".format(np.transpose(state3)))
        #print("Edge user ratio:{}".format(np.transpose(state4)))
        #print("reward functions:{}".format(np.transpose(R_rewards)))

        state = np.reshape(state, [1, state_size])###
        rewardsum = 0                         #Accumulator for the reward
        cqisum=0
        R_reward1=0
        R_reward2=0
        R_reward3=0
        for time in range(max_env_steps):
            print("*******************************")

            print("episode: {}/{}, step: {}".format(e+1, EPISODES, time))
            action_index = agent.act(state)
            action=np.base_repr(action_index+int(a_level)**int(a_num),base=int(a_level))[-a_num:]# decoding the action index to the action vector
            action=[int(action[s]) for s in range(len(action))]
            action=np.concatenate((np.zeros(a_num-len(action)),action),axis=None)
            action=[step_CIO*(x-np.floor(a_level/2)) for x in action]#action vector
            next_state, reward, done, _ = env.step(action)
            if next_state is None:#To avoid crashing the simulation if the handover failiure occured in NS-3 simulation
                OK=0 # Handover failiure occured
                EPISODES=EPISODES+1
                break
            OK=1 # No handover failiure occured

            state1 = np.reshape(next_state['rbUtil'], [3, 1])
            state2 = np.reshape(next_state['dlThroughput'],[3, 1])
            state2_norm=state2/18
            state3 = np.reshape(next_state['UserCount'], [3, 1])
            state3_norm=state3/Usersnum
            MCS_t=np.array(next_state['MCSPen'])
            state4=np.sum(MCS_t[:,:10], axis=1)
            state4=np.reshape(state4,[3, 1])
            # To report other reward functions
            R_rewards = np.reshape(next_state['rewards'], [3, 1])
            R_rewards =[j for sub in R_rewards for j in sub]

	    # Map MCS to CQI
            xx=state3 [0]
            MCS_t[0,:] *= xx/Usersnum
            xx=state3 [1]
            MCS_t[1,:] *= xx/Usersnum
            xx=state3 [2]
            MCS_t[2,:] *= xx/Usersnum
            AVGMCS= [sum(x) for x in zip(*MCS_t)]
            AVGMCS=np.reshape(AVGMCS,[1, 29])
            AVG_CQI=np.sum(AVGMCS*MCS2CQI)
            #print("AVG_CQI:{}".format((AVG_CQI)))
            
            next_state  = np.concatenate((state1,state2_norm,state3_norm,state4),axis=None)
            next_state = np.reshape(next_state, [1, state_size])
            agent.remember(state, action_index, reward, next_state, done) # Add to the experience buffer
            state = next_state

            print("Step reward:{}".format(reward))
            print("current action:{}".format(action))
            #print("RB utilization:{}".format(np.transpose(state1)))
            #print("Cell throuhput:{}".format(np.transpose(state2)))
            #print("Users count:{}".format(np.transpose(state3)))
            #print("Edge user ratio:{}".format(np.transpose(state4)))

            R_reward1+= R_rewards[0]
            R_reward2+= R_rewards[1]
            R_reward3+= R_rewards[2]
            #print("reward function 1:{}".format(R_reward1))
            #print("reward function 2:{}".format(R_reward2))
            #print("reward function 3:{}".format(R_reward3))
            rewardsum += reward
            cqisum+=AVG_CQI
            if len(agent.memory) > batch_size:
                loss = agent.replay(batch_size)
                # Logging training loss every 10 timesteps
                if time % 10 == 0:
                    print("loss: {:.4f}"
                        .format(loss))   
        if OK==1: # The episode completed without any NS-3 error
            rew_history.append(rewardsum)
            reward1_list.append(R_reward1)
            reward2_list.append(R_reward2)
            reward3_list.append(R_reward3)
            AVG_CQI_list.append(cqisum)
            agent.update_target_model()
            if (e+1) % 10 == 0:
                agent.save("./LTE-DDQN.h5") # Save the model

                Result_row=[]
                with open('Rewards_' + 'DDQN' + '.csv', 'w', newline='') as rewardcsv:
                    results_writer = csv.writer(rewardcsv, delimiter=';', quotechar=';', quoting=csv.QUOTE_MINIMAL)
                    Result_row.clear()
                    Result_row=Result_row+reward1_list
                    results_writer.writerow(Result_row)
                    Result_row.clear()
                    Result_row=Result_row+reward2_list
                    results_writer.writerow(Result_row)
                    Result_row.clear()
                    Result_row=Result_row+reward3_list
                    results_writer.writerow(Result_row)
                    Result_row.clear()
                    Result_row=Result_row+AVG_CQI_list
                    results_writer.writerow(Result_row) 
                rewardcsv.close()
            
    reward1_list[:] = [x / max_env_steps for x in reward1_list]
    reward2_list[:] = [x / max_env_steps for x in reward2_list]
    reward3_list[:] = [x / max_env_steps for x in reward3_list]
    AVG_CQI_list[:] = [x / max_env_steps for x in AVG_CQI_list]

    fig1, ax = plt.subplots()
    ln1, = plt.plot(reward1_list, label='DDQN')
    legend = ax.legend(loc='upper left', shadow=True, fontsize='x-small')
    plt.xlabel("Episode")
    plt.ylabel("Average overall throughput")

    fig1, ax = plt.subplots()
    ln1, = plt.plot(reward2_list, label='DDQN')
    legend = ax.legend(loc='upper left', shadow=True, fontsize='x-small')
    plt.xlabel("Episode")
    plt.ylabel("Average Deviation")
    
    
    fig1, ax = plt.subplots()
    ln1, = plt.plot(reward3_list, label='DDQN')
    legend = ax.legend(loc='upper left', shadow=True, fontsize='x-small')
    plt.xlabel("Episode")
    plt.ylabel("Precentage of Non-blocked Users")
    
    
    fig1, ax = plt.subplots()
    ln1, = plt.plot(AVG_CQI_list, label='DDQN')
    legend = ax.legend(loc='upper left', shadow=True, fontsize='x-small')
    plt.xlabel("Episode")
    plt.ylabel("Average CQI")

    plt.show()

