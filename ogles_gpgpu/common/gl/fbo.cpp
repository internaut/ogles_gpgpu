#include "fbo.h"

using namespace std;
using namespace ogles_gpgpu;

FBO::FBO() {
	id = 0;
	texW = texH = 0;
}

FBO::~FBO() {
    freeFBOBuffers();
}

void FBO::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void FBO::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::freeFBOBuffers() {
    
}

void FBO::createAttachedTex(int w, int h, bool genMipmap, GLenum attachment) {
	assert(attachedTexId > 0 && w > 0 && h > 0);
    
    cout << "ogles_gpgpu::FBO - " << id
         << " - Creating attached texture " << attachedTexId
         << " of size " << w << "x" << h << endl;
    
	texW = w;
	texH = h;
    
	GLenum format = GL_RGBA;
    
	// create texture for FBO
	glBindTexture(GL_TEXTURE_2D, attachedTexId);
	glTexImage2D(GL_TEXTURE_2D, 0,
				 format,
			     w, h, 0,
			     format, GL_UNSIGNED_BYTE,
			     NULL);	// we do not need to pass texture data -> it will be generated!
    
	if (genMipmap) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
    
	// bind it to FBO
	bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER,
						   attachment,
						   GL_TEXTURE_2D,
						   attachedTexId, 0);
	unbind();
    
	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
        cerr << "ogles_gpgpu::FBO - " << id
             << " - Framebuffer incomplete - error " << fboStatus << endl;
	}
}

void FBO::readBuffer(unsigned char *buf) {
	assert(attachedTexId > 0 && texW > 0 && texH > 0);
    
//#if defined (BENCHMARK) || defined (SECURE_GRAPHICS_BUF_READ)
//	glFinish();
//#endif
    
	bind();
    
	// old (and slow) way using glReadPixels:
    glReadPixels(0, 0, texW, texH, GL_RGBA, GL_UNSIGNED_BYTE, buf);
    Tools::checkGLErr("ogles_gpgpu::FBO");
    
	unbind();
    
//#ifdef BENCHMARK
//	glFinish();
//#endif
}