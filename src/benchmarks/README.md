# Benchmarking

This folder contains several baseline programs used to benchmark HighFive and assess
its overhead wrt no-highfive programs.

It features a straightforward Makefile whose flags can be easily tuned. By default
compilation features -O2 -g and finds HDF5 via `pkg-config`.
Additionally, a `run_benchmarks.sh` script is provided to measure execution time and
profile using hpctoolkit.

## Compile

Basically, run `make`. By default it compiles with `-g -O2` but it's configurable. e.g.

```
make CXX=clang++ COMPILE_OPTS="-g -O1"
```
