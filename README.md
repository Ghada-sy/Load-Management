# Mobility Load Management 
We consider the problem of mobility load management (balancing) in cellular LTE networks. In this page we provide the source codes for the following papers:

1- "Mobility Load Management in Cellular Networks: A Deep Reinforcement Learning Approach":
We propose a comprehensive deep reinforcement learning (RL) framework for steering the cell individual offset (CIO) of every pair of cells. We consider two scenarios, Proof-of-Concept scenario and Realistic LTE Network Scenario.

2- "Optimized Power and Cell Individual Offset for Cellular Load Balancing via Reinforcement Learning":
In this work, we propose a joint power levels and CIOs optimization using reinforcement learning (RL).


# Installation
## Installing Prerequisites

1- Install Python 3.8.10 (Do Not use the virtual environment)

2- Install [ns-3.30](https://www.nsnam.org/wiki/Installation) (Follow prerequisites steps and then manual installation (Do not use bake)).
 Notice that:  1- some of the packages to be installed are deprecated with higher Ubuntu versions
			         2-(./build.py) is not applicable for versions less than 3.36. skip it and use (Configuration with Waf) commands. 
			         3- Make sure you run (./waf -enable-tests --enable-examples configure) and (./waf build) before running the test (./test.py)

3- Download [ns3gym](https://apps.nsnam.org/app/ns3-gym/). For versions less than 3.36 download the package in this [link](https://github.com/tkn-tub/ns3-gym/tree/app)


4- Follow installation steps: [ns3gym installation](https://github.com/tkn-tub/ns3-gym). For versions less than 3.36 use the instructions in this [link](https://github.com/tkn-tub/ns3-gym/tree/app)


5- Install tensorflow 2.8.0 and keras 2.8.0. (Do Not use the virtual environment, use pip3). 

6- Install [Stablebasline3](https://github.com/DLR-RM/stable-baselines3).

7- Install mobility models from [here](https://drive.google.com/file/d/1fyL4PGqiqbIlOouuoAEH4TrHVXOqhQWG/view?usp=sharing) and [here](https://drive.google.com/file/d/11UdEeDm5oidBuLs9Ud9w5zmWwloGh8Z3/view?usp=sharing) in /Scratch directory
 

## Installing the code

Note: that ns-3 installtion directory is called Path_to_NS3_Directory.

1- Copy POCS, RealSce and Power_CIO folders to Path_to_NS3_Directory/scratch/. Try not to use nested folders i.e. avoid: scratch/folder1/folder2/file.cc, instead use scratch/folder1/file.cc. 

2- Replace the diectory Path_to_NS3_Directory/scr/lte with the directory inside the archived file in lte(1).zip (Rememebr to backup the original)

3- Copy cell-individual-offset.h and cell-individual-offset.cc to Path_to_NS3_Directoy/src/lte/model/.

4- Copy LTE_Attributes.txt, Real_model-attributes.txt, script_LTE_POCS.sh, script_LTE_RealSce.sh and Power_CIO.sh to Path_to_NS3_Directoy/.

5- Copy and replace lte-ue-rrc.cc with Path_to_NS3_Directoy/src/lte/model/lte-ue-rrc.cc (Rememebr to backup the original).

6- Copy and replace lte-enb-phy.cc with Path_to_NS3_Directoy/src/lte/model/lte-enb-phy.cc (Rememebr to backup the original).

7- Copy and replace wscript with Path_to_NS3_Directoy/src/lte/wscript (Remember to backup the original).

8- Rename ns3gym folder to "opengym" and place it in Path_to_NS3_Directory/src.

9- Build ns-3 again by navigating to Path_to_NS3_Directoy and running the commands:
```
$ ./waf configure -d debug --enable-examples --enable-tests
$ ./waf
```
Notice that: If the ./waf command resulted in an error, remove the directory Path_to_NS3_Directory/contrib/opengym from this path
  
10- You can configure your own attributes from the (LTE_Attributes.txt, Real_model-attributes.txt)

11- Place the mobility model files in Path_to_NS3_Directory/scratch.

12- To run the Proof of concept scenario. In the directory Path_to_NS3_Directoy:

- right-click to open the terminal and run the command:
     
```
$ ./script_LTE_POCS.sh
```
For the first run, you may need to run
```
$ chmod +x ./script_LTE_POCS.sh
```

-To run one episode only run the following command instead:

```
$ ./waf --run "scratch/POCS/POCS --RunNum=$(($i))"
```

- Open a new tab in the terminal. This tab is used to run the DDQN agent code. 
     
```
$ cd scratch/POCS
$ python3 ddqn_agent.py
```

13- To run the realistic scenario. In the directory Path_to_NS3_Directoy:

- right-click to open the terminal and run the command:
     
```
$ ./script_LTE_RealSce.sh
```

- To run one episode only run the follosing command instead:
     
```
$ ./waf --run "scratch/RealSce/RealSce --RunNum=$(($i))"
```

- Open a new tab in the terminal. This tab is used to run the TD3 agent code. 
     
```
$ cd scratch/RealSce
$ python3 Agent_TD3.py
```

14- To run the joint power and CIO optimization scenario.

In the directory Path_to_NS3_Directoy:

- right-click to open the terminal and run the command:
     
```
$ ./Power_CIO.sh
```

- To run one episode only run the follosing command instead:
     
```
$ ./waf --run "scratch/Power_CIO/Power_CIO"
```

- Open a new tab in the terminal. This tab is used to run the TD3 agent code. 
     
```
$ cd scratch/Power_CIO
$ python3 BL_TD3.py
```



# References
If you find this code useful for your research, please cite the following papers:
```
@article{Alsuhli2020Mobility,
  title={Mobility Load Management in Cellular Networks: A Deep Reinforcement Learning Approach},
  author={Alsuhli, Ghada and Banawan, Karim and Attiah, Kareem and Elezabi, Ayman and Seddik, Karim and Gaber, Ayman and Zaki, Mohamed and Gadallah, Yasser},
  journal={arXiv preprint XXXXXXXX},
  year={2020}
}

@inproceedings{alsuhli2021optimized,
  title={Optimized power and cell individual offset for cellular load balancing via reinforcement learning},
  author={Alsuhli, Ghada and Banawan, Karim and Seddik, Karim and Elezabi, Ayman},
  booktitle={2021 IEEE Wireless Communications and Networking Conference (WCNC)},
  pages={1--7},
  year={2021},
  organization={IEEE}
}
```
