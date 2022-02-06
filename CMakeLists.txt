project(DRAUGHTS)

cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)

find_package(Python3 COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(include)

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_library(scan ${SRC_FILES})

pybind11_add_module(draughts tools/python-bindings.cpp)

add_executable(scan31 tools/scan.cpp)
target_link_libraries(scan31 scan)
target_link_libraries(scan31 Threads::Threads)
