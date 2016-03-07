/**
 * GPGPU flow processor.
 */

#ifndef OGLES_GPGPU_COMMON_PROC_FLOW
#define OGLES_GPGPU_COMMON_PROC_FLOW

#include "../common_includes.h"
#include "base/filterprocbase.h"
#include "base/multipassproc.h" // for FlowPipeline
#include <memory>

BEGIN_OGLES_GPGPU

class FlowProc : public FilterProcBase
{
public:
    FlowProc(float tau=0.004, float strength = 1.0f);
    virtual const char *getProcName() { return "FlowProc"; }
    virtual void filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target);
    virtual void getUniforms();
    virtual void setUniforms();
    
    float getStrength() const { return strength; }
    float getTau() const { return tau; }
private:
    
    virtual const char *getFragmentShaderSource() { return fshaderFlowSrc; }
    virtual const char *getVertexShaderSource() { return vshaderGPUImage; }
    
    GLint texelWidthUniform;
    GLint texelHeightUniform;
    
    GLint shParamUTau;
    GLfloat tau = 0.004;
    
    GLint shParamUStrength;
    GLfloat strength = 1.0f;
    
    static const char *fshaderFlowSrc;    // fragment shader source
    static const char *fshaderFlowSrcOpt;   // fragment shader source
};


// Explicit access of first and last filter saves a whole bunch of boilerplate virtual API stuff:
// TODO: may make this more like a multipass filter in the future
class FlowPipeline /* : public FilterProcBase */
{
public:
    
    FlowPipeline(float tau = 0.004f, float strength = 1.0f);
    virtual ~FlowPipeline();
    
    virtual FilterProcBase * first();
    virtual FilterProcBase * last();
    
    /**
     * Return the processors name.
     */
    virtual const char *getProcName() { return "FlowPipeline"; }
    
protected:
    
    struct Impl;
    
    std::unique_ptr<Impl> m_pImpl;
};



END_OGLES_GPGPU

#endif // OGLES_GPGPU_COMMON_PROC_FLOW
