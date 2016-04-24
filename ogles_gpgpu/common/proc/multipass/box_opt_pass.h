//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU boxian smoothing processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_BOX_OPT_PASS
#define OGLES_GPGPU_COMMON_PROC_BOX_OPT_PASS

#include "../../common_includes.h"

#include "../base/filterprocbase.h"

namespace ogles_gpgpu {

/**
 * This filter applies boxian smoothing to an input image.
 */
class BoxOptProcPass : public FilterProcBase {
public:
    /**
     * Construct as render pass <pass> (1 or 2).
     */
    BoxOptProcPass(int pass, float radius)
    : FilterProcBase()
    , renderPass(pass)
    , pxDx(0.0f)
    , pxDy(0.0f)
    {
        setRadius(radius);
        assert(renderPass == 1 || renderPass == 2);
    }
    
    void setRadius(float newValue);

    /**
     * Return the processors name.
     */
    virtual const char *getProcName() { return "BoxOptProcPass"; }

    virtual void filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target);
    virtual void setUniforms();
    virtual void getUniforms();
    virtual const char *getFragmentShaderSource();
    virtual const char *getVertexShaderSource();

private:
    
    int renderPass; // render pass number. must be 1 or 2

    float pxDx;	// pixel delta value for texture access
    float pxDy;	// pixel delta value for texture access

    float _blurRadiusInPixels = 0.0; // start 0 (uninitialized)
    
    GLint shParamUTexelWidthOffset;
    GLint shParamUTexelHeightOffset;
    
    std::string vshaderBoxSrc;
    std::string fshaderBoxSrc;
};

}
#endif
