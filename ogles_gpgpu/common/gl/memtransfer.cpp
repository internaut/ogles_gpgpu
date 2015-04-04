//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "memtransfer.h"

using namespace ogles_gpgpu;

#pragma mark static methods

bool MemTransfer::initPlatformOptimizations() {
    // always return false here. this method is only fully implemented
    // in platform-specialized classes of MemTransfer.
    return false;
}

#pragma mark constructor/deconstructor

MemTransfer::MemTransfer() {
    // set defaults
    inputW = inputH = outputW = outputH = 0;
    inputTexId = 0;
    outputTexId = 0;
    initialized = false;
    preparedInput = false;
    preparedOutput = false;
    inputPixelFormat = GL_RGBA;
}

MemTransfer::~MemTransfer() {
    // release in- and output
    releaseInput();
    releaseOutput();
}

#pragma mark public methods

GLuint MemTransfer::prepareInput(int inTexW, int inTexH, GLenum inputPxFormat, void *inputDataPtr) {
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
    
    // generate texture id
    glGenTextures(1, &inputTexId);
    
    if (inputTexId == 0) {
        OG_LOGERR("MemTransfer", "no valid input texture generated");
        return 0;
    }
    
    // done
    preparedInput = true;
    
    return inputTexId;
}

GLuint MemTransfer::prepareOutput(int outTexW, int outTexH) {
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
    
    // generate texture id
    glGenTextures(1, &outputTexId);
    
    if (outputTexId == 0) {
        OG_LOGERR("MemTransfer", "no valid output texture generated");
        return 0;
    }
    
    // will bind the texture, too:
    setCommonTextureParams(outputTexId);
    
    Tools::checkGLErr("MemTransfer", "fbo texture parameters");
    
    // create empty texture space on GPU
	glTexImage2D(GL_TEXTURE_2D, 0,
				 GL_RGBA,
			     outTexW, outTexH, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     NULL);	// we do not need to pass texture data -> it will be generated!
    
    Tools::checkGLErr("MemTransfer", "fbo texture creation");
    
    // done
    preparedOutput = true;
    
    return outputTexId;
}

void MemTransfer::releaseInput() {
    if (inputTexId > 0) {
        glDeleteTextures(1, &inputTexId);
        inputTexId = 0;
    }
}

void MemTransfer::releaseOutput() {
    if (outputTexId > 0) {
        glDeleteTextures(1, &outputTexId);
        outputTexId = 0;
    }
}

void MemTransfer::toGPU(const unsigned char *buf) {
    assert(preparedInput && inputTexId > 0 && buf);
    
	glBindTexture(GL_TEXTURE_2D, inputTexId);	// bind input texture

    // copy data as texture to GPU
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, inputW, inputH, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
    
    // check for error
    Tools::checkGLErr("MemTransfer", "toGPU (glTexImage2D)");
    
    setCommonTextureParams(0);
}

void MemTransfer::fromGPU(unsigned char *buf) {
    assert(preparedOutput && outputTexId > 0 && buf);
    
	glBindTexture(GL_TEXTURE_2D, outputTexId);
    
	// default (and slow) way using glReadPixels:
    glReadPixels(0, 0, outputW, outputH, GL_RGBA, GL_UNSIGNED_BYTE, buf);

    // check for error
    Tools::checkGLErr("MemTransfer", "fromGPU (glReadPixels)");
}

#pragma mark protected methods

void MemTransfer::setCommonTextureParams(GLuint texId) {
    if (texId > 0) {
        glBindTexture(GL_TEXTURE_2D, texId);
    }
    
	// set clamping (allows NPOT textures)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}