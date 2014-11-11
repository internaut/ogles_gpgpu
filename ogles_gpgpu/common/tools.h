#ifndef OGLES_GPGPU_COMMON_TOOLS
#define OGLES_GPGPU_COMMON_TOOLS

#define TO_STR_(x) #x
#define TO_STR(x) TO_STR_(x)

namespace ogles_gpgpu {
    
class Tools {
public:
    static void checkGLErr(const char *msg);
};
    
}

#endif