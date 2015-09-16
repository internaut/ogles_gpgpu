# Copyright (c) 2015, David Hirvonen, Ruslan Baratov
# All rights reserved.

cmake_minimum_required(VERSION 3.0)

### Hunter snapshot that will be used ###
include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.8.13.tar.gz"
    SHA1 "7103f0ab481523ca93a06c1c55abcd56741d3a97"
)

project(ogles_gpgpu VERSION 0.1.0) # Don't see version #?

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# See: cmake/Hunter/config.cmake
hunter_add_package(Sugar)
include("${SUGAR_ROOT}/cmake/Sugar")
include(sugar_include)

set(is_android ${ANDROID}) # syntax compatibility
string(COMPARE EQUAL "${CMAKE_OSX_SYSROOT}" "iphoneos" is_ios)
string(COMPARE EQUAL "${CMAKE_SYSTEM_NAME}" "Linux" is_linux)
string(COMPARE EQUAL "${CMAKE_BUILD_TYPE}" "DEBUG" is_debug)
string(COMPARE EQUAL "${CMAKE_GENERATOR}" "Xcode" is_xcode)

message("ANDROID ${ANDROID}")
message("is_ios ${is_ios}")
message("is_linux ${is_linux}")
message("is_xcode ${is_xcode}")

if(is_xcode)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
endif()

## #################################################################
## Dependencies - OpenGL stuff
## #################################################################

## #################################################################
## Project
## #################################################################

add_subdirectory(ogles_gpgpu)

## #################################################################
## Examples
## #################################################################

message("BUILD EXAMPLES ??????")

option(BUILD_EXAMPLES "Build examples" ON)

message("BUILD EXAMPLES ${BUILD_EXAMPLES}")
if(BUILD_EXAMPLES)
  message("BUILD EXAMPLES.......")
  add_subdirectory(examples)
endif()
