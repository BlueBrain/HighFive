---
name: Build failure
about: Report an issue with the build-system.
title: ''
labels: ''
assignees: ''

---

**Bug Description**
Unfortunately, build failures tend to be highly specific and there's many
things that could go wrong on both our and your side. Therefore, a reproducer
is essential. You could try the minimal setup found here:
https://github.com/BlueBrain/HighFive/blob/master/doc/installation.md#manually-install-highfive

from there you can work upwards by adding complexity until you reproduce the
issue.

Once you have a reproducer, please paste it and the exact `cmake` command used to
configure the build and include the output. For the compilation phase please
ensure that the actual compiler invocation is visible, e.g.,
```
$ cmake --build build --verbose
[ 50%] Building CXX object CMakeFiles/dummy.dir/dummy.cpp.o
/usr/bin/c++ ... -isystem ${HIGHFIVE_ROOT}/include -isystem ${HDF5_ROOT}/include ... -c dummy.cpp
```
and include at least the first error message. (If in doubt include more rather
than less output.)

**Version Information**
  - HighFive:
  - Compiler:
  - OS:
  - CMake:
  - HDF5:

**Style Guide**
1. Please paste text as text and not as a screen shot.
2. If in doubt paste too much output rather than too little, i.e. don't be too
   scared of a large wall of text. Especially, if it's a compiler error.
   (Anything past the first error is largely uninformative and can be safely
   stripped.)
3. Please strip all boilerplate.

