# SO-CPU-Scheduler
Repo for the Operating Systems project for academic year 2022/2023 and for my Bachelor's degree thesis.

Cpu Scheduling Simulator:
   Modify the scheduling simulator presented at lesson to handle:
     1. multiple (configurable) cpus
     2. preemptive shortest job first with quantum prediction q(t+1) = a * q_current + (1-a) * q(t)

Random process generator:
   Project and implement a program to take as input probability distributions and generate as output files representing random processes to pass as input to the scheduling  simulator. The distribution of CPU and I/O bursts in the processes must be coherent with the input distribution, given enough samples. 
