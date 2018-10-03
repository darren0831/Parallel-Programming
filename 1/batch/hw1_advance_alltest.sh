#!/bin/bash
#SBATCH -p batch -N 1 -n 4
time srun ./HW1_106062588_advance 4 ./testcase/testcase01 ./output/lalala01.out
time srun ./HW1_106062588_advance 15 ./testcase/testcase02 ./output/lalala02.out
time srun ./HW1_106062588_advance 21 ./testcase/testcase03 ./output/lalala03.out
time srun ./HW1_106062588_advance 50 ./testcase/testcase04 ./output/lalala04.out
time srun ./HW1_106062588_advance 100 ./testcase/testcase05 ./output/lalala05.out
time srun ./HW1_106062588_advance 65536 ./testcase/testcase06 ./output/lalala06.out
time srun ./HW1_106062588_advance 12345 ./testcase/testcase07 ./output/lalala07.out
time srun ./HW1_106062588_advance 100000 ./testcase/testcase08 ./output/lalala08.out
time srun ./HW1_106062588_advance 99999 ./testcase/testcase09 ./output/lalala09.out
time srun ./HW1_106062588_advance 63942 ./testcase/testcase10 ./output/lalala10.out