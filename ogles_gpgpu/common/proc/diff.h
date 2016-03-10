/**
 * GPGPU flow processor.
 */

#ifndef OGLES_GPGPU_COMMON_PROC_DIFF
#define OGLES_GPGPU_COMMON_PROC_DIFF

#include "../common_includes.h"
#include "two.h"

namespace ogles_gpgpu {

// ######### Temporal derivative shader #################

class DiffProc : public TwoInputProc
{
public:
    
    DiffProc(float strength = 1.0f, bool squared=false) : strength(strength), squared(squared) {}
    virtual const char *getProcName() { return "DiffProc"; }
    virtual void getUniforms();
    virtual void setUniforms();
    virtual void setStrength(float value) { strength = value; }
private:
    
    GLint texelWidthUniform;
    GLint texelHeightUniform;
    
    GLint shParamUStrength;
    float strength = 1.f;
    
    bool squared = false;
    
    virtual const char *getVertexShaderSource() { return vshaderGPUImage; }
    virtual const char *getFragmentShaderSource() { return squared ? fshaderDiff2Src : fshaderDiffSrc; }
    static const char *fshaderDiffSrc;   // fragment shader source (diff)
    static const char *fshaderDiff2Src;  // fragment shader source (squared diff)
};
}

#endif // OGLES_GPGPU_COMMON_PROC_DIFF
