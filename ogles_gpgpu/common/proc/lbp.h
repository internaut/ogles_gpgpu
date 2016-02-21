//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 (http://www.mkonrad.net)
//         David Hirvonen
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU lbp processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_LBP
#define OGLES_GPGPU_COMMON_PROC_LBP

#include "proc/filter3x3.h"

namespace ogles_gpgpu {

/**
 * GPGPU lbpient, lbpient magnitude and orientation 
 */
class LbpProc : public Filter3x3Proc {
public:
    /**
     * Constructor.
     */
    LbpProc();

    /**
     * Return the processors name.
     */
    virtual const char *getProcName() {
        return "LbpProc";
    }

private:
    
    /**
     * Get the fragment shader source.
     */
    virtual const char *getFragmentShaderSource() { return fshaderLbpSrc; }
    
    /**
     * Get uniform indices.
     */
    virtual void getUniforms();
    
    static const char *fshaderLbpSrc;   // fragment shader source
};
}

#endif
