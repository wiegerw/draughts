# draughts
The draughts library is a C++ library for draughts. It contains
Scan 3.0.

## Requirements
A C++17 compiler.

## Build
The following build systems are supported
* CMake 3.15+
* B2 (https://www.bfgroup.xyz/b2/)

## Dependencies
Draughts uses the following third-party libraries. They are included in the repository.

* Boost (http://boost.org) 

### Python bindings
Draughts comes with python bindings. For this the package pybind11 is needed, see 
https://github.com/pybind/pybind11. Note that on Ubuntu the installation of
the `pybind11-dev` package may fail, so instead a manual installation may be needed.
See also https://github.com/pybind/pybind11/issues/2748.

A manual install of pybind11 can for example be done using:
```
cd <PYBIND11DIR>
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install -DPYTHON_EXECUTABLE=$(which python)
make
make install
```
and then use the CMake setting `pybind11_DIR=<PYBIND11DIR>/install`.
