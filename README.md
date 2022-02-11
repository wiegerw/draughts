# draughts
The draughts library is a C++ library for draughts. It contains python bindings for Scan 3.1.

## Requirements
A C++17 compiler.

## Build
The following build systems are supported
* CMake 3.15+

### Python bindings
Draughts comes with python bindings. For this the package pybind11 is needed, see 
https://github.com/pybind/pybind11. Note that on Ubuntu the installation of
the `pybind11-dev` package may fail, so instead a manual installation may be needed.
See also https://github.com/pybind/pybind11/issues/2748.
