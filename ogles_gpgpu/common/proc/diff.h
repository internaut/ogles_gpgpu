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
    DiffProc(float strength = 1.f);
    virtual const char *getProcName() { return "DiffProc"; }
    virtual void getUniforms();
    virtual void setUniforms();
    virtual void setStrength(float value) { strength = value; }
    virtual int render(int position = 0);
private:
    GLuint shParamUStrength;
    float strength;
    virtual const char *getVertexShaderSource() { return vshaderGPUImage; }
    virtual const char *getFragmentShaderSource() { return fshaderDiffSrc; }
    static const char *fshaderDiffSrc;   // fragment shader source (diff)
};
}

#endif // OGLES_GPGPU_COMMON_PROC_DIFF
