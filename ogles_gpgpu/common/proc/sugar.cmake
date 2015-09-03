# This file generated automatically by:
#   generate_sugar_files.py
# see wiki for more info:
#   https://github.com/ruslo/sugar/wiki/Collecting-sources

if(DEFINED OGLES_GPGPU_COMMON_PROC_SUGAR_CMAKE_)
  return()
else()
  set(OGLES_GPGPU_COMMON_PROC_SUGAR_CMAKE_ 1)
endif()

include(sugar_files)
include(sugar_include)

sugar_include(base)
sugar_include(multipass)

sugar_files(
    OGLES_GPGPU_SRCS
    adapt_thresh.h
    disp.cpp
    disp.h
    gauss.h
    grayscale.cpp
    grayscale.h
    thresh.cpp
    thresh.h
)
