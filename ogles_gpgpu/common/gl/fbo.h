#ifndef OGLES_GPGPU_COMMON_GL_FBO
#define OGLES_GPGPU_COMMON_GL_FBO

#include "../common_includes.h"

namespace ogles_gpgpu {
    
class FBO {
public:
    FBO();
    virtual ~FBO();
    
    /**
     * Generate an FBO id and a texture id for the attached texture.
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
    virtual void bind();
    
    /**
     * Unbind FBO.
     */
    virtual void unbind();
    
    /**
     * Set output texture id to <texId> (if you did not call generateIds()).
     */
	void setAttachedTexId(GLuint texId) { attachedTexId = texId; }
    
    /**
     * Get output texture id.
     */
	int getAttachedTexId() const { return attachedTexId; }
    
    /**
     * Will create a framebuffer texture with texture id <attachedTexId> 
     * and will bind it to this FBO.
     */
    virtual void createAttachedTex(int w, int h, bool genMipmap = false, GLenum attachment = GL_COLOR_ATTACHMENT0);
    
    /**
     * Copy the framebuffer data which was written to the framebuffer texture back to
     * main memory at <buf>.
     */
	virtual void readBuffer(unsigned char *buf);
    
    /**
     * Free allocated FBO buffers.
     */
    virtual void freeFBOBuffers();
    
	int getTexWidth() const { return texW; }
	int getTexHeight() const { return texH; }
    
protected:
	GLuint id;
	GLuint attachedTexId;
    
	int texW;
	int texH;
};
    
}

#endif