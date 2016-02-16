//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 (http://www.mkonrad.net)
//         David Hirvonen
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU grad processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_GRAD
#define OGLES_GPGPU_COMMON_PROC_GRAD

#include "proc/filter3x3.h"

namespace ogles_gpgpu {

/**
 * GPGPU gradient, gradient magnitude and orientation 
 */
class GradProc : public Filter3x3Proc {
public:
    /**
     * Constructor.
     */
    GradProc();

    /**
     * Return the processors name.
     */
    virtual const char *getProcName() {
        return "GradProc";
    }

    /**
     * Init the processor for input frames of size <inW>x<inH> which is at
     * position <order> in the processing pipeline.
     */
    virtual int init(int inW, int inH, unsigned int order, bool prepareForExternalInput = false);

    /**
     * Render the output.
     */
    virtual void render();

private:

    
};
}

#endif
