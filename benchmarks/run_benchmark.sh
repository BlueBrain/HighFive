#!/bin/sh
set -eu

executables="hdf5_bench hdf5_bench_improved highfive_bench"

# Compile all
make

# Time it : overview and eventual cache warm up
for exe in $executables; do
    echo -e "\nRunning $exe"
    time ./$exe
done


# Profile using hpctoolkit
module load hpctoolkit
rm -rf hpctoolkit-* *.hpcstruct

for exe in $executables; do
    echo -e "\nProfiling $exe"
    hpcrun -c f1000000 -e PERF_COUNT_HW_CPU_CYCLES -e REALTIME  ./$exe
    hpcstruct ./$exe
    hpcprof -S $exe.hpcstruct -I ./ -I '../include/*' -I /gpfs/bbp.cscs.ch/ssd/apps/hpc/jenkins/deploy/compilers/2021-01-06/linux-rhel7-x86_64/gcc-4.8.5/gcc-10.2.0-wvxdxx/include/* hpctoolkit-$exe-*
done
