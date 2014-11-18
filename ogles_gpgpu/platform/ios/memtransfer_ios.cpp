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
    releaseBuffers();
    CFRelease(textureCache);
    CFRelease(bufferAttr);
}

void MemTransferIOS::releaseBuffers() {
    CVPixelBufferRelease(inputPixelBuffer);
    inputPixelBuffer = NULL;
    
    CVPixelBufferRelease(outputPixelBuffer);
    outputPixelBuffer = NULL;
    
    CFRelease(inputTexture);
    inputTexture = NULL;
    
    CFRelease(outputTexture);
    outputTexture = NULL;
    
    CVOpenGLESTextureCacheFlush(textureCache, 0);
}

void MemTransferIOS::init() {
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
}

void MemTransferIOS::prepare(int inTexW, int inTexH, int outTexW, int outTexH, GLenum inputPxFormat) {
    if (inputW == inTexW
     && inputH == inTexH
     && outputW == outTexW
     && outputH == outTexH
     && inputPixelFormat == inputPxFormat)
    {
        return; // no change;
    }
    
    if (prepared) { // already prepared -- release buffers!
        releaseBuffers();
    }
    
    // call parent to set attributes
    MemTransfer::prepare(inTexW, inTexH, outTexW, outTexH, inputPxFormat);
    
    CVReturn res;
    
    // define pixel format
    OSType pxFmt;
    if (inputPixelFormat == GL_RGBA) {
        pxFmt = kCVPixelFormatType_32RGBA;
    } else if (inputPxFormat == GL_BGRA) {
        pxFmt = kCVPixelFormatType_32BGRA;
    } else {
        cerr << "ogles_gpgpu::MemTransferIOS - prepare - unsupported input pixel format " << inputPixelFormat << endl;
        prepared = false;
        return;
    }
    
    // create input pixel buffer
    res = CVPixelBufferCreate(kCFAllocatorDefault,
                              inputW, inputH,
                              pxFmt,
                              bufferAttr,
                              &inputPixelBuffer);
    
    if (res != kCVReturnSuccess) {
        cerr << "ogles_gpgpu::MemTransferIOS - prepare - error at CVPixelBufferCreate - " << res << endl;
        prepared = false;
        return;
    }
    
    inputPixelBufferSize = inputW * inputH * 4; // always assume 4 channels of 8 bit data
    
    // create output pixel buffer
    res = CVPixelBufferCreate(kCFAllocatorDefault,
                              outputW, outputH,
                              kCVPixelFormatType_32RGBA,    // default OpenGL format
                              bufferAttr,
                              &outputPixelBuffer);
    
    if (res != kCVReturnSuccess) {
        cerr << "ogles_gpgpu::MemTransferIOS - prepare - error at CVPixelBufferCreate - " << res << endl;
        prepared = false;
        return;
    }

    // create input texture
    res = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                       textureCache,
                                                       inputPixelBuffer,
                                                       NULL, // texture attributes
                                                       GL_TEXTURE_2D,
                                                       GL_RGBA, // opengl format
                                                       inputW,
                                                       inputH,
                                                       inputPixelFormat,
                                                       GL_UNSIGNED_BYTE,
                                                       0,
                                                       &inputTexture);
    
    if (res != kCVReturnSuccess) {
        cerr << "ogles_gpgpu::MemTransferIOS - prepare - error at CVOpenGLESTextureCacheCreateTextureFromImage for input - " << res << endl;
        prepared = false;
        return;
    }
    
    outputPixelBufferSize = outputW * outputH * 4; // always assume 4 channels of 8 bit data

    // create output texture
    res = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                       textureCache,
                                                       outputPixelBuffer,
                                                       NULL, // texture attributes
                                                       GL_TEXTURE_2D,
                                                       GL_RGBA, // opengl format
                                                       outputW,
                                                       outputH,
                                                       GL_RGBA, // opengl format
                                                       GL_UNSIGNED_BYTE,
                                                       0,
                                                       &inputTexture);
    
    if (res != kCVReturnSuccess) {
        cerr << "ogles_gpgpu::MemTransferIOS - prepare - error at CVOpenGLESTextureCacheCreateTextureFromImage for output - " << res << endl;
        prepared = false;
        return;
    }
    
    // get the created texture ids
    inputTexId = CVOpenGLESTextureGetName(inputTexture);
    outputTexId = CVOpenGLESTextureGetName(outputTexture);
    
    // all done
    cout << "ogles_gpgpu::MemTransferIOS - prepare - created input tex id " << inputTexId << " / output tex id " << outputTexId << endl;
}

void MemTransferIOS::toGPU(const unsigned char *buf) {
    assert(prepared && inputTexId > 0 && buf);
    
    // copy data to pixel buffer
    void *pixelBufferAddr = lockBufferAndGetPtr(BUF_TYPE_INPUT);
    memcpy(pixelBufferAddr, buf, inputPixelBufferSize);
    unlockBuffer(BUF_TYPE_INPUT);

    // bind the texture
    glBindTexture(GL_TEXTURE_2D, inputTexId);
}

void MemTransferIOS::fromGPU(unsigned char *buf) {
    assert(prepared && outputTexId > 0 && buf);
    
    // bind the texture
	glBindTexture(GL_TEXTURE_2D, outputTexId);
    
    const void *pixelBufferAddr = lockBufferAndGetPtr(BUF_TYPE_OUTPUT);
    memcpy(buf, pixelBufferAddr, outputPixelBufferSize);
    unlockBuffer(BUF_TYPE_OUTPUT);
}

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