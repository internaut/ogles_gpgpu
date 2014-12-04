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
