#!/bin/bash
#SBATCH -p batch -N 4 -n 48
time srun ./HW1_106062588_advance_cacl 322639999 ./mytestcase ./output/mytestcase_ad.out

