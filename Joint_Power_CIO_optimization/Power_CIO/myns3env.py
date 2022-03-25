import numpy as np
import gym
from gym import spaces
from ns3gym import ns3env
import csv
import time

reward1=[]
reward2=[]
reward3=[]
COI=[]
ActivUes=[]
avg_reward1=[]
avg_reward2=[]
avg_reward3=[]
avg_COI=[]
MCS2CQI=np.array([1,2,3,3,3,4,4,5,5,6,6,6,7,7,8,8,8,9,9,9,10,10,10,11,11,12,12,13,14])
class myns3env(gym.Env):
  """
  Custom Environment that follows gym interface.
  This is a simple env where the agent must learn to go always left.
  """


  def __init__(self,):
    super(myns3env, self).__init__()
    port=2221
    simTime= 45
    stepTime=0.2
    startSim=0
    seed=3
    simArgs = {"--duration": simTime,}
    debug=True
    self.max_env_steps = 250
    self.env = ns3env.Ns3Env(port=port, stepTime=stepTime, startSim=startSim, simSeed=seed, simArgs=simArgs, debug=debug)
    self.env._max_episode_steps = self.max_env_steps
    self.time_var="t_{}".format(int(time.time()))

    self.Cell_num=6
    self.max_throu=30
    self.Users=40
    self.state_dim = self.Cell_num*4
    print("state_dim:{}".format(self.state_dim ))
    self.action_dim =  self.env.action_space.shape[0]
    self.CIO_action_bound =  6
    self.power_action_bound =  6#self.env.action_space.high

    # Define action and observation space
    # They must be gym.spaces objects
    # Example when using discrete actions, we have two: left and right

    self.action_space = spaces.Box(low=-1, high=1,
                                        shape=(self.action_dim,), dtype=np.float32)
    # The observation will be the coordinate of the agent
    # this can be described both by Discrete and Box space
    self.observation_space = spaces.Box(low=0, high=self.Users,
                                        shape=(self.state_dim,), dtype=np.float32)
    #print("action_space :", self.action_space)
    
  def reset(self):
    """
    Important: the observation must be a numpy array :return: (np.array)
    """
    print("reset                       reset                       reset                       reset                       reset                       ")
    state = self.env.reset()
    if state is None:#To avoid crashing the simulation if the handover failiure occured in NS-3 simulation
        state=[0] * self.state_dim
        return np.array(state)
    state1 = np.reshape(state['rbUtil'], [self.Cell_num, 1])#Reshape the matrix
    state2 = np.reshape(state['dlThroughput'],[self.Cell_num,1])
    state2_norm=state2/self.max_throu
    state3 = np.reshape(state['UserCount'], [self.Cell_num, 1])#Reshape the matrix
    state3_norm=state3/self.Users
    MCS_t=np.array(state['MCSPen'])
    state4=np.sum(MCS_t[:,:10], axis=1)
    state4=np.reshape(state4,[self.Cell_num,1])
    # To report other rewards
    R_rewards = np.reshape(state['rewards'], [3, 1])#Reshape the matrix
    R_rewards =[j for sub in R_rewards for j in sub]

    state  = np.concatenate((state1,state2_norm,state3_norm,state4),axis=None)
    #print("RB utilization:{}".format(np.transpose(state1)))
    #print("Cell throuhput:{}".format(np.transpose(state2)))
    #print("Users count:{}".format(np.transpose(state3)))
    #print("Edge user ratio:{}".format(np.transpose(state4)))
    #print("rewards:{}".format(np.transpose(R_rewards)))

    state = np.reshape(state, [self.state_dim,])###

    #print(" state.shape:{}".format( state.shape))
    #print("self.observation_space.shape:{}".format(self.observation_space.shape))
    #if (    state.shape ==self.observation_space.shape and np.all(state >= self.observation_space.low) and np.all(state <= self.observation_space.high)):
        #print("ok")
    #print("np.array(state):{}".format(np.array(state)))
    return np.array(state)

  def step(self, action):
    #print("self.CIO_action_bound:{}".format(self.CIO_action_bound))
    #print("self.power_action_bound:{}".format(self.power_action_bound))
    action1=action[:11]*(self.CIO_action_bound)
    action2=action[11:]*(self.power_action_bound)
    #print("action:{}".format(len(np.array(action))))
    #print("action1:{}".format(len(np.array(action1))))
    #print("action2:{}".format(len(np.array(action2))))
    
    #print("action:{}".format(action))
    #print("action1:{}".format(action1))
    #print("action2:{}".format(action2))
    
    action  = np.concatenate((action1,action2),axis=None)
    next_state, reward, done, info = self.env.step(action)
    global reward1
    global reward2
    global reward3
    global reward3
    global COI
    global ActivUes

    if next_state is None:#To avoid crashing the simulation if the handover failiure occured in NS-3 simulation
        
        if len(reward1) % self.max_env_steps != 0:
            print("Interrupted episode,  after {} steps".format((len(reward1) % self.max_env_steps)))
            print("len(reward1):{}".format(len(reward1)))
            for e in range(len(reward1) % self.max_env_steps): 
              reward1.pop()
              
        if len(reward2) % self.max_env_steps != 0:
            print("len(reward2):{}".format(len(reward2)))
            for e in range(len(reward2) % self.max_env_steps): 
              reward2.pop()

        if len(reward3) % self.max_env_steps != 0:
            print("len(reward3):{}".format(len(reward3)))
            for e in range(len(reward3) % self.max_env_steps): 
              reward3.pop()
              
        if len(COI) % self.max_env_steps != 0:
            print("len(COI):{}".format(len(COI)))
            for e in range(len(COI) % self.max_env_steps): 
              COI.pop()
              
        if len(ActivUes) % self.max_env_steps != 0:
            print("len(ActivUes):{}".format(len(ActivUes)))
            for e in range(len(ActivUes) % self.max_env_steps): 
              ActivUes.pop()
                              
        reward=0         # Handover failiure occured
        done=True
        next_state=[0] * self.state_dim
        return np.array(next_state), reward, done, info

    state1 = np.reshape(next_state['rbUtil'], [self.Cell_num, 1])#Reshape the matrix (do we need that?)
    state2 = np.reshape(next_state['dlThroughput'],[self.Cell_num,1])
    state2_norm=state2/self.max_throu
    state3 = np.reshape(next_state['UserCount'], [self.Cell_num, 1])#Reshape the matrix (do we need that?)
    state3_norm=state3/self.Users
    MCS_t=np.array(next_state['MCSPen'])
    state4=np.sum(MCS_t[:,:10], axis=1)
    state4=np.reshape(state4,[self.Cell_num,1])
    # To report other rewards

    R_rewards = np.reshape(next_state['rewards'], [3, 1])#Reshape the matrix
    R_rewards =[j for sub in R_rewards for j in sub]
    xx=state3 [0]
    #print("MCS_t[2,:]:{}".format((MCS_t[2,:])))    

    if (sum(state3)) == 0:
        MCS_t[0,:] *= xx/self.Users
        xx=state3 [1]
        MCS_t[1,:] *= xx/self.Users
        xx=state3 [2]
        MCS_t[2,:] *=xx/self.Users
        xx=state3 [3]
        MCS_t[3,:] *=xx/self.Users
        xx=state3 [4]
        MCS_t[4,:] *=xx/self.Users
        xx=state3 [5]
        MCS_t[5,:] *= xx/self.Users
    else:
        MCS_t[0,:] *= xx/sum(state3)#self.Users
        xx=state3 [1]
        MCS_t[1,:] *= xx/sum(state3)#self.Users
        xx=state3 [2]
        MCS_t[2,:] *= xx/sum(state3)#self.Users
        xx=state3 [3]
        MCS_t[3,:] *= xx/sum(state3)#self.Users
        xx=state3 [4]
        MCS_t[4,:] *= xx/sum(state3)#
        xx=state3 [5]
        MCS_t[5,:] *= xx/sum(state3)#self.Users
    AVGMCS= [sum(x) for x in zip(*MCS_t)]

    #print("AVGMCS:{}".format((AVGMCS)))    

    #print("AVGMCS:{}".format((AVGMCS)))
    AVGMCS=np.reshape(AVGMCS,[1, 29])

    AVG_CQI=np.sum(AVGMCS*MCS2CQI)
    #print("AVG_CQI:{}".format((AVG_CQI)))
    
    reward1.append(R_rewards[0])
    reward2.append(R_rewards[1])
    reward3.append(R_rewards[2])
    COI.append(AVG_CQI)
    if (sum(state3)) == 0:
        ActivUes.append(np.array(self.Users).reshape(1))
    else:
        ActivUes.append(sum(state3))
           
    print("ActivUes:{}".format(ActivUes[- 1]))

    if len(reward1) % self.max_env_steps == 0:
        avg_reward1 = np.array(reward1).reshape(-1, self.max_env_steps).mean(axis=1)
        avg_reward2 = np.array(reward2).reshape(-1, self.max_env_steps).mean(axis=1)
        avg_reward3 = np.array(reward3).reshape(-1, self.max_env_steps).mean(axis=1)
        avg_COI1 = np.array(COI).reshape(-1, self.max_env_steps).mean(axis=1)
        avg_ActivUes = np.array(ActivUes).reshape(-1, self.max_env_steps).mean(axis=1)
        std_ActivUes = np.array(ActivUes).reshape(-1, self.max_env_steps).std(axis=1)

    if len(reward1) % self.max_env_steps == 0:
        Result_row=[]
        with open('Rewards_' + self.time_var + '.csv', 'w', newline='') as rewardcsv:
            results_writer = csv.writer(rewardcsv, delimiter=',', quotechar=',', quoting=csv.QUOTE_MINIMAL)
            Result_row.clear()
            Result_row=Result_row+reward1
            results_writer.writerow(Result_row)
            Result_row.clear()
            Result_row=Result_row+reward2
            results_writer.writerow(Result_row)
            Result_row.clear()
            Result_row=Result_row+reward3
            results_writer.writerow(Result_row)
            Result_row.clear()
            Result_row=Result_row+COI
            results_writer.writerow(Result_row)
            Result_row.clear()
            Result_row=Result_row+ActivUes
            results_writer.writerow(Result_row)
            Result_row.clear()
            Result_row=Result_row+avg_reward1.tolist()
            results_writer.writerow(Result_row)
            Result_row.clear()
            Result_row=Result_row+avg_reward2.tolist()
            results_writer.writerow(Result_row)
            Result_row.clear()
            Result_row=Result_row+avg_reward3.tolist()
            results_writer.writerow(Result_row)
            Result_row.clear()
            Result_row=Result_row+avg_COI1.tolist()
            results_writer.writerow(Result_row)
            Result_row.clear()
            Result_row=Result_row+avg_ActivUes.tolist()
            results_writer.writerow(Result_row)
            Result_row.clear()
            Result_row=Result_row+std_ActivUes.tolist()
            results_writer.writerow(Result_row)
        rewardcsv.close()
    print("action:{}".format((action)))
    print("R_rewards:{}".format((R_rewards)))
    #print("done:{}".format((done)))

    #print("reward1 :{}".format(reward1))
    #print("reward2 :{}".format(reward2))
    #print("reward3 :{}".format(reward3))

    print("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-")
    #R_reward1,R_reward2,R_reward3 = np.reshape(state['rewards'], [3, 1])#Reshape the matrix
    next_state  = np.concatenate((state1,state2_norm,state3_norm,state4),axis=None)
    next_state = np.reshape(next_state, [self.state_dim,])

    #print("RB utilization:{}".format(np.transpose(state1)))
    #print("Cell throuhput:{}".format(np.transpose(state2)))
    #print("Users count:{}".format(np.transpose(state3)))
    #print("Edge user ratio:{}".format(np.transpose(state4)))
    #print("rewards:{}".format(np.transpose(R_rewards)))
    #normalize rewards
    #print(" next_state.shape:{}".format( next_state.shape))
    #print("self.observation_space.shape:{}(self.observation_space.shape))
    #if (    next_state.shape ==self.observation_space.shape and np.all(next_state >= self.observation_space.low) and np.all(next_state <= self.observation_space.high)):
        #print("ok")
  
    return np.array(next_state), reward, done, info

  def render(self, mode='console'):
    print("......")


  def close(self):
    pass

