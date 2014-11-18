#include "fbo.h"

using namespace std;
using namespace ogles_gpgpu;

FBO::FBO() {
	id = 0;
	texW = texH = 0;
    attachedTexId = 0;
    glTexUnit = 0;
    
    core = Core::getInstance();
    memTransfer = MemTransfer::getInstance();
}

FBO::~FBO() {
    destroyAttachedTex();
    destroyFramebuffer();
}

void FBO::generateIds() {
    glGenFramebuffers(1, &id);
    glActiveTexture(GL_TEXTURE0 + glTexUnit);
	glGenTextures(1, &attachedTexId);
    
    cout << "ogles_gpgpu::FBO - " << id
         << " generated ids - attached tex is " << attachedTexId
         << " on tex unit " << glTexUnit << endl;
}

void FBO::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void FBO::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::destroyFramebuffer() {
    cout << "ogles_gpgpu::FBO - " << id << " freeing framebuffer" << endl;
    
	glDeleteFramebuffers(1, &id);
}

void FBO::destroyAttachedTex() {
    cout << "ogles_gpgpu::FBO - " << id << " freeing attached texture" << endl;
    
	glDeleteTextures(1, &attachedTexId);
}

void FBO::createAttachedTex(int w, int h, bool genMipmap, GLenum attachment) {
	assert(attachedTexId > 0 && w > 0 && h > 0);
    
    if (genMipmap && core->getUseMipmaps()) {
        w = Tools::getBiggerPOTValue(w);
        h = Tools::getBiggerPOTValue(h);
    }
    
    cout << "ogles_gpgpu::FBO - " << id
         << " - Creating attached texture " << attachedTexId
         << " of size " << w << "x" << h << " (gen. mipmap: " << genMipmap << ")" << endl;
    
	texW = w;
	texH = h;
    
	GLenum format = GL_RGBA;
    
    // bind FBO
    bind();
    
	// create texture for FBO
    glActiveTexture(GL_TEXTURE0 + glTexUnit);
	glBindTexture(GL_TEXTURE_2D, attachedTexId);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    Tools::checkGLErr("ogles_gpgpu::FBO - fbo texture parameters");
    
	glTexImage2D(GL_TEXTURE_2D, 0,
				 format,
			     w, h, 0,
			     format, GL_UNSIGNED_BYTE,
			     NULL);	// we do not need to pass texture data -> it will be generated!
    
    Tools::checkGLErr("ogles_gpgpu::FBO - fbo texture creation");
    
	if (genMipmap) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
        Tools::checkGLErr("ogles_gpgpu::FBO - fbo texture mipmap generation");
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
        cerr << "ogles_gpgpu::FBO - " << id
             << " - Framebuffer incomplete - error " << fboStatus << endl;
	}
    
    // unbind FBO
	unbind();
}

void FBO::readBuffer(unsigned char *buf) {
	assert(attachedTexId > 0 && texW > 0 && texH > 0);
    
	bind();
    
    memTransfer->fromGPU(buf);
    
	unbind();
}