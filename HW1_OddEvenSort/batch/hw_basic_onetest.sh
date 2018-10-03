#!/bin/bash
#SBATCH -p batch -N 4 -n 48
time srun ./HW1_106062588_basic_calc 997919 ./testcase/mytestcase ./output/mytestcase1.out
