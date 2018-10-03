#!/bin/bash
#SBATCH -p batch -N 1 -n 4
time srun ./HW1_106062588_basic_calc 4 ./testcase/testcase01 ./output/output01.out
time srun ./HW1_106062588_basic_calc 15 ./testcase/testcase02 ./output/output02.out
time srun ./HW1_106062588_basic_calc 21 ./testcase/testcase03 ./output/output03.out
time srun ./HW1_106062588_basic_calc 50 ./testcase/testcase04 ./output/output04.out
time srun ./HW1_106062588_basic_calc 100 ./testcase/testcase05 ./output/output05.out
time srun ./HW1_106062588_basic_calc 65536 ./testcase/testcase06 ./output/output06.out
time srun ./HW1_106062588_basic_calc 12345 ./testcase/testcase07 ./output/output07.out
time srun ./HW1_106062588_basic_calc 100000 ./testcase/testcase08 ./output/output08.out
time srun ./HW1_106062588_basic_calc 99999 ./testcase/testcase09 ./output/output09.out
time srun ./HW1_106062588_basic_calc 63942 ./testcase/testcase10 ./output/output10.out