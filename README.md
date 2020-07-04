# Mobility Load Management 
Source code for the paper "Mobility Load Management in Cellular Networks: A Deep Reinforcement Learning Approach"

# Installation
## Installing Prerequisites

1- Install Python3.6. (Do Not use the virtual environment)

2- Install [ns-3.30](https://www.nsnam.org/wiki/Installation).

3- Install [ns3gym](https://github.com/tkn-tub/ns3-gym).

4- Install [Tensorflow version 1.14](https://www.tensorflow.org/install/pip) and [Keras](https://pypi.org/project/Keras/). (Do Not use the virtual environment, use pip3) 

5- Install [Stablebaselines](https://github.com/hill-a/stable-baselines).

## Installing the code

Note: that ns-3 installtion directory is called Path_to_NS3_Directory.

1- copy POCS and RealSce folders to Path_to_NS3_Directory/scratch/.

2- copy cell-individual-offset.h and cell-individual-offset.cc to Path_to_NS3_Directoy/src/lte/model/.

3- Copy LTE_Attributes.txt, Real_model-attributes.txt, script_LTE_POCS.sh and script_LTE_RealSce.sh to Path_to_NS3_Directoy/.

4- Copy and replace lte-ue-rrc.cc with Path_to_NS3_Directoy/src/lte/model/lte-ue-rrc.cc (Rememebr to backup the original).

5- Copy and replace wscript with Path_to_NS3_Directoy/src/lte/wscript (Remember to backup the original).

6- Build ns-3 again by navigating to Path_to_NS3_Directoy and running the commands:
```
		$ ./waf configure -d debug --enable-examples --enable-tests
		$ ./waf
```

  
7- You can configure your own attributes from the (LTE_Attributes.txt, Real_model-attributes.txt)

8- To run the Proof of concept scenario. In the directory Path_to_NS3_Directoy:
     - right-click to open the terminal
     - run the command:
```
$ ./script_LTE_POCS.sh
```
For the first run, you may need to run
```
$ chmod +x ./script_LTE_POCS.sh
```
     - Open a new tab in the terminal. This tab is used to run the DDQN agent code. 
     
```
$ python3 scratch/POCS/ddqn_agent.py
```


 8- To run the realistic scenario.
 
   In the directory:Path_to_NS3_Directoy
   - right-click to open the terminal
   - run the command:
   

        ```
	$ ./script_LTE_RealSce.sh
	```  
   (For the first run, you may need to run $ chmod +x ./script_LTE_RealSce.sh)

   - Open a new tab in the terminal. This tab is used to run the TD3 agent code. 
       
	```
	$ python3 scratch/RealSce/Agent_TD3.py
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
