#ifndef OGLES_GPGPU_COMMON_PROC_MULTIPROCINTERFACE
#define OGLES_GPGPU_COMMON_PROC_MULTIPROCINTERFACE

#include "procinterface.h"

BEGIN_OGLES_GPGPU

/**
 * Inteface for common multi filter modules: MultiPass, FIFO, etc
 */
    
class MultiProcInterface : public ProcInterface {
  
public:
    
    // ######## First/last filter access:
    MultiProcInterface() {}
    virtual ~MultiProcInterface() {}
    
    virtual ProcInterface* getInputFilter() const = 0;
    virtual ProcInterface* getOutputFilter() const = 0;
    virtual ProcInterface * operator[](int i) const = 0;
    virtual size_t size() const = 0;
    virtual void printInfo();
    
    // ######## Default implementations for

    virtual void setOutputRenderOrientation(RenderOrientation o);
    virtual RenderOrientation getOutputRenderOrientation() const;    
    
    virtual void setExternalInputDataFormat(GLenum fmt);
    virtual void setExternalInputData(const unsigned char *data);
    virtual GLuint getTextureUnit() const;
    virtual void setOutputSize(float scaleFactor);
    virtual void setOutputSize(int outW, int outH);
    virtual int getOutFrameW() const;
    virtual int getOutFrameH() const;
    virtual int getInFrameW() const;
    virtual int getInFrameH() const;
    virtual bool getWillDownscale() const;
    virtual void getResultData(unsigned char *data) const;
    virtual void getResultData(FrameDelegate &delegate) const;
    virtual MemTransfer *getMemTransferObj() const;
    virtual MemTransfer *getInputMemTransferObj() const;
    virtual GLuint getInputTexId() const;
    virtual GLuint getOutputTexId() const;
};

END_OGLES_GPGPU

#endif
    
