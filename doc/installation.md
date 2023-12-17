# Beginners Installation Guide on Linux

These installation instruction are aimed at developers that aren't very
familiar with installing C/C++ software and using CMake on Linux.

## Obtaining CMake
You'll need a compiler and CMake. We'll assume that a reasonably modern C++
compiler is available. Often a sufficiently new version CMake is also present
on the system.

If not, there's two options: use the system package manager or use `pip`. CMake
is improving in leaps and bounds. Which means you want a recent version. We
suggest reconsidering fighting an older version of CMake if you can simply
install the newest version via `pip`.

## Obtaining HDF5
First, you need to decide if you need MPI-support. Rule of thumb is: if you're
unsure, then you don't need it.  If you need MPI you must install an
MPI-enabled version of HDF5. Otherwise pick either one, if something is already
installed, e.g. because `h5py` was installed as a system package, stick with
that version.

The options for installing HDF5 are:
1. Use the system package manager.
2. On a cluster use modules.
3. Use [Spack](https://github.com/spack/spack).
4. Use [Conan](https://conan.io).
5. Manually install it.

The system package manager will install HDF5 in a default location, were CMake
will later be able to find it without further help. All the other approaches
install into a non-default location and CMake might need help locating HDF5.
The way one tells CMake where to find HDF5 is through `CMAKE_PREFIX_PATH`,
e.g.,

    cmake -DCMAKE_PREFIX_PATH="${HDF5_ROOT}" ...

Note that `${HDF5_ROOT}` points to the folder which contains the two folders
`include` and `lib`.

### System Package Manager
The default choice is to use the system package manager to install HDF5.
One thing to keep an eye out is that certain Linux distributions don't install
the headers automatically. Since you're about to develop an application which
(indirectly) uses HDF5, you need those headers. If the packages are split, the
development package is often suffixed with `-dev` or `-devel`.

#### Ubuntu
The package manager is apt. To install the HDF5 C library without MPI support:

    sudo apt-get install libhdf5-dev

for MPI support you'd install `libhdf5-openmpi-dev`.

#### ArchLinux
On ArchLinux you install

    sudo pacman -S hdf5

or `hdf5-openmpi` for HDF5 with OpenMPI support.


### Using Modules
If you're on a cluster, HDF5 has almost certainly been installed for you.
Figure out how to use it. This is the preferred solution on clusters. As
always, the precise instructions depend on the cluster, but something like

    module load hdf5

will probably be part of the solution. Try if `module avail` helps. Otherwise,
you'd need to check the documentation for your cluster. Cluster admins like to
hide the good stuff, i.e. modern versions, behind another package `"new"` or
some other mechanism.

You might need to know where HDF5 has been installed. You can find out what a
module does by typing

    module show hdf5

If it contains something about prepending to `CMAKE_PREFIX_PATH`, then CMake
should find the correct version automatically after loading the module.

### Using Spack
If neither of the above work, the next best choice might be Spack. It's a
package manager for scientific computing. The general idea behind it is to
avoid dependency issues by compiling a compatible set of everything.

Obtain Spack by cloning their repo:

    git clone https://github.com/spack/spack.git

Activate Spack by sourcing a magic file:

    source spack/share/spack/setup-env.sh

which will put the command `spack` into your `PATH`. Okay, now we're set. First
step is to create an environment for your project, which we'll call `useful`:

    spack env create useful
    spack env activate -p useful
    spack add hdf5
    spack install --jobs NPROC

If you need MPI support use `hdf5+mpi` instead. The location of the HDF5
installation is `spack location --install-dir hdf5`.

### Conan
If Spack doesn't work, you can try Conan.

### Manually Install HDF5
If all else fails, you can try to manually install HDF5. First you need to
obtain the source code. For example by using `git` or by downloading an archive
from their webpage.

    git clone https://github.com/HDFGroup/hdf5
    cd hdf5
    git checkout hdf5-1_14_0

Now, fingers crossed it'll compile and install:

    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../hdf5-v1.14.0 -B build .
    cmake --build build --parallel [NPROC]
    cmake --install build

Note that here we picked the installation path (or more precisely prefix) to be
`../hdf5-v1.14.0`. You might want to install HDF5 somewhere else. This
installation prefix is the also the path you need to give CMake so it's able to
find HDF5 later on.

### Confirming HDF5 Has Been Installed
For this purpose we need a dummy file `dummy.cpp` to compile:

    #include <hdf5.h>

    int main() {
      auto file = H5Fcreate("foo.h5", H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
      H5Fclose(file);
      return 0;
    }

and a `CMakeLists.txt` with the following contents

    cmake_minimum_required(VERSION 3.19)
    project(Dummy)

    find_package(HDF5 REQUIRED)
    add_executable(dummy dummy.cpp)
    target_link_libraries(dummy HDF5::HDF5)

Now run CMake to configure the build system and keep an eye out for some a line
related to HDF5, e.g.

    $ cmake -B build .
    ...
    -- Found HDF5: hdf5-shared (found version "1.14.0")
    ...

Compile and check that it's doing something with sensible

    $ cmake -build build --verbose
    [ 50%] Building CXX object CMakeFiles/dummy.dir/dummy.cpp.o
    /usr/bin/c++ ... -isystem ${HDF5_ROOT}/include ... -c ${PWD}/dummy.cpp
    [100%] Linking CXX executable dummy
    /usr/bin/c++ ... -o dummy -Wl,-rpath,${HDF5_ROOT}/lib ${HDF5_ROOT}/lib/libhdf5.so.310.0.0 ...

mostly you're checking that the paths are what you'd expect them to be. If
this command was successful, chances are high that HDF5 is properly installed
and you've figured out the correct CMake invocations. If you want you can run
the executable:

    build/dummy

which would create an empty file `foo.h5`.

## Obtaining HighFive

In principle the same instruction as for HDF5 can be used. However, HighFive is
much less popular compared to HDF5 and therefore the system package manager
likely doesn't know about it, nor does Conan. You're left with Spack and the
manual process. It seems someone has done the wonderful work of adding HighFive
to conda-forge, so maybe that's also an option.

### Git Submodules
This is the well-tested method for "vendoring" HighFive, i.e. including the
HighFive sources with those of you project.

### Spack
Similarly as for HDF5, you can use Spack to install HighFive:

    spack env activate -p useful
    spack add highfive
    spack install --jobs NPROC

Again `spack location --install-dir highfive` will return the path where
HighFive was installed. Since the Spack recipe of HighFive declares HDF5 as a
dependency, technically, it's not necessary to add `hdf5`, just `highfive` is
enough.

### Manually Install HighFive
Just like before the steps are, clone, configure, compile (kinda a no-op),
install. The detailed instructions would be

    git clone --recursive https://github.com/BlueBrain/HighFive.git
    cd HighFive
    git checkout v2.8.0

If it complains that Catch is missing, you forgot the `--recursive`. To fix
this you type

    git submodule update --init --recursive

Okay, on to configure, compile and install. The CMake commands are

    cmake -DCMAKE_INSTALL_PREFIX=build/install -B build .
    cmake --build build --parallel
    cmake --install build

Later you'd pass the installation directory, i.e. `${PWD}/build/install`, to
`CMAKE_PREFIX_PATH`.

### Confirming It Works
We again need a dummy file called `dummy.cpp` with the following contents

    #include <highfive/highfive.hpp>

    int main() {
      auto file = HighFive::File("foo.h5", HighFive::File::Create);
      return 0;
    }

and the following `CMakeLists.txt`:

    cmake_minimum_required(VERSION 3.19)
    project(UseHighFive)

    find_package(HighFive REQUIRED)
    add_executable(dummy dummy.cpp)
    target_link_libraries(dummy HighFive)

The required CMake commands are:

    $ cmake -DCMAKE_PREFIX_PATH="${HDF5_ROOT};${HIGHFIVE_ROOT}" -B build .
    ...
    -- HIGHFIVE 2.7.1: (Re)Detecting Highfive dependencies (HIGHFIVE_USE_INSTALL_DEPS=NO)
    -- Found HDF5: hdf5-shared (found version "1.14.0")
    ...

    $ cmake --build build --verbose
    [ 50%] Building CXX object CMakeFiles/dummy.dir/dummy.cpp.o
    /usr/bin/c++ ... -isystem ${HIGHFIVE_ROOT}/include -isystem ${HDF5_ROOT}/include ... -c dummy.cpp
    [100%] Linking CXX executable dummy
    /usr/bin/c++ ... -o dummy -Wl,-rpath,${HDF5_ROOT}/lib ${HDF5_ROOT}/lib/libhdf5.so.310.0.0 ...

Pay attention to the semi-colon (not colon like the rest of Linux) used to
separate directories in `CMAKE_PREFIX_PATH`. If this worked you should be set
to either copy the instruction to your "real" project, or start developing the
rest of your project.
