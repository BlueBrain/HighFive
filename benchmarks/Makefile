CXX=g++
HDF5_ROOT=/usr/local
CXXFLAGS=-I ../include/ -I ${HDF5_ROOT}/include -L ${HDF5_ROOT}/lib -lhdf5 -std=c++11 -O2 -Wall
PROGRAMS:=min_highfive hdf5_bench highfive_bench

all: $(PROGRAMS)

%: %.cpp $(DEPS)
	$(CXX) -o $@ $< $(CXXFLAGS)

clean:
	rm -f ${PROGRAMS}

.PHONY: clean
