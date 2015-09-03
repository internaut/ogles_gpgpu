# This file generated automatically by:
#   generate_sugar_files.py
# see wiki for more info:
#   https://github.com/ruslo/sugar/wiki/Collecting-sources

if(DEFINED OGLES_GPGPU_COMMON_GL_SUGAR_CMAKE_)
  return()
else()
  set(OGLES_GPGPU_COMMON_GL_SUGAR_CMAKE_ 1)
endif()

include(sugar_files)

sugar_files(
    OGLES_GPGPU_SRCS
    fbo.cpp
    fbo.h
    memtransfer.cpp
    memtransfer.h
    memtransfer_factory.cpp
    memtransfer_factory.h
    memtransfer_optimized.h
    shader.cpp
    shader.h
)
