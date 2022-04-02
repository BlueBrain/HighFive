#!/bin/sh
set -eu

executables="hdf5_bench hdf5_bench_improved highfive_bench"

# Compile all
make

# Time it
for exe in $executables; do
    echo -e "\nRunning $exe"
    time ./$exe
done

# Profile using hpctoolkit
module load hpctoolkit
rm -rf hpctoolkit-*

for exe in $executables; do
    echo -e "\nProfiling $exe"
    hpcrun -e IO ./$exe
    hpcrun -e REALTIME@50000 ./$exe
    hpcrun -e CPUTIME@50000 ./$exe
    hpcstruct ./$exe
    hpcprof -I ./ -I ../include/* hpctoolkit-$exe-*
done
