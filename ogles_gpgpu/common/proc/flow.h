/**
 * GPGPU flow processor.
 */

#ifndef OGLES_GPGPU_COMMON_PROC_FLOW
#define OGLES_GPGPU_COMMON_PROC_FLOW

#include "../common_includes.h"
#include "base/filterprocbase.h"
#include "base/multipassproc.h" // for FlowPipeline
#include "two.h"

#include <memory>

BEGIN_OGLES_GPGPU

// Input: Ix Iy It
// Performs tensor and smoothing operations in the same fragment shader

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
    
    std::string vshaderFlowDynamic;
    std::string fshaderFlowDynamic;
    
    static const char *fshaderFlowSrc;     // fragment shader source
    static const char *fshaderFlowSrcOpt;  // fragment shader source
};


// Explicit access of first and last filter saves a whole bunch of boilerplate virtual API stuff:
// TODO: may make this more like a multipass filter in the future
class FlowPipeline : public MultiPassProc
{
public:
    FlowPipeline(float tau = 0.004f, float strength = 1.0f, bool doGray=false);
    virtual ~FlowPipeline();
    virtual float getStrength() const;
    
    //virtual ProcInterface * first();
    //virtual ProcInterface * last();
    
    virtual ProcInterface* getInputFilter() const;
    virtual ProcInterface* getOutputFilter() const;
    
    virtual int init(int inW, int inH, unsigned int order, bool prepareForExternalInput = false);
    virtual int reinit(int inW, int inH, bool prepareForExternalInput = false);
    virtual int render(int position);
    
    /**
     * Return the processors name.
     */
    virtual const char *getProcName() { return "FlowPipeline"; }
    
protected:
    
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;
};

//##########################################################################
//                    +=> [Ix^2; Ix*Iy; Iy^2; Ix*It] => SMOOTH ===+
// [Ix; Iy; It; A] ==>|                                           | => FLOW
//                    +=> [Ix^2; Ix*Iy; Iy^2; Iy*It] => SMOOTH ===+
//##########################################################################

class FlowImplProc : public FilterProcBase
{
public:
    
    FlowImplProc(bool isX, float strength = 1.0f);
    virtual const char *getProcName() { return "FlowImplProc"; }
    virtual void getUniforms();
    virtual void setUniforms();
    virtual void setStrength(float value) { strength = value; }
private:
    void filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target);
    
    bool isX = false;
    GLint shParamUStrength;
    float strength = 1.f;
    virtual const char *getVertexShaderSource() { return vshaderGPUImage; }
    virtual const char *getFragmentShaderSource() { return isX ? fshaderFlowXSrc : fshaderFlowYSrc; }
    static const char *fshaderFlowXSrc;
    static const char *fshaderFlowYSrc;
};

class Flow2Proc : public TwoInputProc
{
public:
    Flow2Proc(float tau=0.004, float strength = 1.0f);
    virtual const char *getProcName() { return "Flow2Proc"; }
    virtual void filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target);
    virtual void getUniforms();
    virtual void setUniforms();
    
    float getStrength() const { return strength; }
    float getTau() const { return tau; }
    
private:
    
    virtual const char *getFragmentShaderSource() { return fshaderFlowSrc; }
    virtual const char *getVertexShaderSource() { return vshaderGPUImage; }
    
    GLint shParamUTau;
    GLfloat tau = 0.004;
    
    GLint shParamUStrength;
    GLfloat strength = 1.0f;
    
    static const char *fshaderFlowSrc;     // fragment shader source
};

class Flow2Pipeline : public MultiPassProc
{
public:
    
    Flow2Pipeline(float tau = 0.004f, float strength = 1.0f, bool doGray=false);
    virtual ~Flow2Pipeline();
    
    virtual float getStrength() const;
    virtual ProcInterface * corners(); // corner output
    
    virtual ProcInterface* getInputFilter() const;
    virtual ProcInterface* getOutputFilter() const;

    /**
     * Return the processors name.
     */
    virtual const char *getProcName() { return "Flow2Pipeline"; }
    
    virtual int init(int inW, int inH, unsigned int order, bool prepareForExternalInput = false);
    virtual int reinit(int inW, int inH, bool prepareForExternalInput = false);
    virtual int render(int position);
    
protected:
    
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;
};

typedef Flow2Pipeline FlowOptPipeline;
//typedef FlowPipeline FlowOptPipeline;

END_OGLES_GPGPU

#endif // OGLES_GPGPU_COMMON_PROC_FLOW
