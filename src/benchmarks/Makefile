# This is minimal makefile to build the benchmark programs
# It was kept as an independent Makefile so that use can trivially change flags
#
# Blue Brain Project - EPFL, 2022

PROGRAMS:=hdf5_bench hdf5_bench_improved highfive_bench

CXX?=g++
HDF5_ROOT?=/usr/local
COMPILE_OPTS=-g -O2 -Wall
CXXFLAGS=-I ../../include/ -I ${HDF5_ROOT}/include -L ${HDF5_ROOT}/lib -lhdf5 -std=c++11 ${COMPILE_OPTS}


all: $(PROGRAMS)

%: %.cpp $(DEPS)
	$(CXX) -o $@ $< $(CXXFLAGS)

clean:
	rm -f ${PROGRAMS}

.PHONY: clean
