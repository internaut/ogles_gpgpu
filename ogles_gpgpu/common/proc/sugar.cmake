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
    fifo.cpp
    fifo.h
    filter3x3.cpp
    filter3x3.h
    gauss.h
    gauss_opt.h
    grad.cpp
    grad.h
    grayscale.cpp
    grayscale.h
    lnorm.h
    lbp.cpp
    lbp.h
    nms.cpp
    nms.h
    pyramid.cpp
    pyramid.h
    shitomasi.cpp # corner
    shitomasi.h
    tensor.cpp # tensor
    tensor.h
    thresh.cpp
    thresh.h
    transform.cpp
    transform.h
    two.cpp
    two.h
    video.cpp # image source
    video.h
    yuv2rgb.cpp
    yuv2rgb.h
)
