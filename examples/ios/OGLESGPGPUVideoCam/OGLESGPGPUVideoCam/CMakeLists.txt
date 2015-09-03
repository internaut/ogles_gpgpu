# Copyright (c) 2013-2014, Ruslan Baratov, David Hirvonen
# All rights reserved.

cmake_minimum_required(VERSION 3.0.0)

### Emulate toolchain
set(CMAKE_OSX_SYSROOT "iphoneos")
set(CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos;-iphonesimulator")
set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
### -- end

include_directories("${SUGAR_ROOT}/examples/third_party")
include_directories("${CMAKE_SOURCE_DIR}/src/app/ios")

#sugar_include("./")
sugar_include("${SUGAR_ROOT}/examples/resources/ios/icons")
sugar_include("${SUGAR_ROOT}/examples/resources/ios/images")

set(OGLESGPGPU_VIDEOCAM_SRCS
  AppDelegate.h
  AppDelegate.mm
  CamView.h
  CamView.mm
  RootViewController.h
  RootViewController.mm
  main.m
)

set(OGLESGPGPU_VIDEOCAM_RESOURCES ${IOS_ICONS} ${DEFAULT_IOS_IMAGES})

if(NOT XCODE_VERSION)
  sugar_fatal_error("Xcode only")
endif()

add_executable(OGLESGPGPUVideoCam ${OGLESGPGPU_VIDEOCAM_SRCS} ${OGLESGPGPU_VIDEOCAM_RESOURCES})

set_target_properties(
  OGLESGPGPUVideoCam
  PROPERTIES
  MACOSX_BUNDLE YES
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/storyboard.plist.in"
  XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
  XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2" # Universal (iPad + iPhone)
  # http://stackoverflow.com/a/20982506/2288008
  XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
  XCODE_ATTRIBUTE_INSTALL_PATH "${CMAKE_BINARY_DIR}/ProductRelease"
  # By default this setting is empty =>
  # archives not showing up in Xcode organizer.
  # http://stackoverflow.com/a/8102602/2288008
  XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES "NO" # If this setting is "YES"
  # application can't pass archive validation.
  # http://stackoverflow.com/a/24040412/2288008
  RESOURCE "${RESOURCES}"
)

set_target_properties(
  OGLESGPGPUVideoCam
  PROPERTIES
  XCODE_ATTRIBUTE_PRODUCT_NAME
  "OGLESGPGPUVideoCam"
  XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER
  "com.elucideye.OGLESGPGPUVideoCam.TestOpencv"
)

set_target_properties(
  OGLESGPGPUVideoCam
  PROPERTIES
  XCODE_ATTRIBUTE_PRODUCT_NAME[variant=Debug]
  "OGLESGPGPUVideoCam-Dbg"
  XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER[variant=Debug]
  "com.elucideye.OGLESGPGPUVideoCam.debug"
  )

target_link_libraries(
  OGLESGPGPUVideoCam
  ogles_gpgpu
  "-framework CoreGraphics"
  "-framework Foundation"
  "-framework UIKit"
  "-framework QuartzCore"
  "-framework ImageIO"
  "-framework CoreGraphics"
  "-framework CoreVideo"
  "-framework CoreMedia"
  "-framework CoreLocation" 
  "-framework AVFoundation"
  "-framework GLKit"
  "-framework Accelerate"
  "-framework OpenGLES"
  "-framework UIKit"
  )

