/**
 * GPGPU gaussian smoothing processor (two-pass).
 */
#ifndef OGLES_GPGPU_COMMON_PROC_GAUSS
#define OGLES_GPGPU_COMMON_PROC_GAUSS

#include "../common_includes.h"

#include "base/multipassproc.h"
#include "multipass/gauss_pass.h"

namespace ogles_gpgpu {
class GaussProc : public MultiPassProc {
public:
    GaussProc() {
        GaussProcPass *gaussPass1 = new GaussProcPass(1);
        GaussProcPass *gaussPass2 = new GaussProcPass(2);
        
        procPasses.push_back(gaussPass1);
        procPasses.push_back(gaussPass2);
        
        multiPassInit();
    }
};
}

#endif