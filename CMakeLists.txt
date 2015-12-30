# Copyright (c) 2015, David Hirvonen, Ruslan Baratov
# All rights reserved.

cmake_minimum_required(VERSION 3.0)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.12.33.tar.gz"
    SHA1 "f64eda11326e12d84b6aa7f4aed80c09c182aa64"
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

message("BUILD EXAMPLES ??????")

option(BUILD_EXAMPLES "Build examples" ON)

message("BUILD EXAMPLES ${BUILD_EXAMPLES}")
if(BUILD_EXAMPLES)
  message("BUILD EXAMPLES.......")
  add_subdirectory(examples)
endif()
