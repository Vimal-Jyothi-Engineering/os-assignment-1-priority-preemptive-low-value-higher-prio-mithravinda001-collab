[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/tox7PhM5)
Write a C program priority.c that implements non-preemptive priority
CPUscheduling (low value has higher priority).
Input Format:
n
PID1 AT1 BT1 PR1
PID2 AT2 BT2 PR2
..............................
PIDn ATn BTn PRn
15 CO1 L3
2
Sample Input:
5
P1 2 6 3
P2 5 2 1
P3 1 8 4
P4 0 3 2
P5 4 4 2
Sample Output:
Waiting Time:
P1 5
P2 2
P3 14
P4 0
P5 1
Turnaround Time:
P1 11
P2 4
P3 22
P4 3
P5 5
Average Waiting Time: 4.40
Average Turnaround Time: 9.00
