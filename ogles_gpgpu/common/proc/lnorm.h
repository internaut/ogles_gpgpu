//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 (http://www.mkonrad.net)
//         David Hirvonen
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU adaptive thresholding processor (two-pass).
 */
#ifndef OGLES_GPGPU_COMMON_PROC_LOCAL_NORM
#define OGLES_GPGPU_COMMON_PROC_LOCAL_NORM

#include "../common_includes.h"

#include "base/multipassproc.h"
#include "multipass/local_norm_pass.h"

namespace ogles_gpgpu {
class LocalNormProc : public MultiPassProc {
public:
    LocalNormProc(float normConst=0.005) {
        LocalNormPass *localNormPass1 = new LocalNormPass(1);
        LocalNormPass *localNormPass2 = new LocalNormPass(2, normConst);

        procPasses.push_back(localNormPass1);
        procPasses.push_back(localNormPass2);

        multiPassInit();
    }

    /**
     * Return the processors name.
     */
    virtual const char *getProcName() {
        return "LocalNormProc";
    }
};
}

#endif
