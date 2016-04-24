# This file generated automatically by:
#   generate_sugar_files.py
# see wiki for more info:
#   https://github.com/ruslo/sugar/wiki/Collecting-sources

if(DEFINED OGLES_GPGPU_PLATFORM_ANDROID_SUGAR_CMAKE_)
  return()
else()
  set(OGLES_GPGPU_PLATFORM_ANDROID_SUGAR_CMAKE_ 1)
endif()

include(sugar_files)

sugar_files(
    OGLES_GPGPU_SRCS
    egl.cpp
    egl.h
    gl_includes.h
    macros.h
    memtransfer_android.cpp
    memtransfer_android.h
)
