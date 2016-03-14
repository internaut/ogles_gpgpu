#ifndef OGLES_GPGPU_COMMON_LOW_PASS_PROC
#define OGLES_GPGPU_COMMON_LOW_PASS_PROC

#include "iir.h"

BEGIN_OGLES_GPGPU

class LowPassFilterProc : public IirFilterProc
{
public:
    LowPassFilterProc(float alpha=0.5) : IirFilterProc(IirFilterProc::kLowPass, alpha) {}
    ~LowPassFilterProc() {}
    virtual const char *getProcName() { return "LowPassFilterProc"; }
};

END_OGLES_GPGPU

#endif 
