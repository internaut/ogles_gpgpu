#include "memtransfer.h"

using namespace ogles_gpgpu;

#pragma mark singleton stuff

MemTransfer *MemTransfer::instance = NULL;

MemTransfer *MemTransfer::getInstance() {
    if (!MemTransfer::instance) {
        // TODO: create platform specific instance
        MemTransfer::instance = new MemTransfer();
    }
    
    return MemTransfer::instance;
}

void MemTransfer::destroy() {
    if (MemTransfer::instance) {
        delete MemTransfer::instance;
        MemTransfer::instance = NULL;
    }
}

#pragma mark public methods

MemTransfer::MemTransfer() {
    inputW = inputH = outputW = outputH = 0;
    inputTexId = 0;
    outputTexId = 0;
    prepared = false;
}

void MemTransfer::prepare(int inTexW, int inTexH, int outTexW, int outTexH, GLenum inputPxFormat) {
    assert(inTexW > 0 && inTexH > 0 && outTexW > 0 && outTexH > 0);
    
    inputW = inTexW;
    inputH = inTexH;
    outputW = outTexW;
    outputH = outTexH;
    inputPixelFormat = inputPxFormat;
    prepared = true;
    
    cout << "ogles_gpgpu::MemTransfer - prepare with input size "
         << inputW << "x" << inputH << " and " << outputW << "x" << outputH
         << " (px format " << inputPixelFormat << ")" << endl;
}

void MemTransfer::toGPU(const unsigned char *buf) {
    assert(prepared && inputTexId > 0 && buf);
    
	glBindTexture(GL_TEXTURE_2D, inputTexId);	// bind input texture

    // copy data as texture to GPU
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, inputW, inputH, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
    
    // check for error
    Tools::checkGLErr("ogles_gpgpu::MemTransfer - toGPU");
}

void MemTransfer::fromGPU(unsigned char *buf) {
    assert(prepared && outputTexId > 0 && buf);
    
	glBindTexture(GL_TEXTURE_2D, outputTexId);
    
	// default (and slow) way using glReadPixels:
    glReadPixels(0, 0, outputW, outputH, GL_RGBA, GL_UNSIGNED_BYTE, buf);

    // check for error
    Tools::checkGLErr("ogles_gpgpu::MemTransfer - fromGPU");
}