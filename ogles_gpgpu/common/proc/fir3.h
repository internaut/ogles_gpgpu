/**
 * GPGPU blend filter for iir processors.
 */

#ifndef OGLES_GPGPU_COMMON_FIR_PROC
#define OGLES_GPGPU_COMMON_FIR_PROC

#include "../common_includes.h"
#include "three.h"

BEGIN_OGLES_GPGPU

class Fir3Proc : public ThreeInputProc
{
public:
    Fir3Proc(bool doRgb = false);
    virtual const char *getProcName() { return "Fir3Proc"; }
    virtual void getUniforms();
    virtual void setUniforms();
    virtual void setWeights(const Vec3f &value)
    {
        weights = value;
    }

    virtual void setWeights(const Vec3f &f1, const Vec3f &f2, const Vec3f &f3)
    {
        weightsRGB[0] = f1;
        weightsRGB[1] = f2;
        weightsRGB[2] = f3;
    }
    
private:
    
    bool doRgb = false;

    GLint shParamUWeights;
    Vec3f weights;
    
    GLint shParamUWeights1;
    GLint shParamUWeights2;
    GLint shParamUWeights3;
    Vec3f weightsRGB[3];
    
    virtual const char *getFragmentShaderSource() { return doRgb ? fshaderFir3RGBSrc : fshaderFir3Src; }
    static const char *fshaderFir3Src;      // fragment shader source
    static const char *fshaderFir3RGBSrc;   // fragment shader source
};

END_OGLES_GPGPU

#endif // OGLES_GPGPU_COMMON_PROC_IXYT
