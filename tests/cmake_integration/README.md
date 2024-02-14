# Examples of CMake Integration.
This folder container examples of projects using CMake to integrate HighFive in
the project. The following examples have been provided:

*  `application` contains an application/executable
  that uses HighFive and the optional Boost dependency.

* `dependent_library` contains a library that uses HighFive in its API. It
  consists of a shared and static library; and includes, as an optional
  component, a Boost dependency.

* `test_dependent_library` is an application to test that (or demonstrate how)
  `dependent_library` can be consumed easily.

## Vendoring and Integration Strategy
Note that all examples have been written to pick different vendoring and
integration strategies. This is for testing purposes only. Any real project
would pick a single integration strategy and at most two vendoring strategies.

## Testing
Run `bash test_cmake_integration.sh` to check if the CMake integration example
are working as expected.
