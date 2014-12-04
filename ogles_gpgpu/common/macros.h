#ifndef OGLES_GPGPU_COMMON_MACROS
#define OGLES_GPGPU_COMMON_MACROS

#define OG_TO_STR_(x) #x
#define OG_TO_STR(x) OG_TO_STR_(x)

#ifdef DEBUG
#define OG_LOGINF(class, args...) fprintf(stdout, "ogles_gpgpu::%s - %s - ", class, __FUNCTION__); fprintf(stdout, args); fprintf(stdout, "\n")
#else
#define OG_LOGINF(class, args...)
#endif

#define OG_LOGERR(class, args...) fprintf(stderr, "ogles_gpgpu::%s - %s - ", class, __FUNCTION__); fprintf(stderr, args); fprintf(stderr, "\n")

#endif
