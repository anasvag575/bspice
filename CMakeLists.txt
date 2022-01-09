cmake_minimum_required (VERSION 3.9)
MESSAGE(STATUS "CMAKE_ROOT: " ${CMAKE_ROOT})

# Project name
project(bspice LANGUAGES C CXX)

# CMake parameters
set(USE_WERROR ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_COMPILER "g++")

# Base Compilation parameters and flags
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -march=native -O4 -flto -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -march=native -O2 -Wall")

######################################
#### Parameters for the simulator ####
######################################
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBSPICE_EIGEN_USE_STLMAPS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBSPICE_EIGEN_USE_KLU")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/circuit_elements)
include_directories(${PROJECT_SOURCE_DIR}/src/util)
include_directories(${PROJECT_SOURCE_DIR}/src/plot)
include_directories(${PROJECT_SOURCE_DIR}/src/simulator)
include_directories(${PROJECT_SOURCE_DIR}/src/lib/)
include_directories(${PROJECT_SOURCE_DIR}/src/lib/Eigen)
include_directories(${PROJECT_SOURCE_DIR}/src/lib/tsl)

# For Suitesparse and Eigen
include_directories(/usr/include/suitesparse/)

#Output to the console some info
message(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
message(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER})
message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_EXE_LINKER_FLAGS: " ${CMAKE_EXE_LINKER_FLAGS})

# Dependency libs for the project
# Suitesparse, Boost
#set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(OpenMP REQUIRED)
#find_package(suitesparse)

# Source code
add_library(circuit_lib src/circuit_elements/circuit.cpp src/util/parser.cpp)
add_library(plot_lib src/plot/plot.cpp)
add_library(simulator_lib src/simulator/mna.cpp src/simulator/sim_engine.cpp)

# Set up the executable
add_executable(bspice src/bspice.cpp)

# Here you can add dynamic libraries 
target_link_libraries(bspice 
	circuit_lib
	plot_lib 
	simulator_lib 
	OpenMP::OpenMP_CXX
	klu
	btf)

