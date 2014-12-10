//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#ifndef OGLES_GPGPU_ANDROID_MACROS
#define OGLES_GPGPU_ANDROID_MACROS

#include <android/log.h>

#define OG_TO_STR_(x) #x
#define OG_TO_STR(x) OG_TO_STR_(x)

#ifdef DEBUG

#define OG_LOGINF(class, args...)  __android_log_write(ANDROID_LOG_INFO, "ogles_gpgpu", class); __android_log_write(ANDROID_LOG_INFO, "ogles_gpgpu", __FUNCTION__); __android_log_print(ANDROID_LOG_INFO, "ogles_gpgpu", args)
#else
#define OG_LOGINF(class, args...)
#endif

#define OG_LOGERR(class, args...) __android_log_write(ANDROID_LOG_ERROR, "ogles_gpgpu", class); __android_log_write(ANDROID_LOG_ERROR, "ogles_gpgpu", __FUNCTION__); __android_log_print(ANDROID_LOG_ERROR, "ogles_gpgpu", args)

#endif
