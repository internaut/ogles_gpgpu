#ifndef OGLES_GPGPU_COMMON_GL_FBO
#define OGLES_GPGPU_COMMON_GL_FBO

#include "../common_includes.h"
#include "../core.h"
#include "memtransfer_factory.h"

namespace ogles_gpgpu {

class Core;
    
class FBO {
public:
    FBO();
    virtual ~FBO();
    
    /**
     * Get output texture id.
     */
	int getAttachedTexId() const { return attachedTexId; }
    
    /**
     * Set GL texture unit (to be used in glActiveTexture()) for output texture.
     * Must be set before calling generateIds()
     */
    void setGLTexUnit(GLuint texUnit) { glTexUnit = texUnit; }
    
    /**
     * Get GL texture unit (to be used in glActiveTexture()) for output texture.
     */
    GLuint getGLTexUnit() const { return glTexUnit; }
    
    /**
     * Generate an FBO id.
     * TODO: make this method redundant and remove it
     */
    virtual void generateIds();
    
    /**
     * Set the FBO id (if you did not call generateIds()).
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
    
	int getTexWidth() const { return texW; }
	int getTexHeight() const { return texH; }
    
    MemTransfer *getMemTransfer() const { return memTransfer; }
    
protected:
    Core *core;
    MemTransfer *memTransfer;
    
	GLuint id;
    GLuint glTexUnit;
	GLuint attachedTexId;
    
	int texW;
	int texH;
};
    
}

#endif