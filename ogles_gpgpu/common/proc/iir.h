#ifndef OGLES_GPGPU_COMMON_IIR_PROC
#define OGLES_GPGPU_COMMON_IIR_PROC

#include "base/multipassproc.h"
#include <memory>

BEGIN_OGLES_GPGPU

class IirFilterProc : public MultiPassProc
{
public:

    enum FilterKind
    {
        kLowPass,
        kHighPass
    };
    
    class Impl;
    
    IirFilterProc(FilterKind kind, float alpha=0.5, float strength=1.f);
    ~IirFilterProc();
    virtual void useTexture(GLuint id, GLuint useTexUnit = 1, GLenum target = GL_TEXTURE_2D, int position=0);
    virtual int render(int position=0);
    virtual const char *getProcName() { return "IirFilterProc"; }
    virtual ProcInterface* getInputFilter() const;
    virtual ProcInterface* getOutputFilter() const;
    
    virtual int init(int inW, int inH, unsigned int order, bool prepareForExternalInput = false);
    virtual int reinit(int inW, int inH, bool prepareForExternalInput = false);
    
    bool isFirst = true;
    std::unique_ptr<Impl> m_impl;
};

END_OGLES_GPGPU

#endif 
