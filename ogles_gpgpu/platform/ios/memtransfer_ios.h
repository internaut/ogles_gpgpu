//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * MemTransferIOS is a platform specific implementation for fast texture access on iOS platforms.
 */

#ifndef OGLES_GPGPU_IOS_GL_MEMTRANSFER_IOS
#define OGLES_GPGPU_IOS_GL_MEMTRANSFER_IOS

#include "../../common/gl/memtransfer.h"
#include "../../common/gl/memtransfer_optimized.h"

#import <CoreFoundation/CoreFoundation.h>
#import <CoreVideo/CoreVideo.h>

namespace ogles_gpgpu {
    
/**
 * MemTransferIOS is a platform specific implementation for fast texture access on iOS platforms.
 * It uses CoreVideo's TextureCache API as explained at
 * http://allmybrain.com/2011/12/08/rendering-to-a-texture-with-ios-5-texture-cache-api/ .
 */
class MemTransferIOS : public MemTransfer, public MemTransferOptimized {
public:
    /**
     * Try to initialize platform optimizations. Returns true on success, else false.
     */
    static bool initPlatformOptimizations();

    /**
     * Constructor. Set defaults.
     */
    MemTransferIOS() :  MemTransfer(),
                        bufferAttr(NULL),
                        inputPixelBuffer(NULL),
                        outputPixelBuffer(NULL),
                        inputTexture(NULL),
                        outputTexture(NULL),
                        textureCache(NULL),
                        inputPixelBufferSize(0),
                        outputPixelBufferSize(0) { }
    
    /**
     * Deconstructor. Release in- and outputs.
     */
    virtual ~MemTransferIOS();
    
    /**
     * Initialize method to be called AFTER the OpenGL context was created. Creates the TextureCache.
     */
    virtual void init();
    
    /**
     * Prepare for input frames of size <inTexW>x<inTexH>. Return a texture id for the input frames.
     */
    virtual GLuint prepareInput(int inTexW, int inTexH, GLenum inputPxFormat = GL_RGBA, void *inputDataPtr = NULL);
    
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

    /**
     * Lock the input or output buffer and return its base address.
     * The input buffer will be locked for reading AND writing, while the
     * output buffer will be locked for reading only.
     */
    virtual void *lockBufferAndGetPtr(BufType bufType);
    
    /**
     * Unlock the input or output buffer.
     */
    virtual void unlockBuffer(BufType bufType);
    
private:
    /**
     * Sets <buf> and <lockOpt> to the necessary pointers/values according to <bufType>.
     */
    void getPixelBufferAndLockFlags(BufType bufType, CVPixelBufferRef *buf, CVOptionFlags *lockOpt);
    
    
    CFMutableDictionaryRef bufferAttr;  // buffer attributes
    
    CVPixelBufferRef inputPixelBuffer;  // input pixel buffer
    CVPixelBufferRef outputPixelBuffer; // output pixel buffer
    
    CVOpenGLESTextureRef inputTexture;  // input texture reference
    CVOpenGLESTextureRef outputTexture; // output texture reference
    
    CVOpenGLESTextureCacheRef textureCache; // common texture cache
    
    size_t inputPixelBufferSize;    // input pixel buffer size in bytes
    size_t outputPixelBufferSize;   // output pixel buffer size in bytes
};

}
#endif
