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

void MemTransfer::toGPU(GLuint texId, int texW, int texH, const unsigned char *buf) {
	glBindTexture(GL_TEXTURE_2D, texId);	// bind input texture

    // copy data as texture to GPU
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
    
    // check for error
    Tools::checkGLErr("ogles_gpgpu::MemTransfer - toGPU");
}

void MemTransfer::fromGPU(GLuint texId, int texW, int texH, unsigned char *buf) {
	glBindTexture(GL_TEXTURE_2D, texId);
    
	// old (and slow) way using glReadPixels:
    glReadPixels(0, 0, texW, texH, GL_RGBA, GL_UNSIGNED_BYTE, buf);

    // check for error
    Tools::checkGLErr("ogles_gpgpu::MemTransfer - fromGPU");
}