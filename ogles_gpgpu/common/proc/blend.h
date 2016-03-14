/**
 * GPGPU blend filter for iir processors.
 */

#ifndef OGLES_GPGPU_COMMON_BLEND_PROC
#define OGLES_GPGPU_COMMON_BLEND_PROC

#include "../common_includes.h"
#include "two.h"

BEGIN_OGLES_GPGPU

class BlendProc : public TwoInputProc
{
public:
    BlendProc(float alpha = 1.0f);
    virtual const char *getProcName() { return "BlendProc"; }
    virtual void getUniforms();
    virtual void setUniforms();
    virtual void setAlpha(float value) { alpha = value; }
    virtual int render(int position = 0);

private:
    
    GLint shParamUAlpha;
    GLfloat alpha = 0.5f;
    
    virtual const char *getFragmentShaderSource() { return fshaderBlendSrc; }
    static const char *fshaderBlendSrc;   // fragment shader source
};

END_OGLES_GPGPU

#endif // OGLES_GPGPU_COMMON_PROC_IXYT
