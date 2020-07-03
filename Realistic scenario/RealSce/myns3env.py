import numpy as np
import gym
from gym import spaces
from ns3gym import ns3env
import csv

reward1=[]
reward2=[]
reward3=[]
class myns3env(gym.Env):
  """
  Custom Environment that follows gym interface.
  This is a simple env where the agent must learn to go always left.
  """


  def __init__(self,):
    super(myns3env, self).__init__()
    port=9999
    simTime= 45
    stepTime=0.2
    startSim=0
    seed=3
    simArgs = {"--duration": simTime,}
    debug=True
    max_env_steps = 250
    self.env = ns3env.Ns3Env(port=port, stepTime=stepTime, startSim=startSim, simSeed=seed, simArgs=simArgs, debug=debug)
    self.env._max_episode_steps = max_env_steps
    self.Cell_num=6
    self.max_throu=30
    self.Users=40
    self.state_dim = self.Cell_num*4
    self.action_dim =  self.env.action_space.shape[0]
    self.action_bound =  self.env.action_space.high
    self.action_space = spaces.Box(low=-1, high=1,
                                        shape=(self.action_dim,), dtype=np.float32)
    self.observation_space = spaces.Box(low=0, high=self.Users,
                                        shape=(self.state_dim,), dtype=np.float32)    
  def reset(self):

    state = self.env.reset()
    state1 = np.reshape(state['rbUtil'], [self.Cell_num, 1])#Reshape the matrix
    state2 = np.reshape(state['dlThroughput'],[self.Cell_num,1])
    state2_norm=state2/self.max_throu
    state3 = np.reshape(state['UserCount'], [self.Cell_num, 1])#Reshape the matrix
    state3_norm=state3/self.Users
    MCS_t=np.array(state['MCSPen'])
    state4=np.sum(MCS_t[:,:10], axis=1)
    state4=np.reshape(state4,[self.Cell_num,1])
    # To report other reward functions
    R_rewards = np.reshape(state['rewards'], [3, 1])#Reshape the matrix
    R_rewards =[j for sub in R_rewards for j in sub]

    state  = np.concatenate((state1,state2_norm,state3_norm,state4),axis=None)
    state = np.reshape(state, [self.state_dim,])###

    return np.array(state)

  def step(self, action):
    action=action*self.action_bound
    next_state, reward, done, info = self.env.step(action)

    state1 = np.reshape(next_state['rbUtil'], [self.Cell_num, 1])#Reshape the matrix (do we need that?)
    state2 = np.reshape(next_state['dlThroughput'],[self.Cell_num,1])
    state2_norm=state2/self.max_throu
    state3 = np.reshape(next_state['UserCount'], [self.Cell_num, 1])#Reshape the matrix (do we need that?)
    state3_norm=state3/self.Users
    MCS_t=np.array(next_state['MCSPen'])
    state4=np.sum(MCS_t[:,:10], axis=1)
    state4=np.reshape(state4,[self.Cell_num,1])
    # To report other reward functions
    R_rewards = np.reshape(next_state['rewards'], [3, 1])#Reshape the matrix
    R_rewards =[j for sub in R_rewards for j in sub]
    global reward1
    global reward2
    global reward3
    reward1.append(R_rewards[0])
    reward2.append(R_rewards[1])
    reward3.append(R_rewards[2])
    if len(reward1) % 1000 == 0:
        Result_row=[]
        with open('Rewards_' + 'Thr_opt' + '.csv', 'w', newline='') as rewardcsv:
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
        rewardcsv.close()
    print("action:{}".format((action)))
    print("Reward functions:{}".format((R_rewards)))


    print("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-")
    next_state  = np.concatenate((state1,state2_norm,state3_norm,state4),axis=None)
    next_state = np.reshape(next_state, [self.state_dim,])
  
    return np.array(next_state), reward, done, info

  def render(self, mode='console'):
    print("......")


  def close(self):
    pass
