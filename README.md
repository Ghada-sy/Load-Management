# Mobility Load Management 
Source code for the paper "Mobility Load Management in Cellular Networks: A Deep Reinforcement Learning Approach"

# Installation
## Installing Prerequisites

1- Install Python 3.8.10 (Do Not use the virtual environment)

2- Install [ns-3.30](https://www.nsnam.org/wiki/Installation).

3- Download [ns3gym](https://apps.nsnam.org/app/ns3-gym/).

4- Follow installation steps: [ns3gym](https://github.com/tkn-tub/ns3-gym).

5- Install tensorflow 2.8.0 and keras 2.8.0. (Do Not use the virtual environment, use pip3). 

6- Install [Stablebasline3](https://github.com/DLR-RM/stable-baselines3).


## Installing the code

Note: that ns-3 installtion directory is called Path_to_NS3_Directory.

1- Copy POCS and RealSce folders to Path_to_NS3_Directory/scratch/.

2- Copy cell-individual-offset.h and cell-individual-offset.cc to Path_to_NS3_Directoy/src/lte/model/.

3- Copy LTE_Attributes.txt, Real_model-attributes.txt, script_LTE_POCS.sh and script_LTE_RealSce.sh to Path_to_NS3_Directoy/.

4- Copy and replace lte-ue-rrc.cc with Path_to_NS3_Directoy/src/lte/model/lte-ue-rrc.cc (Rememebr to backup the original).

5- Copy and replace wscript with Path_to_NS3_Directoy/src/lte/wscript (Remember to backup the original).

6- Rename ns3gym folder to "opengym" and place it in Path_to_NS3_Directory/src.

7- Build ns-3 again by navigating to Path_to_NS3_Directoy and running the commands:
```
$ ./waf configure -d debug --enable-examples --enable-tests
$ ./waf
```

  
8- You can configure your own attributes from the (LTE_Attributes.txt, Real_model-attributes.txt)

9- To run the Proof of concept scenario. In the directory Path_to_NS3_Directoy:

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

10- To run the realistic scenario. In the directory Path_to_NS3_Directoy:

- right-click to open the terminal and run the command:
     
```
$ ./script_LTE_RealSce.sh
```

- To run one episode only run the follosing command instead:
     
```
$ ./waf --run "scratch/Power_CIO/Power_CIO"
```

- Open a new tab in the terminal. This tab is used to run the TD3 agent code. 
     
```
$ cd scratch/RealSce
$ python3 Agent_TD3.py
```



# References
If you find this code useful for your research, please cite the following paper:
```
@article{Alsuhli2020Mobility,
  title={Mobility Load Management in Cellular Networks: A Deep Reinforcement Learning Approach},
  author={Alsuhli, Ghada and Banawan, Karim and Attiah, Kareem and Elezabi, Ayman and Seddik, Karim and Gaber, Ayman and Zaki, Mohamed and Gadallah, Yasser},
  journal={arXiv preprint XXXXXXXX},
  year={2020}
}
```
