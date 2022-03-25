import os
import time
import gym
import numpy as np
import matplotlib.pyplot as plt

from stable_baselines import TD3
from stable_baselines.td3.policies import MlpPolicy
from stable_baselines import results_plotter
from VecMonitor import VecMonitor
from stable_baselines.results_plotter import load_results, ts2xy
from stable_baselines.common.noise import NormalActionNoise, OrnsteinUhlenbeckActionNoise, AdaptiveParamNoiseSpec
from stable_baselines.common.vec_env import DummyVecEnv, VecNormalize
from stable_baselines.common.callbacks import BaseCallback
from stable_baselines.common.cmd_util import make_vec_env
from myns3env import myns3env

class SaveOnBestTrainingRewardCallback(BaseCallback):
    """
    Callback for saving a model (the check is done every ``check_freq`` steps)
    based on the training reward (in practice, we recommend using ``EvalCallback``).

    :param check_freq: (int)
    :param log_dir: (str) Path to the folder where the model will be saved.
      It must contains the file created by the ``Monitor`` wrapper.
    :param verbose: (int)
    """
    def __init__(self, check_freq: int, log_dir: str, verbose=1):
        super(SaveOnBestTrainingRewardCallback, self).__init__(verbose)
        self.check_freq = check_freq
        self.log_dir = log_dir
        self.save_path = os.path.join(log_dir, 'best_model')
        self.best_mean_reward = -np.inf

    def _init_callback(self) -> None:
        # Create folder if needed
        if self.save_path is not None:
            os.makedirs(self.save_path, exist_ok=True)

    def _on_step(self) -> bool:
        print("Steps: {}".format(self.num_timesteps))

        if self.n_calls % self.check_freq == 0:

          # Retrieve training reward
          x, y = ts2xy(load_results(self.log_dir), 'timesteps')
          if len(x) > 0:
              # Mean training reward over the last 100 episodes
              mean_reward = np.mean(y[-100:])
              if self.verbose > 0:
                print("Num timesteps: {}".format(self.num_timesteps))
                print("Best mean reward: {:.2f} - Last mean reward per episode: {:.2f}".format(self.best_mean_reward, mean_reward))

              # New best model, you could save the agent here
              if mean_reward > self.best_mean_reward:
                  self.best_mean_reward = mean_reward
                  # Example for saving best model
                  if self.verbose > 0:
                    print("Saving new best model to {}".format(self.save_path))
                  self.model.save(self.save_path)

        return True

# Create log dir
log_dir = "tmp_{}/".format(int(time.time()))
os.makedirs(log_dir, exist_ok=True)

# Create and wrap the environment
# Instantiate the env
env = myns3env()
# wrap it
env = make_vec_env(lambda: env, n_envs=1)

env = VecMonitor(env, log_dir)
#env = VecNormalize(env, norm_obs=True, norm_reward=False, clip_obs=100.)
# the noise objects for TD3

n_actions = env.action_space.shape[-1]

action_noise = NormalActionNoise(mean=np.zeros(n_actions), sigma=0.1 * np.ones(n_actions))

model = TD3(MlpPolicy, env, action_noise=action_noise, verbose=1, tensorboard_log="./TD3_ped_veh_tensorboard/")


# Create the callback: check every 1000 steps
callback = SaveOnBestTrainingRewardCallback(check_freq=250, log_dir=log_dir) 
# Train the agent
time_steps = 50000
model.learn(total_timesteps=int(time_steps), callback=callback)
model.save("TD3_ped_veh_r1_{}".format(int(time.time())))
print("######################Testing#####################")

# Test the agent
# episode_rewards = []
# for i in range(2):
    # reward_sum = 0
    # obs = env.reset()
    # for j in range(250):  
        # print("Test: Step : {} | Episode: {}".format(j, i))
        # action, _states = model.predict(obs)
        # obs, rewards, dones, info = env.step(action)
        # reward_sum += rewards
        # print("rewards:{}".format((rewards)))
        # print("reward_sum:{}".format((reward_sum)))
    # episode_rewards.append(reward_sum)
# print('agent average Reward:{} '.format(episode_rewards))
# episode_rewards_0 = []

# for i in range(2):
    # reward_sum0 = 0
    # obs = env.reset()
    # for j in range(250):  
        # action=[0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0]
        # print("0CIO: Step : {} | Episode: {}".format(j, i))
        # obs, rewards, dones, info = env.step(action)
        # reward_sum0 += rewards
    # episode_rewards_0.append(reward_sum0)
# print('0 CIO  average Reward:{} '.format(episode_rewards_0))

# fig1, ax = plt.subplots()
# ln1, = plt.plot(episode_rewards, label='TD3')
# ln1, = plt.plot(episode_rewards_0, label='CIO=0')
# legend = ax.legend(loc='upper left', shadow=True, fontsize='x-small')
# plt.xlabel("Episode")
# plt.ylabel("Average overall throuput")
# plt.title('TD3, Realistic Scenario, "')
# plt.savefig('TD3_0CIO_{}.png'.format(int(time.time())))

