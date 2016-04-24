/**
 * GPGPU flow processor.
 */

#ifndef OGLES_GPGPU_COMMON_REMAP_PROC
#define OGLES_GPGPU_COMMON_REMAP_PROC

#include "two.h"

namespace ogles_gpgpu {

class RemapProc : public TwoInputProc
{
public:
    RemapProc();
    virtual const char *getProcName() { return "RemapProc"; }
private:
    
    void filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target);
    void getUniforms();
    void setUniforms();
    
    GLint texelWidthUniform;
    GLint texelHeightUniform;
    
    virtual const char *getVertexShaderSource() { return vshaderRemapSrc; }
    virtual const char *getFragmentShaderSource() { return fshaderRemapSrc; }
    
    static const char *vshaderRemapSrc;
    static const char *fshaderRemapSrc;
};
}

#endif // OGLES_GPGPU_COMMON_REMAP_PROC
