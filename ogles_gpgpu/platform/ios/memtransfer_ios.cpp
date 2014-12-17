//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "memtransfer_ios.h"

#import <CoreVideo/CoreVideo.h>

#include "../../common/core.h"

/**
 * Most code as from http://allmybrain.com/2011/12/08/rendering-to-a-texture-with-ios-5-texture-cache-api/
 */

using namespace std;
using namespace ogles_gpgpu;

#pragma mark static methods

bool MemTransferIOS::initPlatformOptimizations() {
#if TARGET_IPHONE_SIMULATOR
    OG_LOGERR("MemTransferIOS", "platform optimizations not available in simulator");
    return false;   // not CV texture cache API not available in simulator
#else
    return true;    // CV texture cache API available, but nothing to initialize, just return true
#endif
}

#pragma mark constructor / deconstructor

MemTransferIOS::~MemTransferIOS() {
    // release in- and outputs
    releaseInput();
    releaseOutput();
    
    // release texture cache and buffer attributes
    CFRelease(textureCache);
    CFRelease(bufferAttr);
}

#pragma mark public methods

void MemTransferIOS::releaseInput() {
    if (inputPixelBuffer) {
        CVPixelBufferRelease(inputPixelBuffer);
        inputPixelBuffer = NULL;
    }
    
    if (inputTexture) {
        CFRelease(inputTexture);
        inputTexture = NULL;
    }
    
    CVOpenGLESTextureCacheFlush(textureCache, 0);
    
    preparedInput = false;
}

void MemTransferIOS::releaseOutput() {
    if (outputPixelBuffer) {
        CVPixelBufferRelease(outputPixelBuffer);
        outputPixelBuffer = NULL;
    }
    
    if (outputTexture) {
        CFRelease(outputTexture);
        outputTexture = NULL;
    }
    
    CVOpenGLESTextureCacheFlush(textureCache, 0);
    
    preparedOutput = false;
}

void MemTransferIOS::init() {
    assert(!initialized);
    
    CFDictionaryRef empty;
    empty = CFDictionaryCreate(kCFAllocatorDefault, // our empty IOSurface properties dictionary
                               NULL,
                               NULL,
                               0,
                               &kCFTypeDictionaryKeyCallBacks,
                               &kCFTypeDictionaryValueCallBacks);
    bufferAttr = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                           1,
                                           &kCFTypeDictionaryKeyCallBacks,
                                           &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(bufferAttr,
                         kCVPixelBufferIOSurfacePropertiesKey,
                         empty);
    
    // create texture cache
    void *glCtxPtr = Core::getInstance()->getGLContextPtr();
    OG_LOGINF("MemTransferIOS", "OpenGL ES context at %p", glCtxPtr);
    assert(glCtxPtr);
    CVReturn res = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault,
                                                NULL,
                                                (CVEAGLContext)glCtxPtr,
                                                NULL,
                                                &textureCache);
    
    if (res != kCVReturnSuccess) {
        OG_LOGERR("MemTransferIOS", "CVOpenGLESTextureCacheCreate error %d", res);
    }
    
    // call parent init
    MemTransfer::init();
}

GLuint MemTransferIOS::prepareInput(int inTexW, int inTexH, GLenum inputPxFormat, void *inputDataPtr) {
    assert(initialized && inTexW > 0 && inTexH > 0);
    
    if (inputDataPtr == NULL && inputW == inTexW && inputH == inTexH && inputPixelFormat == inputPxFormat) {
        return inputTexId; // no change
    }
    
    if (preparedInput) {    // already prepared -- release buffers!
        releaseInput();
    }
    
    // set attributes
    inputW = inTexW;
    inputH = inTexH;
    inputPixelFormat = inputPxFormat;
    
    // prepare
    CVPixelBufferRef bufRef = (CVPixelBufferRef)inputDataPtr;
    CVOpenGLESTextureRef texRef;
    CVReturn res;
    
    // define pixel format
    OSType pxBufFmt;
    if (inputPixelFormat == GL_BGRA) {
        pxBufFmt = kCVPixelFormatType_32BGRA;
    } else {
        OG_LOGERR("MemTransferIOS", "unsupported input pixel format %d", inputPixelFormat);
        preparedInput = false;
        return 0;
    }
    
    // create input pixel buffer if necessary
    if (!bufRef) {
        res = CVPixelBufferCreate(kCFAllocatorDefault,
                                  inputW, inputH,
                                  pxBufFmt,
                                  bufferAttr,
                                  &bufRef);
        
        if (res != kCVReturnSuccess) {
            OG_LOGERR("MemTransferIOS", "CVPixelBufferCreate error %d (input)", res);
            preparedInput = false;
            return 0;
        }
    }
    
    inputPixelBufferSize = inputW * inputH * 4; // always assume 4 channels of 8 bit data

    // create input texture
    res = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                       textureCache,
                                                       bufRef,
                                                       NULL, // texture attributes
                                                       GL_TEXTURE_2D,
                                                       GL_RGBA, // opengl format
                                                       inputW,
                                                       inputH,
                                                       inputPixelFormat,
                                                       GL_UNSIGNED_BYTE,
                                                       0,
                                                       &texRef);
    
    if (res != kCVReturnSuccess) {
        OG_LOGERR("MemTransferIOS", "CVOpenGLESTextureCacheCreateTextureFromImage error %d (input)", res);
        preparedInput = false;
        return 0;
    }
    
    // get created texture id
    inputTexId = CVOpenGLESTextureGetName(texRef);
    
    OG_LOGINF("MemTransferIOS", "created input tex with id %d", inputTexId);
    
    // set texture parameters
    setCommonTextureParams(inputTexId);
    
    // set member variables
    if (inputDataPtr == NULL) {
        inputPixelBuffer = bufRef;  // only necessary if we did not specify our own pixel buffer via <inputDataPtr>
    }
    inputTexture = texRef;
    preparedInput = true;
    
    return inputTexId;
}

