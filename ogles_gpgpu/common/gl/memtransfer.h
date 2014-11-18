#ifndef OGLES_GPGPU_COMMON_GL_MEMTRANSFER
#define OGLES_GPGPU_COMMON_GL_MEMTRANSFER

#include "../common_includes.h"

namespace ogles_gpgpu {

class MemTransfer {
public:
    /**
     * Get singleton instance.
     */
    static MemTransfer *getInstance();
    
    /**
     * Destroy singleton instance.
     */
    static void destroy();
    
    /**
     * Initialize method to be called AFTER the OpenGL context was created.
     */
    virtual void init() { }
    
    /**
     * Prepare for input frames of size <inTexW>x<inTexH> and output frame of size <outTexW>x<outTexH> and
     * a certain <inputPxFormat>.
     */
    virtual void prepare(int inTexW, int inTexH, int outTexW, int outTexH, GLenum inputPxFormat = GL_RGBA);
    
    /**
     * Set an input texture id
     */
    virtual void setInputTexId(GLuint texId) { inputTexId = texId; }
    
    /**
     * Set an output texture id
     */
    virtual void setOutputTexId(GLuint texId) { outputTexId = texId; }
    
    virtual GLuint getInputTexId() const { return inputTexId; }
    virtual GLuint getOutputTexId() const { return outputTexId; }
    
    /**
     * Map data in <buf> to GPU.
     */
    virtual void toGPU(const unsigned char *buf);
    
    /**
     * Map data from GPU to <buf>
     */
    virtual void fromGPU(unsigned char *buf);
    
protected:
    /**
     * Private constructor for singleton instance.
     */
    MemTransfer();
    
    /**
     * Empty copy constructor.
     */
    MemTransfer (const MemTransfer&) {}
    
    
    bool prepared;
    
    int inputW;
    int inputH;
    int outputW;
    int outputH;
    
    GLuint inputTexId;
    GLuint outputTexId;
    
    GLenum inputPixelFormat;
    
private:
    static MemTransfer *instance;  // singleton instance
};

}
    
#endif