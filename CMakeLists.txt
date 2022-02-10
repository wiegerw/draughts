project(DRAUGHTS)

cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)

find_package(Python3 COMPONENTS Interpreter Development)
find_package(PythonLibs 3 REQUIRED)
find_package(pybind11 REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(include)

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_library(libscan ${SRC_FILES})

pybind11_add_module(draughts1 ${SRC_FILES} tools/python-bindings.cpp)

add_executable(scan31 tools/scan.cpp)
target_link_libraries(scan31 libscan Threads::Threads)

add_executable(search examples/search.cpp)
target_link_libraries(search libscan Threads::Threads)

# Testing (optional)
option(ENABLE_TESTS "Enable tests" OFF)
if (${ENABLE_TESTS})
    find_package(Boost 1.75 COMPONENTS unit_test_framework REQUIRED)
    include_directories(${Boost_INCLUDE_DIRS} include)
    add_subdirectory(tests)
    enable_testing()
endif()

# Installation
#include(GNUInstallDirs)
#install(TARGETS libdraughts scan31 libscan
#        COMPONENT python
#        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
#        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
#        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")