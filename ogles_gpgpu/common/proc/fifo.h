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
#ifndef OGLES_GPGPU_COMMON_PROC_FIFO
#define OGLES_GPGPU_COMMON_PROC_FIFO

#include "../common_includes.h"
#include "base/filterprocbase.h"
#include "gl/fbo.h"

namespace ogles_gpgpu {

/**
 * GPGPU FIFO texture buffer
 */
class FIFOProc : public FilterProcBase {
public:
    /**
     * Constructor.
     */
    FIFOProc();

    /**
     * Return the processors name.
     */
    virtual const char *getProcName() {
        return "FIFOProc";
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
    virtual const char *getFragmentShaderSoure() { return fshaderFIFOSrc; }
    
    /**
     * Set additional uniforms.
     */
    virtual void setUniforms();
    
    /**
     * Get uniform indices.
     */
    virtual void getUniforms();

    static const char *fshaderFIFOSrc;  // fragment shader source

    std::shared_ptr<FBO> prevFBO;
};
}

#endif
