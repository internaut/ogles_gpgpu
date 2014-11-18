#ifndef OGLES_GPGPU_IOS_GL_MEMTRANSFER_IOS
#define OGLES_GPGPU_IOS_GL_MEMTRANSFER_IOS

#include "../../common/gl/memtransfer.h"

#import <CoreFoundation/CoreFoundation.h>

namespace ogles_gpgpu {
    
typedef enum {
    BUF_TYPE_INPUT = 0,
    BUF_TYPE_OUTPUT
} BufType;

class MemTransferIOS : public MemTransfer {
public:
    ~MemTransferIOS();
    
    virtual void init();
    
    virtual void prepare(int inTexW, int inTexH, int outTexW, int outTexH, GLenum inputPxFormat = GL_RGBA);
    
    virtual void setInputTexId(GLuint texId) { /* does nothing */ }
    virtual void setOutputTexId(GLuint texId) { /* does nothing */ }
    
    virtual void toGPU(const unsigned char *buf);
    virtual void fromGPU(unsigned char *buf);
    
protected:
    MemTransferIOS() : MemTransfer(),
                       bufferAttr(NULL),
                       inputPixelBuffer(NULL),
                       outputPixelBuffer(NULL),
                       inputTexture(NULL),
                       outputTexture(NULL),
                       textureCache(NULL),
                       inputPixelBufferSize(0),
                       outputPixelBufferSize(0) { }

private:
    void releaseBuffers();
    
    void *lockBufferAndGetPtr(BufType bufType);
    void unlockBuffer(BufType bufType);
    
    void getPixelBufferAndLockFlags(BufType bufType, CVPixelBufferRef *buf, CVOptionFlags *lockOpt);
    
    
    CFMutableDictionaryRef bufferAttr;
    CVPixelBufferRef inputPixelBuffer;
    CVPixelBufferRef outputPixelBuffer;
    CVOpenGLESTextureRef inputTexture;
    CVOpenGLESTextureRef outputTexture;
    CVOpenGLESTextureCacheRef textureCache;
    
    size_t inputPixelBufferSize;
    size_t outputPixelBufferSize;
};

}
#endif