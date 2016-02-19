# This file generated automatically by:
#   generate_sugar_files.py
# see wiki for more info:
#   https://github.com/ruslo/sugar/wiki/Collecting-sources

if(DEFINED OGLES_GPGPU_COMMON_SUGAR_CMAKE_)
  return()
else()
  set(OGLES_GPGPU_COMMON_SUGAR_CMAKE_ 1)
endif()

include(sugar_files)
include(sugar_include)

sugar_include(gl)
sugar_include(proc)

sugar_files(
    OGLES_GPGPU_COMMON_PUBLIC_HDRS
    common_includes.h
    core.h
    macros.h
    tools.h
    types.h
)

sugar_files(
    OGLES_GPGPU_SRCS
    ${OGLES_GPGPU_COMMON_PUBLIC_HDRS}
    core.cpp
    tools.cpp
    types.cpp
)
