# This file generated automatically by:
#   generate_sugar_files.py
# see wiki for more info:
#   https://github.com/ruslo/sugar/wiki/Collecting-sources

if(DEFINED OGLES_GPGPU_COMMON_PROC_MULTIPASS_SUGAR_CMAKE_)
  return()
else()
  set(OGLES_GPGPU_COMMON_PROC_MULTIPASS_SUGAR_CMAKE_ 1)
endif()

include(sugar_files)

sugar_files(
    OGLES_GPGPU_SRCS
    adapt_thresh_pass.cpp
    adapt_thresh_pass.h
    gauss_pass.cpp
    gauss_pass.h
    gauss_opt_pass.cpp
    gauss_opt_pass.h
    local_norm_pass.cpp
    local_norm_pass.h
)
