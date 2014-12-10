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

#ifdef __APPLE__
    #include "../platform/ios/gl_includes.h"
	#include "macros.h"
#elif __ANDROID__
    #include "../platform/android/gl_includes.h"
	#include "../platform/android/macros.h"
	#include "../platform/android/egl.h"
#else
#error platform not supported. either __APPLE__ or __ANDROID__ must be defined.
#endif

#include "tools.h"
#include "types.h"
