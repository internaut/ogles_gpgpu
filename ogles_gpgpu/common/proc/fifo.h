//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU grayscale processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_FIFO
#define OGLES_GPGPU_COMMON_PROC_FIFO

#include "../common_includes.h"
#include "base/multiprocinterface.h"
#include "gl/fbo.h"

BEGIN_OGLES_GPGPU
    
class FifoProc : public MultiProcInterface {
public:
    
    FifoProc(int size=2);
    
    virtual ~FifoProc();
    virtual int init(int inW, int inH, unsigned int order, bool prepareForExternalInput = false);
    virtual int reinit(int inW, int inH, bool prepareForExternalInput = false);
    virtual void cleanup();    
    virtual const char *getProcName() { return "FifoProc"; }
    virtual void createFBOTex(bool genMipmap);
    virtual int render(int position = 0);
    virtual void useTexture(GLuint id, GLuint useTexUnit = 1, GLenum target = GL_TEXTURE_2D, int position = 0);
    virtual void setOutputRenderOrientation(RenderOrientation o) { assert(false); }
    virtual RenderOrientation getOutputRenderOrientation() const { return RenderOrientationNone; }
    virtual void setOutputSize(float scaleFactor);
    virtual void setOutputSize(int outW, int outH);
    
    virtual ProcInterface* getInputFilter() const;
    virtual ProcInterface* getOutputFilter() const;
    virtual ProcInterface * operator[](int i) const { return procPasses[i]; }    
    virtual size_t size() const { return procPasses.size(); }
    
    virtual int getIn() const;
    virtual int getOut() const;
    
    /**
     * Return te list of processor instances of each pass of this multipass processor.
     */
    std::vector<ProcInterface *>& getProcPasses() { return procPasses; }
    const std::vector<ProcInterface *>& getProcPasses() const { return procPasses; }
    bool isFull() const { return m_count == size(); }
    size_t getBufferCount() const { return m_count; }

protected:
    
    int m_count = 0;
    int m_inputIndex = -1;
    int m_outputIndex = -1;
    
    std::vector<ProcInterface *> procPasses;   // holds all instances to the single processing passes. strong ref!
};

typedef FifoProc FIFOPRoc;
    
END_OGLES_GPGPU

#endif
