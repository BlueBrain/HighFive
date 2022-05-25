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

if [ $# -eq 0 ]; then
    echo "Not running hpctoolkit. Please provide a CLI argument to proceed"
    exit 0
fi

# Profile using hpctoolkit
module load hpctoolkit
rm -rf hpctoolkit-* *.hpcstruct

for exe in $executables; do
    echo -e "\nProfiling $exe"
    hpcrun -c f1000000 -e PERF_COUNT_HW_CPU_CYCLES -e REALTIME  ./$exe
    hpcstruct ./$exe
    hpcprof -S $exe.hpcstruct -I ./ -I '../../include/*' hpctoolkit-$exe-*
done
