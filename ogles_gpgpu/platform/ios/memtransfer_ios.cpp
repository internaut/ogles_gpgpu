#include "memtransfer_ios.h"

#import <CoreVideo/CoreVideo.h>

#include <iostream>
#include "../../common/core.h"

/**
 * Most code as from http://allmybrain.com/2011/12/08/rendering-to-a-texture-with-ios-5-texture-cache-api/
 */

using namespace std;
using namespace ogles_gpgpu;

MemTransferIOS::~MemTransferIOS() {
    releaseInput();
    releaseOutput();
    
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
    cout << "ogles_gpgpu::MemTransferIOS - init with OpenGL ES context " << glCtxPtr << endl;
    assert(glCtxPtr);
    CVReturn res = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault,
                                                NULL,
                                                (CVEAGLContext)glCtxPtr,
                                                NULL,
                                                &textureCache);
    
    if (res != kCVReturnSuccess) {
        cerr << "ogles_gpgpu::MemTransferIOS - toGPU - error at CVOpenGLESTextureCacheCreate - " << res << endl;
    }
    
    // call parent init
    MemTransfer::init();
}

GLuint MemTransferIOS::prepareInput(int inTexW, int inTexH, GLenum inputPxFormat) {
    assert(initialized && inTexW > 0 && inTexH > 0);
    
    if (inputW == inTexW && inputH == inTexH && inputPixelFormat == inputPxFormat) {
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
    CVPixelBufferRef bufRef;
    CVOpenGLESTextureRef texRef;
    CVReturn res;
    
    // define pixel format
    OSType pxFmt;
    if (inputPixelFormat == GL_RGBA) {
        pxFmt = kCVPixelFormatType_32RGBA;
    } else if (inputPxFormat == GL_BGRA) {
        pxFmt = kCVPixelFormatType_32BGRA;
    } else {
        cerr << "ogles_gpgpu::MemTransferIOS - prepareInput - unsupported input pixel format " << inputPixelFormat << endl;
        preparedInput = false;
        return 0;
    }
    
    // create input pixel buffer
    res = CVPixelBufferCreate(kCFAllocatorDefault,
                              inputW, inputH,
                              kCVPixelFormatType_32BGRA,    // !
                              bufferAttr,
                              &bufRef);
    
    if (res != kCVReturnSuccess) {
        cerr << "ogles_gpgpu::MemTransferIOS - prepareInput - error at CVPixelBufferCreate - " << res << endl;
        preparedInput = false;
        return 0;
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
                                                       GL_BGRA, // !
                                                       GL_UNSIGNED_BYTE,
                                                       0,
                                                       &texRef);
    
    if (res != kCVReturnSuccess) {
        cerr << "ogles_gpgpu::MemTransferIOS - prepareInput - error at CVOpenGLESTextureCacheCreateTextureFromImage - " << res << endl;
        preparedInput = false;
        return 0;
    }
    
    // get created texture id
    inputTexId = CVOpenGLESTextureGetName(texRef);
    
    cout << "ogles_gpgpu::MemTransferIOS - prepareInput - created input tex with id " << inputTexId << endl;
    
    // set texture parameters
    setCommonTextureParams(inputTexId);
    
    // set member variables
    inputPixelBuffer = bufRef;
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
        cerr << "ogles_gpgpu::MemTransferIOS - prepareOutput - error at CVPixelBufferCreate - " << res << endl;
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
        cerr << "ogles_gpgpu::MemTransferIOS - prepareOutput - error at CVOpenGLESTextureCacheCreateTextureFromImage - " << res << endl;
        preparedOutput = false;
        return 0;
    }
    
    // get created texture id
    outputTexId = CVOpenGLESTextureGetName(texRef);
    
    cout << "ogles_gpgpu::MemTransferIOS - prepareOutput - created output tex with id " << outputTexId << endl;
    
    // set texture parameters
    setCommonTextureParams(outputTexId);
    
    // set member variables
    outputPixelBuffer = bufRef;
    outputTexture = texRef;
    preparedOutput = true;
    
    return outputTexId;
}

void MemTransferIOS::toGPU(const unsigned char *buf) {
    assert(preparedInput && inputTexId > 0 && buf);
    
    // copy data to pixel buffer
    void *pixelBufferAddr = lockBufferAndGetPtr(BUF_TYPE_INPUT);
    memcpy(pixelBufferAddr, buf, inputPixelBufferSize);
    unlockBuffer(BUF_TYPE_INPUT);

    // bind the texture
    glBindTexture(GL_TEXTURE_2D, inputTexId);
}

void MemTransferIOS::fromGPU(unsigned char *buf) {
    assert(preparedOutput && outputTexId > 0 && buf);
    
    // bind the texture
	glBindTexture(GL_TEXTURE_2D, outputTexId);
    
    const void *pixelBufferAddr = lockBufferAndGetPtr(BUF_TYPE_OUTPUT);
    memcpy(buf, pixelBufferAddr, outputPixelBufferSize);
    unlockBuffer(BUF_TYPE_OUTPUT);
}

#pragma mark private methods

void *MemTransferIOS::lockBufferAndGetPtr(BufType bufType) {
    CVPixelBufferRef buf;
    CVOptionFlags lockOpt;
    getPixelBufferAndLockFlags(bufType, &buf, &lockOpt);
    
    CVReturn res = CVPixelBufferLockBaseAddress(buf, lockOpt);
    
    if (res != kCVReturnSuccess) {
        cerr << "ogles_gpgpu::MemTransferIOS - lockBufferAndGetPtr - error " << res << endl;
        return NULL;
    }
    
    return CVPixelBufferGetBaseAddress(buf);
}

void MemTransferIOS::unlockBuffer(BufType bufType) {
    CVPixelBufferRef buf;
    CVOptionFlags lockOpt;
    getPixelBufferAndLockFlags(bufType, &buf, &lockOpt);
    
    CVReturn res = CVPixelBufferUnlockBaseAddress(buf, lockOpt);
    
    if (res != kCVReturnSuccess) {
        cerr << "ogles_gpgpu::MemTransferIOS - unlockBuffer - error " << res << endl;
    }
}

void MemTransferIOS::getPixelBufferAndLockFlags(BufType bufType, CVPixelBufferRef *buf, CVOptionFlags *lockOpt) {
    if (bufType == BUF_TYPE_INPUT) {
        *buf = inputPixelBuffer;
        *lockOpt = 0;
    } else {
        *buf = outputPixelBuffer;
        *lockOpt = kCVPixelBufferLock_ReadOnly;
    }
}