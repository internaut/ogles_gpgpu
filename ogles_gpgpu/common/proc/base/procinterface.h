//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU processor interface
 */
#ifndef OGLES_GPGPU_COMMON_PROC_PROCINTERFACE
#define OGLES_GPGPU_COMMON_PROC_PROCINTERFACE

#include "../../common_includes.h"

#include "../../gl/memtransfer.h"

BEGIN_OGLES_GPGPU

/**
 * GPGPU processor interface
 */
class ProcInterface {
public:
    
    typedef std::function<void(const std::string &tag)> Logger;

    using FrameDelegate = MemTransfer::FrameDelegate;
    
    /**
     * Important: deconstructor must be virtual
     */
    virtual ~ProcInterface() { }

    /**
     * Init the processor for input frames of size <inW>x<inH> which is at
     * position <order> in the processing pipeline.
     * Abstract method.
     */
    virtual int init(int inW, int inH, unsigned int order, bool prepareForExternalInput = false) = 0;

    /**
     * Reinitialize the proc for a different input frame size of <inW>x<inH>.
     */
    virtual int reinit(int inW, int inH, bool prepareForExternalInput = false) = 0;

    /**
     * Cleanup processor's resources.
     */
    virtual void cleanup() = 0;

    /**
     * Set pixel data format for input data to <fmt>. Must be set before init() / reinit().
     */
    virtual void setExternalInputDataFormat(GLenum fmt) = 0;

    /**
     * Insert external data into this processor. It will be used as input texture.
     * Note: init() must have been called with prepareForExternalInput = true for that.
     */
    virtual void setExternalInputData(const unsigned char *data) = 0;

    /**
     * Create a texture that is attached to the FBO and will contain the processing result.
     * Set <genMipmap> to true to generate a mipmap (usually only works with POT textures).
     */
    virtual void createFBOTex(bool genMipmap) = 0;

    /**
     * Render a result, i.e. run the shader on the input texture.
     * Abstract method.  Return 0 on success.
     */
    virtual int render(int position=0) = 0;

    /**
     * Return the processors name.
     */
    virtual const char *getProcName() = 0;

    /**
     * Set a custom tag name.
     */
    virtual void setProcTitle(const std::string &name) { title = name; }
    
    /**
     * Return the custom tag name.
     */
    virtual const char *getProcTitle() { return title.c_str(); }
    
    /**
     * Print some information about the processor's setup.
     */
    virtual void printInfo() = 0;

    /**
     * Use texture id <id> as input texture at texture <useTexUnit> with texture target <target>.
     */
    virtual void useTexture(GLuint id, GLuint useTexUnit = 1, GLenum target = GL_TEXTURE_2D, int position=0) = 0;

    /**
     * Return used texture unit.
     */
    virtual GLuint getTextureUnit() const = 0;

    /**
     * Return the texture target (i.e., GL_TEXTURE_2D, ...)
     */
    virtual GLenum getTextureTarget() const { return GL_TEXTURE_2D; }
    
    /**
     * Set output size by scaling down or up the input frame size by factor <scaleFactor>.
     */
    virtual void setOutputSize(float scaleFactor) = 0;

    /**
     * Set output size by scaling down or up the input frame to size <outW>x<outH>.
     */
    virtual void setOutputSize(int outW, int outH) = 0;

    /**
     * Set the render orientation to <o>. This will set the order of the output texture coordinates.
     */
    virtual void setOutputRenderOrientation(RenderOrientation o) = 0;

    /**
     * Get the render orientation.
     */
    virtual RenderOrientation getOutputRenderOrientation() const = 0;

    /**
     * Get the output frame width.
     */
    virtual int getOutFrameW() const = 0;

    /**
     * Get the output frame height.
     */
    virtual int getOutFrameH() const = 0;
    
    /**
     * Get the input frame width.
     */
    virtual int getInFrameW() const = 0;
    
    /**
     * Get the input frame height.
     */
    virtual int getInFrameH() const = 0;

    /**
     * Returns true if output size < input size.
     */
    virtual bool getWillDownscale() const = 0;

    /**
     * Return the result data from the FBO.
     */
    virtual void getResultData(unsigned char *data) const = 0;

    /**
     * Return the result data from the FBO (zero copy).
     */
    virtual void getResultData(FrameDelegate &) const = 0;
    
    /**
     * Return pointer to MemTransfer object of this processor.
     */
    virtual MemTransfer *getMemTransferObj() const = 0;

    /**
     * Return pointer to designated input MemTransfer object of this processor.
     *
     * Note: This is typically the same, except for multi-pass processors.
     */
    virtual MemTransfer *getInputMemTransferObj() const { return getMemTransferObj(); }

    /**
     * Return input texture id.
     */
    virtual GLuint getInputTexId() const = 0;

    /**
     * Return the output texture id (= texture that is attached to the FBO).
     */
    virtual GLuint getOutputTexId() const = 0;
    
    // ############ Begin filter chain methods ####################
    
    /**
     * Add a subscriber
     */
    virtual void add(ProcInterface *filter, int position=0);

    /**
     * Prepare the filter chain
     */
    virtual void prepare(int inW, int inH, GLenum inFmt, int index = 0, int position = 0);

    /**
     *  Prepare the filter chain filter[i] : i >= 1
     */
    virtual void prepare(int inW, int inH, int index = 0, int position = 0);
    
    /**
     * Process a filter chain:
     */
    virtual void process(GLuint id, GLuint useTexUnit, GLenum target, int index = 0, int position = 0, Logger logger=0);
    
    /**
     * Process filter chain filter[i] : i >= 1
     */
    virtual void process(int position, Logger logger=0);
    
    /**
     * Allow this proc to use mipmaps
     */
    virtual void setUseMipmaps(bool flag) { useMipmaps = flag; }
    
    /**
     * Turn this filter on/off:
     */
    virtual void setActive(bool active);
    
protected:
    
    bool useMipmaps = false; // TODO:
    
    std::string title;
    
    bool active = true;
    
    std::vector<std::pair<ProcInterface *, int>> subscribers;
};

END_OGLES_GPGPU

#endif
