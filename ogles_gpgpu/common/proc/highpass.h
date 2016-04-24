#ifndef OGLES_GPGPU_COMMON_HIGH_PASS_PROC
#define OGLES_GPGPU_COMMON_HIGH_PASS_PROC

#include "iir.h"

BEGIN_OGLES_GPGPU

class HighPassFilterProc : public IirFilterProc
{
public:
    HighPassFilterProc(float alpha=0.5) : IirFilterProc(IirFilterProc::kHighPass, alpha) {}
    ~HighPassFilterProc() {}
    virtual const char *getProcName() { return "HighPassFilterProc"; }
};

END_OGLES_GPGPU

#endif 
