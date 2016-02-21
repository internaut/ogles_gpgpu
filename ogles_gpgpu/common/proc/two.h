//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU grayscale processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_TWOINPUT
#define OGLES_GPGPU_COMMON_PROC_TWOINPUT

#include "../common_includes.h"
#include "base/filterprocbase.h"

namespace ogles_gpgpu {

/**
 * GPGPU TwoInput texture buffer
 */
class TwoInputProc : public FilterProcBase {
public:
    /**
     * Constructor.
     */
    TwoInputProc();

    /**
     * Return the processors name.
     */
    virtual const char *getProcName() {
        return "TwoInputProc";
    }
    
    /**
     * render
     */
    virtual void render();
    
    /**
     * init
     */    
    virtual int init(int inW, int inH, unsigned int order, bool prepareForExternalInput);
    
private:
    
    /**
     * Get the fragment shader source.
     */
    virtual const char *getFragmentShaderSource() { return fshaderTwoInputSrc; }
    
    /**
     * Set additional uniforms.
     */
    virtual void setUniforms();
    
    /**
     * Get uniform indices.
     */
    virtual void getUniforms();

    static const char *fshaderTwoInputSrc; // fragment shader source

    static const char *vshaderTwoInputSrc; // vertex shader source
};
}

#endif
