//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU gaussian smoothing processor (two-pass).
 */
#ifndef OGLES_GPGPU_COMMON_PROC_OPT
#define OGLES_GPGPU_COMMON_PROC_OPT

#include "../common_includes.h"

#include "base/multipassproc.h"
#include "multipass/gauss_opt_pass.h"

namespace ogles_gpgpu {
class GaussOptProc : public MultiPassProc {
public:
    GaussOptProc(float blurRadius = 5.0) {
        GaussOptProcPass *gaussPass1 = new GaussOptProcPass(1, blurRadius);
        GaussOptProcPass *gaussPass2 = new GaussOptProcPass(2, blurRadius);

        procPasses.push_back(gaussPass1);
        procPasses.push_back(gaussPass2);

        multiPassInit();
    }

    /**
     * Return the processors name.
     */
    virtual const char *getProcName() {
        return "GaussOptProc";
    }
};
}

#endif
