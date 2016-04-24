# Copyright (c) 2015, David Hirvonen, Ruslan Baratov
# All rights reserved.

cmake_minimum_required(VERSION 3.0)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.14.3.tar.gz"
    SHA1 "e05bc95f6d62b7c55188044e39e8be5afb1e8af4"
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
string(COMPARE EQUAL "${CMAKE_GENERATOR}" "Xcode" is_xcode)

message("Android: ${is_android}")
message("iOS    : ${is_ios}")
message("Linux  : ${is_linux}")
message("Xcode  : ${is_xcode}")

if(is_xcode)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configuration types")
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

option(OGLES_GPGPU_BUILD_EXAMPLES "Build examples" OFF)
option(OGLES_GPGPU_INSTALL "Perform installation" OFF)

message("BUILD EXAMPLES ${BUILD_EXAMPLES}")
if(OGLES_GPGPU_BUILD_EXAMPLES)
  message("BUILD EXAMPLES.......")
  add_subdirectory(examples)
endif()
