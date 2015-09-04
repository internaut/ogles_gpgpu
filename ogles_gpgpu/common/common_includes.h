//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * Common header includes
 */

#include <iostream>
#include <cassert>

// Need initial preprocessor pass for nested includes

#if __APPLE__
#  include "TargetConditionals.h"
#  if (TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR) || TARGET_OS_IPHONE
#    define OGLES_GPGPU_IOS 1
#  endif
#elif defined(__ANDROID__) || defined(ANDROID)
#  define OGLES_GPGPU_ANDROID 1
#endif

#ifdef OGLES_GPGPU_IOS
#  include "../platform/ios/gl_includes.h" 
#  include "macros.h" 
#elif OGLES_GPGPU_ANDROID
#  error  no way
#  include "../platform/android/gl_includes.h" 
#  include "../platform/android/macros.h" 
#  include "../platform/android/egl.h" 
#else
#  include "../platform/opengl/gl_includes.h"
#  include "macros.h" 
#endif

/* #ifdef __APPLE__ */
/*     #include "../platform/ios/gl_includes.h" */
/* 	#include "macros.h" */
/* #elif __ANDROID__ */
/*     #include "../platform/android/gl_includes.h" */
/* 	#include "../platform/android/macros.h" */
/* 	#include "../platform/android/egl.h" */
/* #else */
/* #error platform not supported. either __APPLE__ or __ANDROID__ must be defined. */
/* #endif */


#include "tools.h"
#include "types.h"
