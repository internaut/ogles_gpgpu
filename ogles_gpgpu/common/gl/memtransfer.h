//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * MemTransfer handles memory transfer and mapping between CPU and GPU memory space.
 */

#ifndef OGLES_GPGPU_COMMON_GL_MEMTRANSFER
#define OGLES_GPGPU_COMMON_GL_MEMTRANSFER

#include "../common_includes.h"

namespace ogles_gpgpu {

/**
 * MemTransfer handles memory transfer and mapping between CPU and GPU memory space.
 * Input (from CPU to GPU space) and output (from GPU to CPU space) can be set up
 * separately.
 */
class MemTransfer {
public:
    /**
     * Constructor
     */
    MemTransfer();
    
    /**
     * Deconstructor
     */
    virtual ~MemTransfer();
    
    /**
     * Initialize method to be called AFTER the OpenGL context was created.
     */
    virtual void init() { initialized = true; }
    
    /**
     * Prepare for input frames of size <inTexW>x<inTexH>. Return a texture id for the input frames.
     */
    virtual GLuint prepareInput(int inTexW, int inTexH, GLenum inputPxFormat = GL_RGBA, void *inputDataPtr = NULL);
    
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
    
    /**
     * Get input texture id.
     */
    virtual GLuint getInputTexId() const { return inputTexId; }
    
    /**
     * Get output texture id.
     */
    virtual GLuint getOutputTexId() const { return outputTexId; }
    
    /**
     * Map data in <buf> to GPU.
     */
    virtual void toGPU(const unsigned char *buf);
    
    /**
     * Map data from GPU to <buf>
     */
    virtual void fromGPU(unsigned char *buf);
    
    /**
     * Try to initialize platform optimizations. Returns true on success, else false.
     * Is only fully implemented in platform-specialized classes of MemTransfer.
     */
    static bool initPlatformOptimizations();
    
protected:
    /**
     * bind texture if <texId> > 0 and
     * set clamping (allows NPOT textures)
     */
    virtual void setCommonTextureParams(GLuint texId);
    
    
    bool initialized;       // is initialized?
    
    bool preparedInput;     // input is prepared?
    bool preparedOutput;    // output is prepared?
    
    int inputW;             // input texture width
    int inputH;             // input texture height
    int outputW;            // output texture width
    int outputH;            // output texture heights
    
    GLuint inputTexId;      // input texture id
    GLuint outputTexId;     // output texture id
    
    GLenum inputPixelFormat;    // input texture pixel format
};

}
    
#endif