//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 (http://www.mkonrad.net)
//         David Hirvonen <dhirvonen@elucideye.com>
// 
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU filter3x3 processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_FILTER3X3
#define OGLES_GPGPU_COMMON_PROC_FILTER3X3

#include "../common_includes.h"

#include "base/filterprocbase.h"

namespace ogles_gpgpu {

/**
 * GPGPU filter3x3 processor will apply 2d parametric filter3x3ations.
 */
class Filter3x3Proc : public FilterProcBase {
public:
    
    /**
     * Constructor.
     */
    Filter3x3Proc();
    
    /**
     * Return the processors name.
     */
    virtual const char *getProcName() { return "Filter3x3Proc"; }

    /**
     * Init the processor for input frames of size <inW>x<inH> which is at
     * position <order> in the processing pipeline.
     */
    virtual int init(int inW, int inH, unsigned int order, bool prepareForExternalInput = false);

    /**
     * Render the output.
     */
    virtual void render();

    /**
     * Setup the shaders
     */
    virtual void filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target);

    void setTexelWidth(float width)
    {
        hasOverriddenImageSizeFactor = true;
        texelWidth = width;
    }
    
    void setTexelHeight(float height)
    {
        hasOverriddenImageSizeFactor = true;
        texelHeight = height;
    }
    
    void reset()
    {
        hasOverriddenImageSizeFactor = false;
    }
    
protected:
    
    bool hasOverriddenImageSizeFactor = false;
    GLint texelWidthUniform, texelHeightUniform;
    float texelWidth, texelHeight;
    
    static const char *vshaderFilter3x3Src;   // fragment shader source
    static const char *fshaderFilter3x3Src;   // fragment shader source
};
}

#endif
