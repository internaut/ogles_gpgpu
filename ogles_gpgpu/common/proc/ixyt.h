/**
 * GPGPU flow processor.
 */

#ifndef OGLES_GPGPU_COMMON_PROC_IXYT
#define OGLES_GPGPU_COMMON_PROC_IXYT

#include "../common_includes.h"
#include "two.h"

namespace ogles_gpgpu {

// ######### Temporal derivative shader #################

class IxytProc : public TwoInputProc
{
public:
    
    IxytProc() {}
    virtual const char *getProcName() { return "IxytProc"; }
    virtual void getUniforms();
    virtual void setUniforms();
private:
    
    GLint texelWidthUniform;
    GLint texelHeightUniform;
    
    virtual const char *getVertexShaderSource() { return vshaderFilter3x3Src; }
    virtual const char *getFragmentShaderSource() { return fshaderIxytSrc; }
    static const char *fshaderIxytSrc;   // fragment shader source
};
}

#endif // OGLES_GPGPU_COMMON_PROC_IXYT
