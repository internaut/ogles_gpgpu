//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU adaptive thresholding processor (two-pass).
 */
#ifndef OGLES_GPGPU_COMMON_PROC_ADAPT_THRESH
#define OGLES_GPGPU_COMMON_PROC_ADAPT_THRESH

#include "../common_includes.h"

#include "base/multipassproc.h"
#include "multipass/adapt_thresh_pass.h"

namespace ogles_gpgpu {
class AdaptThreshProc : public MultiPassProc {
public:
    AdaptThreshProc() {
        AdaptThreshProcPass *adaptThreshPass1 = new AdaptThreshProcPass(1);
        AdaptThreshProcPass *adaptThreshPass2 = new AdaptThreshProcPass(2);
        
        procPasses.push_back(adaptThreshPass1);
        procPasses.push_back(adaptThreshPass2);
        
        multiPassInit();
    }
    
    /**
     * Return the processors name.
     */
    virtual const char *getProcName() { return "AdaptThreshProc"; }
};
}

#endif