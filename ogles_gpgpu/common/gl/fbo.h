#ifndef OGLES_GPGPU_COMMON_GL_FBO
#define OGLES_GPGPU_COMMON_GL_FBO

#include "../common_includes.h"

namespace ogles_gpgpu {
    
class FBO {
public:
    FBO();
    virtual ~FBO();
    
    virtual void bind();
    virtual void unbind();
    
    /**
     * Set output texture id to <texId>.
     */
	void setAttachedTexId(GLuint texId) { attachedTexId = texId; }
    
    /**
     * Get output texture id.
     */
	int getAttachedTexId() const { return attachedTexId; }
    
    /**
     * Will create a framebuffer texture with texture id <attachedTexId> 
     * and will bind it to this FBO.
     * Abstract method.
     */
    virtual void createAttachedTex(int w, int h, bool genMipmap = false, GLenum attachment = GL_COLOR_ATTACHMENT0);
    
    /**
     * Copy the framebuffer data which was written to the framebuffer texture back to
     * main memory at <buf>.
     * Abstract method.
     */
	virtual void readBuffer(unsigned char *buf);
    
    /**
     * Free allocated FBO buffers.
     */
    virtual void freeFBOBuffers();
    
	void setId(GLuint fboId) { id = fboId; }
	GLuint getId() const { return id; }
    
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