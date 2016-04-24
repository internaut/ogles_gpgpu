# This file generated automatically by:
#   generate_sugar_files.py
# see wiki for more info:
#   https://github.com/ruslo/sugar/wiki/Collecting-sources

if(DEFINED OGLES_GPGPU_PLATFORM_SUGAR_CMAKE_)
  return()
else()
  set(OGLES_GPGPU_PLATFORM_SUGAR_CMAKE_ 1)
endif()

include(sugar_include)

if(is_ios)
  sugar_include(ios)
elseif(APPLE)
  sugar_include(osx)
elseif(ANDROID)
  sugar_include(android)
else()
  message("include opengl platforms.............")
  sugar_include(opengl)
endif()



