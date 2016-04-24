//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 (http://www.mkonrad.net)
//         David Hirvonen
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU median processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_MEDIAN
#define OGLES_GPGPU_COMMON_PROC_MEDIAN

#include "proc/filter3x3.h"

BEGIN_OGLES_GPGPU
    
/**
 * GPGPU median filter
 */
class MedianProc : public Filter3x3Proc {
public:
    /**
     * Constructor.
     */
    MedianProc();

    /**
     * Return the processors name.
     */
    virtual const char *getProcName() {
        return "MedianProc";
    }

private:
    
    /**
     * Get the fragment shader source.
     */
    virtual const char *getFragmentShaderSource() { return fshaderMedianSrc; }
    
    /**
     * Get uniform indices.
     */
    virtual void getUniforms();
    
    static const char *fshaderMedianSrc;   // fragment shader source
};

END_OGLES_GPGPU

#endif
