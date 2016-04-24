//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "memtransfer_osx.h"

#import <CoreVideo/CoreVideo.h>

#include "../../common/core.h"

/**
 * Most code as from http://allmybrain.com/2011/12/08/rendering-to-a-texture-with-osx-5-texture-cache-api/
 */

using namespace std;
using namespace ogles_gpgpu;

#pragma mark static methods

bool MemTransferOSX::initPlatformOptimizations() {
    // Texture cache currently not working in os x.
    // Documentation and working examples are sparse.
    // Macbook video frames at 1280x720 load in 1 ms with
    return false;
    //return true;
}

#pragma mark constructor / deconstructor

MemTransferOSX::~MemTransferOSX() {
    // release in- and outputs
    releaseInput();
    releaseOutput();

    // release texture cache and buffer attributes
    CFRelease(textureCache);
    CFRelease(bufferAttr);
}

#pragma mark public methods

void MemTransferOSX::releaseInput() {
    if (inputPixelBuffer) {
        CVPixelBufferRelease(inputPixelBuffer);
        inputPixelBuffer = NULL;
    }

    if (inputTexture) {
        CFRelease(inputTexture);
        inputTexture = NULL;
    }

    CVOpenGLTextureCacheFlush(textureCache, 0);

    preparedInput = false;
}

void MemTransferOSX::releaseOutput() {
    if (outputPixelBuffer) {
        CVPixelBufferRelease(outputPixelBuffer);
        outputPixelBuffer = NULL;
    }

    if (outputTexture) {
        CFRelease(outputTexture);
        outputTexture = NULL;
    }

    CVOpenGLTextureCacheFlush(textureCache, 0);

    preparedOutput = false;
}

void MemTransferOSX::init() {
    assert(!initialized);

    CFDictionaryRef empty;
    empty = CFDictionaryCreate(kCFAllocatorDefault, // our empty OSXurface properties dictionary
                               NULL,
                               NULL,
                               0,
                               &kCFTypeDictionaryKeyCallBacks,
                               &kCFTypeDictionaryValueCallBacks);
    bufferAttr = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                           1,
                                           &kCFTypeDictionaryKeyCallBacks,
                                           &kCFTypeDictionaryValueCallBacks);

    CFDictionarySetValue(bufferAttr, kCVPixelBufferIOSurfacePropertiesKey, empty);

    // create texture cache
    CGLContextObj glCtxPtr = (CGLContextObj) Core::getInstance()->getGLContextPtr();
    OG_LOGINF("MemTransferOSX", "OpenGL ES context at %p", glCtxPtr);

    assert(glCtxPtr);
    CVReturn res = CVOpenGLTextureCacheCreate((CFAllocatorRef) kCFAllocatorDefault,
                   (CFDictionaryRef) NULL,
                   (CGLContextObj) glCtxPtr,
                   CGLGetPixelFormat(glCtxPtr),
                   (CFDictionaryRef) NULL,
                   &textureCache);

    if (res != kCVReturnSuccess) {
        OG_LOGERR("MemTransferOSX", "CVOpenGLTextureCacheCreate error %d", res);
    }

    // call parent init
    MemTransfer::init();
}

GLuint MemTransferOSX::prepareInput(int inTexW, int inTexH, GLenum inputPxFormat, void *inputDataPtr) {
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
    CVOpenGLTextureRef texRef;
    CVReturn res;

    // define pixel format
    OSType pxBufFmt;
    if (inputPixelFormat == GL_BGRA) {
        pxBufFmt = kCVPixelFormatType_32BGRA;
    } else {
        OG_LOGERR("MemTransferOSX", "unsupported input pixel format %d", inputPixelFormat);
        preparedInput = false;
        return 0;
    }

    // create input pixel buffer if necessary
    if (!bufRef) {
        res = CVPixelBufferCreate(kCFAllocatorDefault,
                                  inputW,
                                  inputH,
                                  pxBufFmt,
                                  bufferAttr,
                                  &bufRef);

        if (res != kCVReturnSuccess) {
            OG_LOGERR("MemTransferOSX", "CVPixelBufferCreate error %d (input)", res);
            preparedInput = false;
            return 0;
        }
    }

    inputPixelBufferSize = inputW * inputH * 4; // always assume 4 channels of 8 bit data

    CVOpenGLTextureCacheFlush(textureCache, 0);

    // create input texture
    //error = CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault, textureCache, renderTarget, NULL, &renderTexture);
    res = CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
            textureCache,
            bufRef,
            NULL, // texture attributes
            &texRef);

    if (res != kCVReturnSuccess) {
        OG_LOGERR("MemTransferOSX", "CVOpenGLTextureCacheCreateTextureFromImage error %d (input)", res);
        preparedInput = false;
        return 0;
    }

    // get created texture id
    inputTexId = CVOpenGLTextureGetName(texRef);
    OG_LOGINF("MemTransferOSX", "created input tex with id %d", inputTexId);
    //GL_TEXTURE_RECTANGLE_ARB
    GLenum target = CVOpenGLTextureGetTarget(texRef);
    glEnable(target);

    // set texture parameters
    setCommonTextureParams(inputTexId, target);

    // set member variables
    if (inputDataPtr == NULL) {
        inputPixelBuffer = bufRef;  // only necessary if we did not specify our own pixel buffer via <inputDataPtr>
    }
    inputTexture = texRef;
    preparedInput = true;

    return inputTexId;
}

