#ifndef OGLES_GPGPU_COMMON_GL_MEMTRANSFER
#define OGLES_GPGPU_COMMON_GL_MEMTRANSFER

#include "../common_includes.h"

namespace ogles_gpgpu {

class MemTransfer {
public:
    /**
     * Instance creator.
     */
    static MemTransfer *createInstance();
    
    /**
     * Deconstructor
     */
    ~MemTransfer();
    
    /**
     * Initialize method to be called AFTER the OpenGL context was created.
     */
    virtual void init() { initialized = true; }
    
    /**
     * Prepare for input frames of size <inTexW>x<inTexH>. Return a texture id for the input frames.
     */
    virtual GLuint prepareInput(int inTexW, int inTexH, GLenum inputPxFormat = GL_RGBA);
    
    /**
     * Prepare for output frames of size <outTexW>x<outTexH>. Return a texture id for the output frames.
     */
    virtual GLuint prepareOutput(int outTexW, int outTexH);
    
    /**
     * Delete input texture.
     */
    virtual void releaseInput();
    
    /**
     * Delete output texture.
     */
    virtual void releaseOutput();
    
//    /**
//     * Set an input texture id
//     */
//    virtual void setInputTexId(GLuint texId) { inputTexId = texId; }
//    
//    /**
//     * Set an output texture id
//     */
//    virtual void setOutputTexId(GLuint texId) { outputTexId = texId; }
    
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
     * Constructor
     */
    MemTransfer();
    
    
    bool initialized;
    
    bool preparedInput;
    bool preparedOutput;
    
    int inputW;
    int inputH;
    int outputW;
    int outputH;
    
    GLuint inputTexId;
    GLuint outputTexId;
    
    GLenum inputPixelFormat;
};

}
    
#endif