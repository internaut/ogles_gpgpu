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

#include "../../common/proc/yuv2rgb.h"


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
    
    if (lumaTexture) {
        CFRelease(lumaTexture);
        lumaTexture = NULL;
    }
    
    if (chromaTexture) {
        CFRelease(chromaTexture);
        chromaTexture = NULL;
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
    
    int bytesPerLine = 0;

    // define pixel format
    OSType pxBufFmt;
    if (inputPixelFormat == GL_BGRA) {
        bytesPerLine = inputW * 4;
        pxBufFmt = kCVPixelFormatType_32BGRA;
    } else if(inputPixelFormat == 0) {
        bytesPerLine = inputW;
        pxBufFmt = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
    } else {
        OG_LOGERR("MemTransferIOS", "unsupported input pixel format %d", inputPixelFormat);
        preparedInput = false;
        return 0;
    }
    
    // prepare
    CVPixelBufferRef bufRef;
    CVOpenGLESTextureRef texRef = 0;
    CVOpenGLESTextureRef luminanceTextureRef = 0;
    CVOpenGLESTextureRef chrominanceTextureRef = 0;
    CVReturn res;
    
    if(inputDataPtr) {
        if(useRawPixels) {
            
            if(pxBufFmt == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange) {

                res = CVPixelBufferCreate(kCFAllocatorDefault,
                                          inputW,
                                          inputH,
                                          pxBufFmt,
                                          bufferAttr,
                                          &bufRef);
                
                CVPixelBufferLockBaseAddress(bufRef, 0);
                
                assert(CVPixelBufferGetPlaneCount(bufRef) == 2);
                
                // get plane addresses
                unsigned char *baseAddressY  = (unsigned char *)CVPixelBufferGetBaseAddressOfPlane(bufRef, 0);
                size_t bufferHeight0 = CVPixelBufferGetHeightOfPlane(bufRef, 0);
                size_t bufferWidth0 = CVPixelBufferGetWidthOfPlane(bufRef, 0);
                size_t bytesPerRow0 = CVPixelBufferGetBytesPerRowOfPlane(bufRef, 0);
                
                unsigned char *baseAddressUV = (unsigned char *)CVPixelBufferGetBaseAddressOfPlane(bufRef, 1);
                size_t bufferHeight1 = CVPixelBufferGetHeightOfPlane(bufRef, 1);
                size_t bufferWidth1 = CVPixelBufferGetWidthOfPlane(bufRef, 1);
                size_t bytesPerRow1 = CVPixelBufferGetBytesPerRowOfPlane(bufRef, 1);
                
                uint8_t *yPtr = reinterpret_cast<uint8_t *>(inputDataPtr);
                uint8_t *uvPtr = yPtr + (bytesPerRow0 * bufferHeight0);
                
                //TODO: copy your data buffers to the newly allocated memory locations
                memcpy(baseAddressY, yPtr, bytesPerRow0 * bufferHeight0);
                memcpy(baseAddressUV, uvPtr, bytesPerRow1 * bufferHeight1);
                
                // unlock pixel buffer address
                CVPixelBufferUnlockBaseAddress(bufRef, 0);
            }
            else {
                res = CVPixelBufferCreateWithBytes(
                                                   kCFAllocatorDefault,
                                                   inputW,
                                                   inputH,
                                                   pxBufFmt,
                                                   inputDataPtr,
                                                   bytesPerLine,
                                                   nullptr, // releaseCallback
                                                   nullptr, // releaseRefCon
                                                   bufferAttr, // pixelBufferAttributes
                                                   &bufRef);
            }
            if (res != kCVReturnSuccess) {
                OG_LOGERR("MemTransferIOS", "CVPixelBufferCreate error %d (input)", res);
                preparedInput = false;
                return 0;
            }
        }
        else {
            bufRef = (CVPixelBufferRef)inputDataPtr;
        }
    }
    else {
        
        // create input pixel buffer if necessary
        res = CVPixelBufferCreate(kCFAllocatorDefault,
                                  inputW,
                                  inputH,
                                  pxBufFmt,
                                  bufferAttr,
                                  &bufRef);
        
        if (res != kCVReturnSuccess) {
            OG_LOGERR("MemTransferIOS", "CVPixelBufferCreate error %d (input)", res);
            preparedInput = false;
            return 0;
        }
    }

    // create input texture
    if(inputDataPtr) {
        if(inputPixelFormat == GL_BGRA) {
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
                Core::printCVPixelBuffer("Texture creation failed", bufRef);
                OG_LOGERR("MemTransferIOS", "CVOpenGLESTextureCacheCreateTextureFromImage error %d (input)", res);
                preparedInput = false;
                return 0;
            }
            
            inputPixelBufferSize = inputW * inputH * 4; // always assume 4 channels of 8 bit data
            
            // get created texture id
            inputTexId = CVOpenGLESTextureGetName(texRef);
            
            OG_LOGINF("MemTransferIOS", "created input tex with id %d", inputTexId);
            
            // set texture parameters
            setCommonTextureParams(inputTexId);
        }
        else {
            
            inputPixelBufferSize = (inputW * inputH) * 3 / 2;
            
            glActiveTexture(GL_TEXTURE4);
            res = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                               textureCache,
                                                               bufRef,
                                                               NULL,
                                                               GL_TEXTURE_2D,
                                                               GL_LUMINANCE,
                                                               inputW,
                                                               inputH,
                                                               GL_LUMINANCE,
                                                               GL_UNSIGNED_BYTE,
                                                               0,
                                                               &luminanceTextureRef);
            
            if (res != kCVReturnSuccess) {
                Core::printCVPixelBuffer("Texture creation failed", bufRef);
                OG_LOGERR("MemTransferIOS", "CVOpenGLESTextureCacheCreateTextureFromImage error %d (input)", res);
                preparedInput = false;
                return 0;
            }
            luminanceTexId = CVOpenGLESTextureGetName(luminanceTextureRef);
            glBindTexture(GL_TEXTURE_2D, luminanceTexId);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glActiveTexture(GL_TEXTURE5);
            res = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                               textureCache,
                                                               bufRef,
                                                               NULL,
                                                               GL_TEXTURE_2D,
                                                               GL_LUMINANCE_ALPHA,
                                                               inputW/2,
                                                               inputH/2,
                                                               GL_LUMINANCE_ALPHA,
                                                               GL_UNSIGNED_BYTE,
                                                               1,
                                                               &chrominanceTextureRef);
            
            if (res != kCVReturnSuccess) {
                Core::printCVPixelBuffer("Texture creation failed", bufRef);
                OG_LOGERR("MemTransferIOS", "CVOpenGLESTextureCacheCreateTextureFromImage error %d (input)", res);
                preparedInput = false;
                return 0;
            }
            
            chrominanceTexId = CVOpenGLESTextureGetName(chrominanceTextureRef);
            glBindTexture(GL_TEXTURE_2D, chrominanceTexId);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        
        lumaTexture = luminanceTextureRef;
        chromaTexture = chrominanceTextureRef;
    }

    // set member variables

    if (inputDataPtr == NULL || useRawPixels) {
        // only necessary if we did not specify our own pixel buffer via <inputDataPtr>
        // ...or if we are using a raw image pointer as an input, in which cas we are
        // internally allocating a CVPixelBuferRef
        inputPixelBuffer = bufRef;
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
                              kCVPixelFormatType_32BGRA,
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
