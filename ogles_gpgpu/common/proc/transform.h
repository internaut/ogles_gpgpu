//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
//         David Hirvonen <dhirvonen@elucideye.com>
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU transform processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_TRANSFORM
#define OGLES_GPGPU_COMMON_PROC_TRANSFORM

#include "../common_includes.h"

#include "base/filterprocbase.h"

namespace ogles_gpgpu {

/**
 * GPGPU transform processor will apply 2d parametric transformations.
 */
class TransformProc : public FilterProcBase {
public:
    
    enum Interpolation
    {
        BILINEAR,
        BICUBIC
    };
    
    /**
     * Constructor.
     */
    TransformProc();
    
    /**
     * Return the processors name.
     */
    virtual const char *getProcName() { return "TransformProc"; }

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
     * Get the transformation matrix.
     */
    const Mat44f & getTransformMatrix() const { return transformMatrix; }

    /**
     * Set the transformation matrix.
     */
    void setTransformMatrix(const Mat44f & matrix);
    
    /**
     * Set interpolation mode.
     */
    void setInterpolation(Interpolation kind) { interpolation = kind; }
    
    /**
     * Get interpolation mode.
     */
    Interpolation getInterpolatino() const { return interpolation; }
    
    /**
     * Set the transformation matrix.
     */
    virtual void filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target);

private:
    static const char *vshaderTransformSrc;   // fragment shader source
    static const char *fshaderTransformSrc;   // fragment shader source
    static const char *fshaderTransformBicubicSrc; // bicubic shader
    
    Interpolation interpolation = BILINEAR;
    
    GLint shParamUTransform;        // shader uniform transformation matrix
    Mat44f transformMatrix;         // currently set weighted channel grayscale conversion vector
    
    GLint shParamUTransformSize=0;  // texture size (for bicubic warp)
};
}

#endif
