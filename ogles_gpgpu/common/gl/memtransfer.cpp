#include "memtransfer.h"

using namespace ogles_gpgpu;

#pragma mark constructor/deconstructor

MemTransfer *MemTransfer::createInstance() {
    return new MemTransfer();   // TODO: specializations
}

MemTransfer::MemTransfer() {
    inputW = inputH = outputW = outputH = 0;
    inputTexId = 0;
    outputTexId = 0;
    initialized = false;
    preparedInput = false;
    preparedOutput = false;
    inputPixelFormat = GL_RGBA;
}

MemTransfer::~MemTransfer() {
    releaseInput();
    releaseOutput();
}

#pragma mark public methods

GLuint MemTransfer::prepareInput(int inTexW, int inTexH, GLenum inputPxFormat) {
    assert(initialized && inTexW > 0 && inTexH > 0);
    
    inputW = inTexW;
    inputH = inTexH;
    inputPixelFormat = inputPxFormat;
    
    glGenTextures(1, &inputTexId);
    
    if (inputTexId == 0) {
        cerr << "ogles_gpgpu::MemTransfer - prepareInput - no valid input texture generated" << endl;
        return 0;
    }
    
    preparedInput = true;
    
    return inputTexId;
}

GLuint MemTransfer::prepareOutput(int outTexW, int outTexH) {
    assert(initialized && outTexW > 0 && outTexH > 0);
    
    outputW = outTexW;
    outputH = outTexH;
    
    glGenTextures(1, &outputTexId);
    
    if (outputTexId == 0) {
        cerr << "ogles_gpgpu::MemTransfer - prepareInput - no valid output texture generated" << endl;
        return 0;
    }
    
    glBindTexture(GL_TEXTURE_2D, outputTexId);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    Tools::checkGLErr("ogles_gpgpu::MemTransfer - prepareInput - fbo texture parameters");
    
	glTexImage2D(GL_TEXTURE_2D, 0,
				 GL_RGBA,
			     outTexW, outTexH, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     NULL);	// we do not need to pass texture data -> it will be generated!
    
    Tools::checkGLErr("ogles_gpgpu::MemTransfer - prepareInput - fbo texture creation");
    
    preparedOutput = true;
    
    return outputTexId;
}

void MemTransfer::releaseInput() {
    if (inputTexId > 0) glDeleteTextures(1, &inputTexId);
    inputTexId = 0;
}

void MemTransfer::releaseOutput() {
    if (outputTexId > 0) glDeleteTextures(1, &outputTexId);
    outputTexId = 0;
}

void MemTransfer::toGPU(const unsigned char *buf) {
    assert(preparedInput && inputTexId > 0 && buf);
    
	glBindTexture(GL_TEXTURE_2D, inputTexId);	// bind input texture

    // copy data as texture to GPU
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, inputW, inputH, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
    
    // check for error
    Tools::checkGLErr("ogles_gpgpu::MemTransfer - toGPU");
    
	// set clamping (allows NPOT textures)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void MemTransfer::fromGPU(unsigned char *buf) {
    assert(preparedOutput && outputTexId > 0 && buf);
    
	glBindTexture(GL_TEXTURE_2D, outputTexId);
    
	// default (and slow) way using glReadPixels:
    glReadPixels(0, 0, outputW, outputH, GL_RGBA, GL_UNSIGNED_BYTE, buf);

    // check for error
    Tools::checkGLErr("ogles_gpgpu::MemTransfer - fromGPU");
}