GLuint MemTransferOSX::prepareOutput(int outTexW, int outTexH) {
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
    CVOpenGLTextureRef texRef;
    CVReturn res;

    // create output pixel buffer
    res = CVPixelBufferCreate(kCFAllocatorDefault,
                              outputW, outputH,
                              kCVPixelFormatType_32BGRA,    // !
                              bufferAttr,
                              &bufRef);

    if (res != kCVReturnSuccess) {
        OG_LOGERR("MemTransferOSX", "CVPixelBufferCreate error %d (output)", res);
        preparedOutput = false;
        return 0;
    }

    outputPixelBufferSize = outputW * outputH * 4; // always assume 4 channels of 8 bit data

    // create output texture
    res = CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
            textureCache,
            bufRef,
            NULL, // texture attributes
            &texRef);

    if (res != kCVReturnSuccess) {
        OG_LOGERR("MemTransferOSX", "CVOpenGLTextureCacheCreateTextureFromImage error %d (output)", res);
        preparedOutput = false;
        return 0;
    }

    // get created texture id
    outputTexId = CVOpenGLTextureGetName(texRef);
    OG_LOGINF("MemTransferOSX", "created output tex with id %d", outputTexId);

    GLenum target = CVOpenGLTextureGetTarget(texRef);
    glEnable(target);

    // set texture parameters
    setCommonTextureParams(outputTexId, target);

    // set member variables
    outputPixelBuffer = bufRef;
    outputTexture = texRef;
    preparedOutput = true;

    return outputTexId;
}

void MemTransferOSX::toGPU(const unsigned char *buf) {
    assert(preparedInput && inputPixelBuffer && inputTexId > 0 && buf);

    // copy data to pixel buffer
    void *pixelBufferAddr = lockBufferAndGetPtr(BUF_TYPE_INPUT);
    memcpy(pixelBufferAddr, buf, inputPixelBufferSize);
    unlockBuffer(BUF_TYPE_INPUT);

    // bind the texture
    glBindTexture(GL_TEXTURE_2D, inputTexId);
}

void MemTransferOSX::fromGPU(unsigned char *buf) {
    assert(preparedOutput && outputPixelBuffer && outputTexId > 0 && buf);

    // bind the texture
    glBindTexture(GL_TEXTURE_2D, outputTexId);

    const void *pixelBufferAddr = lockBufferAndGetPtr(BUF_TYPE_OUTPUT);
    memcpy(buf, pixelBufferAddr, outputPixelBufferSize);
    unlockBuffer(BUF_TYPE_OUTPUT);
}

#pragma mark private methods

void *MemTransferOSX::lockBufferAndGetPtr(BufType bufType) {
    // get the buffer reference and lock options
    CVPixelBufferRef buf;
    CVOptionFlags lockOpt;
    getPixelBufferAndLockFlags(bufType, &buf, &lockOpt);

    // lock
    CVReturn res = CVPixelBufferLockBaseAddress(buf, lockOpt);

    if (res != kCVReturnSuccess) {
        OG_LOGERR("MemTransferOSX", "CVPixelBufferLockBaseAddress error %d", res);
        return NULL;
    }

    // return address
    return CVPixelBufferGetBaseAddress(buf);
}

void MemTransferOSX::unlockBuffer(BufType bufType) {
    // get the buffer reference and lock options
    CVPixelBufferRef buf;
    CVOptionFlags lockOpt;
    getPixelBufferAndLockFlags(bufType, &buf, &lockOpt);

    // unlock
    CVReturn res = CVPixelBufferUnlockBaseAddress(buf, lockOpt);

    if (res != kCVReturnSuccess) {
        OG_LOGERR("MemTransferOSX", "CVPixelBufferUnlockBaseAddress error %d", res);
    }
}

void MemTransferOSX::getPixelBufferAndLockFlags(BufType bufType, CVPixelBufferRef *buf, CVOptionFlags *lockOpt) {
    if (bufType == BUF_TYPE_INPUT) {
        *buf = inputPixelBuffer;
        *lockOpt = 0;                           // read and write
    } else {
        *buf = outputPixelBuffer;
        *lockOpt = kCVPixelBufferLock_ReadOnly; // read only
    }
}
