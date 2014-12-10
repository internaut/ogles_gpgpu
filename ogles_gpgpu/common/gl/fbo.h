//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * Framebuffer object handler.
 */
#ifndef OGLES_GPGPU_COMMON_GL_FBO
#define OGLES_GPGPU_COMMON_GL_FBO

#include "../common_includes.h"
#include "../core.h"
#include "memtransfer_factory.h"

namespace ogles_gpgpu {

class Core;

/**
 * Framebuffer object handler. Set up an OpenGL framebuffer with an attached texture
 * for the framebuffer output.
 */
class FBO {
public:
    /**
     * Constructor.
     */
    FBO();
    
    /**
     * Deconstructor.
     */
    virtual ~FBO();
    
    /**
     * Get output texture id.
     */
	int getAttachedTexId() const { return attachedTexId; }
    
    /**
     * Set GL texture unit (to be used in glActiveTexture()) for output texture.
     */
    void setGLTexUnit(GLuint texUnit) { glTexUnit = texUnit; }
    
    /**
     * Get GL texture unit (to be used in glActiveTexture()) for output texture.
     */
    GLuint getGLTexUnit() const { return glTexUnit; }
    
    /**
     * Set the FBO id manually (usually not necessary).
     */
	void setId(GLuint fboId) { id = fboId; }
    
    /**
     * Return the FBO id.
     */
	GLuint getId() const { return id; }
    
    /**
     * Bind FBO.
     */
    void bind();
    
    /**
     * Unbind FBO.
     */
    void unbind();
    
    /**
     * Will create a framebuffer output texture with texture id <attachedTexId>
     * and will bind it to this FBO.
     */
    virtual void createAttachedTex(int w, int h, bool genMipmap = false, GLenum attachment = GL_COLOR_ATTACHMENT0);
    
    /**
     * Copy the framebuffer data which was written to the framebuffer texture back to
     * main memory at <buf>.
     */
	virtual void readBuffer(unsigned char *buf);
    
    /**
     * Free the framebuffer.
     */
    virtual void destroyFramebuffer();
    
    /**
     * Free the attached texture for framebuffer output.
     */
    virtual void destroyAttachedTex();
    
    /**
     * Return output texture width
     */
	int getTexWidth() const { return texW; }
    
    /**
     * Return output texture height
     */
	int getTexHeight() const { return texH; }
    
    /**
     * Get MemTransfer object associated with this FBO.
     */
    MemTransfer *getMemTransfer() const { return memTransfer; }
    
protected:
    /**
     * Generate an FBO id.
     */
    virtual void generateIds();
    
    
    Core *core;                 // Core singleton
    
    MemTransfer *memTransfer;   // MemTransfer object associated with this FBO
    
	GLuint id;                  // OpenGL FBO id
    GLuint glTexUnit;           // GL texture unit (to be used in glActiveTexture()) for output texture
	GLuint attachedTexId;       // output texture id
    
	int texW;   // output texture width
	int texH;   // output texture height
};
    
}

#endif