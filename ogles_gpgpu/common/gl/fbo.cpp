//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "fbo.h"

using namespace std;
using namespace ogles_gpgpu;

FBO::FBO() {
    // set defaults
	id = 0;
	texW = texH = 0;
    attachedTexId = 0;
    glTexUnit = 0;
    
    // get singleton Core instance
    core = Core::getInstance();
    
    // create a dedicated MemTransfer object for this FBO
    memTransfer = MemTransferFactory::createInstance();
    memTransfer->init();
    
    // generate a FBO id
    generateIds();
}

FBO::~FBO() {
    destroyFramebuffer();
    
    // attached texture will be destroyed together with memTransfer instance
    delete memTransfer;
}

void FBO::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void FBO::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::destroyFramebuffer() {
    OG_LOGINF("FBO", "freeing FBO with ID %d", id);
    
	glDeleteFramebuffers(1, &id);
}

void FBO::destroyAttachedTex() {
    assert(memTransfer);
    
    // will release attached texture
    memTransfer->releaseOutput();
}

void FBO::createAttachedTex(int w, int h, bool genMipmap, GLenum attachment) {
	assert(memTransfer && w > 0 && h > 0);
    
    // get a corrected width and height when we use a mipmap
    if (genMipmap && core->getUseMipmaps()) {
        w = Tools::getBiggerPOTValue(w);
        h = Tools::getBiggerPOTValue(h);
    }
    
    texW = w;
	texH = h;
    
    // bind FBO
    bind();
    
    // create attached texture
    glActiveTexture(GL_TEXTURE0 + glTexUnit);
    attachedTexId = memTransfer->prepareOutput(texW, texH);
    
	// set further texture parameters
	if (genMipmap) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
        Tools::checkGLErr("FBO" , "fbo texture mipmap generation");
	} else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    
	// bind it to FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER,
						   attachment,
						   GL_TEXTURE_2D,
						   attachedTexId, 0);
    
	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
        OG_LOGERR("FBO", "Framebuffer incomplete (error %d)", fboStatus);
        attachedTexId = 0;
	} else {
        OG_LOGINF("FBO", "FBO with ID %d: created attached texture %d of size %dx%d (mipmap: %d)",
                  id, attachedTexId, w, h, genMipmap);
    }
    
    // unbind FBO
	unbind();
}

void FBO::readBuffer(unsigned char *buf) {
	assert(memTransfer && attachedTexId > 0 && texW > 0 && texH > 0);
    
    // bind the FBO
	bind();
    
    // get the contents of its attached texture
    memTransfer->fromGPU(buf);
    
    // unbind again
	unbind();
}

void FBO::generateIds() {
    glGenFramebuffers(1, &id);
}