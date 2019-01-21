cmake_minimum_required(VERSION 2.8)
project(argos3_examples)

# Set the path where CMake will find additional scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# Create compilation database (for autocomplete)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find the ARGoS package
find_package(PkgConfig)
#find_package(Lua51 REQUIRED) 
pkg_check_modules(ARGOS REQUIRED argos3_simulator)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ARGOS_PREFIX}/share/argos3/cmake)
message(STATUS "CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")

# Check whether all the necessary libs have been installed to compile the
# code that depends on Qt and OpenGL
#include(ARGoSCheckQTOpenGL)

# Set ARGoS include dir
set(INTERFACE_DIR interface)
set(CONTROLLER_DIR controllers)

include_directories(${CMAKE_SOURCE_DIR} ${ARGOS_INCLUDE_DIRS} ${GSL_INCLUDE_DIR} ${CONTROLLER_DIR} ${INTERFACE_DIR})

# Set ARGoS link dir
link_directories(${ARGOS_LIBRARY_DIRS})

add_subdirectory(interface)
add_subdirectory(controllers)
add_subdirectory(loop_functions)
