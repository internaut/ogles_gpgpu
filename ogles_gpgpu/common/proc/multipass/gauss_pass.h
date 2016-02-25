//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU gaussian smoothing processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_GAUSS_PASS
#define OGLES_GPGPU_COMMON_PROC_GAUSS_PASS

#include "../../common_includes.h"

#include "../base/filterprocbase.h"

namespace ogles_gpgpu {

/**
 * This filter applies gaussian smoothing to an input image.
 */
class GaussProcPass : public FilterProcBase {
public:
    /**
     * Construct as render pass <pass> (1 or 2).
     */
    GaussProcPass(int pass, bool doR=false)
    : FilterProcBase()
    , renderPass(pass)
    , texelWidth(0.0f)
    , texelHeight(0.0f)
    , doR(doR)
    {  assert(renderPass == 1 || renderPass == 2); }

    /**
     * Return the processors name.
     */
    virtual const char *getProcName() {
        return "GaussProcPass";
    }

    virtual void filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target);
    virtual void setUniforms();
    virtual void getUniforms();
    virtual const char *getFragmentShaderSource();
    virtual const char *getVertexShaderSource();
    
private:
    int renderPass; // render pass number. must be 1 or 2

    bool doR = false; // do r channel only
    
    GLint texelWidthUniform, texelHeightUniform;
    float texelWidth, texelHeight;
    
    static const char *vshaderGauss7Src;
    static const char *fshaderGauss7Src;  // fragment shader source for gaussian smoothing for both passes
    static const char *fshaderGauss7SrcR; // shader for R channel
    
    static const char *vshaderGauss5Src;
    static const char *fshaderGauss5Src;  // fragment shader source for gaussian smoothing for both passes
    static const char *fshaderGauss5SrcR; // shader for R channel
};

}
#endif
