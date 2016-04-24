# This file generated automatically by:
#   generate_sugar_files.py
# see wiki for more info:
#   https://github.com/ruslo/sugar/wiki/Collecting-sources

if(DEFINED OGLES_GPGPU_SUGAR_CMAKE_)
  return()
else()
  set(OGLES_GPGPU_SUGAR_CMAKE_ 1)
endif()

include(sugar_files)
include(sugar_include)

sugar_include(common)
sugar_include(platform)

sugar_files(
    OGLES_GPGPU_PUBLIC_HDRS
    ogles_gpgpu.h
)
