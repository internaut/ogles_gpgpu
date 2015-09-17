# This file generated automatically by:
#   generate_sugar_files.py
# see wiki for more info:
#   https://github.com/ruslo/sugar/wiki/Collecting-sources

if(DEFINED OGLES_GPGPU_PLATFORM_OSX_SUGAR_CMAKE_)
  return()
else()
  set(OGLES_GPGPU_PLATFORM_OSX_SUGAR_CMAKE_ 1)
endif()

include(sugar_files)

sugar_files(
    OGLES_GPGPU_SRCS
    gl_includes.h
    memtransfer_osx.cpp
    memtransfer_osx.h
)
