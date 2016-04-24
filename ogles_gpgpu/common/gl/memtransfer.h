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
#include <functional>

namespace ogles_gpgpu {

/**
 * MemTransfer handles memory transfer and mapping between CPU and GPU memory space.
 * Input (from CPU to GPU space) and output (from GPU to CPU space) can be set up
 * separately.
 */
class MemTransfer {
public:
    
    typedef std::function<void(const Size2d &size, const void *pixels, size_t rowStride)> FrameDelegate;
    
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
    virtual void init() {
        initialized = true;
    }

    /**
     * Prepare for input frames of size <inTexW>x<inTexH>. Return a texture id for the input frames.
     */
    virtual GLuint prepareInput(int inTexW, int inTexH, GLenum inputPxFormat = GL_RGBA, void *inputDataPtr = NULL);

    /**
     * Prepare for output frames of size <outTexW>x<outTexH>. Return a texture id for the output frames.
     */
    virtual GLuint prepareOutput(int outTexW, int outTexH);
    
    /**
     * Set output pixel format.
     */
    virtual void setOutputPixelFormat(GLenum outputPxFormat);

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
    virtual GLuint getInputTexId() const {
        return inputTexId;
    }
    
    /**
     * Get output texture id.
     */
    virtual GLuint getOutputTexId() const {
        return outputTexId;
    }

    /**
     * Get output luminance texture id.
     */
    virtual GLuint getLuminanceTexId() const {
        return luminanceTexId;
    }
    
    /**
     * Get output chrominance texture id.
     */
    virtual GLuint getChrominanceTexId() const {
        return chrominanceTexId;
    }
    
    /**
     * Map data in <buf> to GPU.
     */
    virtual void toGPU(const unsigned char *buf);

    /**
     * Map data from GPU to <buf>
     */
    virtual void fromGPU(unsigned char *buf);

    /**
     * Callback for data in GPU.
     */
    virtual void fromGPU(FrameDelegate &delegate);
    
    /**
     * Get output pixel format (i.e., GL_BGRA or GL_RGBA)
     */
    virtual GLenum getOutputPixelFormat() const { return outputPixelFormat; }
    
    /**
     * Inidcates whether or not this MemTransfer implementation
     * support zero copy texture access (i.e., MemTransferIOS)
     */
    virtual bool hasDirectTextureAccess() const { return false; }
    
    /**
     * Row stride (in bytes) of the underlying FBO.
     */
     
    virtual size_t bytesPerRow();
    
    /**
     * Specify input image format, raw pixels or platform specific image type
     */
    virtual void setUseRawPixels(bool flag) { useRawPixels = flag; }
    
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
    virtual void setCommonTextureParams(GLuint texId, GLenum target=GL_TEXTURE_2D);
    
    bool initialized;       // is initialized?

    bool preparedInput;     // input is prepared?
    bool preparedOutput;    // output is prepared?

    int inputW;             // input texture width
    int inputH;             // input texture height
    int outputW;            // output texture width
    int outputH;            // output texture heights

    GLuint inputTexId;      // input texture id
    GLuint outputTexId;     // output texture id
    
    GLuint luminanceTexId = 0;
    GLuint chrominanceTexId = 0;

    GLenum inputPixelFormat;    // input texture pixel format
    GLenum outputPixelFormat;
    
    bool useRawPixels = false;
};

}

#endif