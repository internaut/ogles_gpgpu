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
     * Get the fragment shader source.
     */
    virtual const char *getFragmentShaderSource();

    /**
     * Get the vertex shader source.
     */
    virtual const char *getVertexShaderSource();
    
    /**
     * Set additional uniforms.
     */
    virtual void setUniforms();
    
    /**
     * Get uniform indices.
     */
    virtual void getUniforms();

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
    Interpolation getInterpolation() const { return interpolation; }
 
protected:
    
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