GLuint MemTransferIOS::prepareOutput(int outTexW, int outTexH) {
    assert(initialized && outTexW > 0 && outTexH > 0);
    
    if (outputW == outTexW && outputH == outTexH) {
        return outputTexId; // no change
    }
    
    if (preparedOutput) {    // already prepared -- release buffers!
        releaseOutput();
    }
    
    // set attributes
    outputW = outTexW;
    outputH = outTexH;
    
    // prepare
    CVPixelBufferRef bufRef;
    CVOpenGLESTextureRef texRef;
    CVReturn res;
    
    // create output pixel buffer
    res = CVPixelBufferCreate(kCFAllocatorDefault,
                              outputW, outputH,
                              kCVPixelFormatType_32BGRA,    // !
                              bufferAttr,
                              &bufRef);
    
    if (res != kCVReturnSuccess) {
        OG_LOGERR("MemTransferIOS", "CVPixelBufferCreate error %d (output)", res);
        preparedOutput = false;
        return 0;
    }
    
    outputPixelBufferSize = outputW * outputH * 4; // always assume 4 channels of 8 bit data
    
    // create output texture
    res = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                       textureCache,
                                                       bufRef,
                                                       NULL, // texture attributes
                                                       GL_TEXTURE_2D,
                                                       GL_RGBA, // opengl format
                                                       outputW,
                                                       outputH,
                                                       GL_RGBA, // opengl format
                                                       GL_UNSIGNED_BYTE,
                                                       0,
                                                       &texRef);
    
    if (res != kCVReturnSuccess) {
        OG_LOGERR("MemTransferIOS", "CVOpenGLESTextureCacheCreateTextureFromImage error %d (output)", res);
        preparedOutput = false;
        return 0;
    }
    
    // get created texture id
    outputTexId = CVOpenGLESTextureGetName(texRef);
    
    OG_LOGINF("MemTransferIOS", "created output tex with id %d", outputTexId);
    
    // set texture parameters
    setCommonTextureParams(outputTexId);
    
    // set member variables
    outputPixelBuffer = bufRef;
    outputTexture = texRef;
    preparedOutput = true;
    
    return outputTexId;
}

void MemTransferIOS::toGPU(const unsigned char *buf) {
    assert(preparedInput && inputPixelBuffer && inputTexId > 0 && buf);
    
    // copy data to pixel buffer
    void *pixelBufferAddr = lockBufferAndGetPtr(BUF_TYPE_INPUT);
    memcpy(pixelBufferAddr, buf, inputPixelBufferSize);
    unlockBuffer(BUF_TYPE_INPUT);

    // bind the texture
    glBindTexture(GL_TEXTURE_2D, inputTexId);
}

void MemTransferIOS::fromGPU(unsigned char *buf) {
    assert(preparedOutput && outputPixelBuffer && outputTexId > 0 && buf);
    
    // bind the texture
	glBindTexture(GL_TEXTURE_2D, outputTexId);
    
    const void *pixelBufferAddr = lockBufferAndGetPtr(BUF_TYPE_OUTPUT);
    memcpy(buf, pixelBufferAddr, outputPixelBufferSize);
    unlockBuffer(BUF_TYPE_OUTPUT);
}

#pragma mark private methods

void *MemTransferIOS::lockBufferAndGetPtr(BufType bufType) {
    // get the buffer reference and lock options
    CVPixelBufferRef buf;
    CVOptionFlags lockOpt;
    getPixelBufferAndLockFlags(bufType, &buf, &lockOpt);
    
    // lock
    CVReturn res = CVPixelBufferLockBaseAddress(buf, lockOpt);
    
    if (res != kCVReturnSuccess) {
        OG_LOGERR("MemTransferIOS", "CVPixelBufferLockBaseAddress error %d", res);
        return NULL;
    }
    
    // return address
    return CVPixelBufferGetBaseAddress(buf);
}

void MemTransferIOS::unlockBuffer(BufType bufType) {
    // get the buffer reference and lock options
    CVPixelBufferRef buf;
    CVOptionFlags lockOpt;
    getPixelBufferAndLockFlags(bufType, &buf, &lockOpt);
    
    // unlock
    CVReturn res = CVPixelBufferUnlockBaseAddress(buf, lockOpt);
    
    if (res != kCVReturnSuccess) {
        OG_LOGERR("MemTransferIOS", "CVPixelBufferUnlockBaseAddress error %d", res);
    }
}

void MemTransferIOS::getPixelBufferAndLockFlags(BufType bufType, CVPixelBufferRef *buf, CVOptionFlags *lockOpt) {
    if (bufType == BUF_TYPE_INPUT) {
        *buf = inputPixelBuffer;
        *lockOpt = 0;                           // read and write
    } else {
        *buf = outputPixelBuffer;
        *lockOpt = kCVPixelBufferLock_ReadOnly; // read only
    }
}