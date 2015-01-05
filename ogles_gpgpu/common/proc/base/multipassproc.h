//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * Abstract class that defines a multipass processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_MULTIPASSPROC
#define OGLES_GPGPU_COMMON_PROC_MULTIPASSPROC

#include "../../common_includes.h"
#include "procinterface.h"

#include <list>

using namespace std;

namespace ogles_gpgpu {

class MultiPassProc : public ProcInterface {
public:
    /**
     * Deconstructor.
     * Removes all pass instances.
     */
    virtual ~MultiPassProc();
    
    /**
     * Init the processor for input frames of size <inW>x<inH> which is at
     * position <order> in the processing pipeline.
     * Abstract method.
     */
    virtual int init(int inW, int inH, unsigned int order, bool prepareForExternalInput = false);
    
    /**
     * Reinitialize the proc for a different input frame size of <inW>x<inH>.
     */
    virtual int reinit(int inW, int inH, bool prepareForExternalInput = false);
    
    /**
     * Cleanup processor's resources.
     */
    virtual void cleanup();
    
    /**
     * Set pixel data format for input data to <fmt>. Must be set before init() / reinit().
     */
    virtual void setExternalInputDataFormat(GLenum fmt);
    
    /**
     * Insert external data into this processor. It will be used as input texture.
     * Note: init() must have been called with prepareForExternalInput = true for that.
     */
    virtual void setExternalInputData(const unsigned char *data);
    
    /**
     * Create a texture that is attached to the FBO and will contain the processing result.
     * Set <genMipmap> to true to generate a mipmap (usually only works with POT textures).
     */
    virtual void createFBOTex(bool genMipmap);
    
    /**
     * Render a result, i.e. run the shader on the input texture.
     * Abstract method.
     */
    virtual void render();
    
    /**
     * Print some information about the processor's setup.
     */
    virtual void printInfo();
    
    /**
     * Use texture id <id> as input texture at texture <useTexUnit> with texture target <target>.
     */
    virtual void useTexture(GLuint id, GLuint useTexUnit = 1, GLenum target = GL_TEXTURE_2D);
    
    /**
     * Return used texture unit.
     */
    virtual GLuint getTextureUnit() const;
    
    /**
     * Set output size by scaling down or up the input frame size by factor <scaleFactor>.
     */
    virtual void setOutputSize(float scaleFactor);
    
    /**
     * Set output size by scaling down or up the input frame to size <outW>x<outH>.
     */
    virtual void setOutputSize(int outW, int outH);
    
    /**
     * Set the render orientation to <o>.
     * Not implemented for multipass processors!
     */
    virtual void setOutputRenderOrientation(RenderOrientation o) { assert(false); }
    
    /**
     * Get the render orientation.
     * Not implemented for multipass processors!
     */
    virtual RenderOrientation getOutputRenderOrientation() const { return RenderOrientationNone; }
    
    /**
     * Get the output frame width.
     */
    virtual int getOutFrameW() const;
    
    /**
     * Get the output frame height.
     */
    virtual int getOutFrameH() const;
    
    /**
     * Returns true if output size < input size.
     */
    virtual bool getWillDownscale() const;
    
    /**
     * Return the result data from the FBO.
     */
    virtual void getResultData(unsigned char *data) const;
    
    /**
     * Return pointer to MemTransfer object of this processor.
     */
    virtual MemTransfer *getMemTransferObj() const;
    
    /**
     * Return input texture id.
     */
    virtual GLuint getInputTexId() const;
    
    /**
     * Return the output texture id (= texture that is attached to the FBO).
     */
    virtual GLuint getOutputTexId() const;

    /**
     * Get number of passes for this multipass processor.
     */
    size_t getNumPasses() const { return procPasses.size(); }
    
    /**
     * Return te list of processor instances of each pass of this multipass processor.
     */
    list<ProcInterface *> getProcPasses() const { return procPasses; }
    
protected:
    /**
     * This method should be called whenever <procPasses> changed.
     */
    void multiPassInit();
    
    
    list<ProcInterface *> procPasses;   // holds all instances to the single processing passes. strong ref!
    
    ProcInterface *firstProc;
    ProcInterface *lastProc;
};
    
}

#endif