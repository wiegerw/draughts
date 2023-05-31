# draughts
The draughts library is a C++ library for international draughts. It contains Python bindings for the strong draughts program Scan 3.1.

This library is used in master thesis projects aimed at the application of AlphaZero like techniques to draughts.

## Requirements
A C++17 compiler.

## Build
The following build systems are supported
* CMake 3.15+

### Python bindings
Draughts comes with python bindings. For this the package pybind11 is needed, see 
https://github.com/pybind/pybind11.

On Windows a recent compiler is needed (e.g. Visual Studio 2019). 

The python bindings can for example be installed using the command `pip3 install . --user`