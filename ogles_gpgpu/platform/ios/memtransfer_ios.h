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
    MemTransferIOS() :  MemTransfer(),
                        bufferAttr(NULL),
                        inputPixelBuffer(NULL),
                        outputPixelBuffer(NULL),
                        inputTexture(NULL),
                        outputTexture(NULL),
                        textureCache(NULL),
                        inputPixelBufferSize(0),
                        outputPixelBufferSize(0) { }
    
    virtual ~MemTransferIOS();
    
    virtual void init();
    
    /**
     * Prepare for input frames of size <inTexW>x<inTexH>. Return a texture id for the input frames.
     */
    virtual GLuint prepareInput(int inTexW, int inTexH, GLenum inputPxFormat = GL_RGBA);
    
    /**
     * Prepare for output frames of size <outTexW>x<outTexH>. Return a texture id for the output frames.
     */
    virtual GLuint prepareOutput(int outTexW, int outTexH);
    
    /**
     * Delete input texture.
     */
    virtual void releaseInput();
    
    /**
     * Delete output texture.
     */
    virtual void releaseOutput();
    
    /**
     * Map data in <buf> to GPU.
     */
    virtual void toGPU(const unsigned char *buf);
    
    /**
     * Map data from GPU to <buf>
     */
    virtual void fromGPU(unsigned char *buf);

private:
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