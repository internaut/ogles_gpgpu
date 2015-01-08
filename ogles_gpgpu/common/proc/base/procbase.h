//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU processor base class.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_BASE
#define OGLES_GPGPU_COMMON_PROC_BASE

#include "../../common_includes.h"
#include "procinterface.h"

#include "../../gl/fbo.h"
#include "../../gl/shader.h"
#include "../../gl/memtransfer.h"

#define OGLES_GPGPU_QUAD_VERTICES 				4
#define OGLES_GPGPU_QUAD_COORDS_PER_VERTEX      3
#define OGLES_GPGPU_QUAD_TEXCOORDS_PER_VERTEX 	2
#define OGLES_GPGPU_QUAD_VERTEX_BUFSIZE          (OGLES_GPGPU_QUAD_VERTICES * OGLES_GPGPU_QUAD_COORDS_PER_VERTEX)
#define OGLES_GPGPU_QUAD_TEX_BUFSIZE 			(OGLES_GPGPU_QUAD_VERTICES * OGLES_GPGPU_QUAD_TEXCOORDS_PER_VERTEX)

namespace ogles_gpgpu {

class FBO;

/**
 * ProcBase implements an abstract GPGPU processor base class with some helper methods.
 */
class ProcBase : public ProcInterface {
public:
    /**
     * Constructor.
     */
    ProcBase();
    
    /**
     * Deconstructor.
     */
    virtual ~ProcBase();
    
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
    virtual void setExternalInputDataFormat(GLenum fmt) { inputDataFmt = fmt; }
    
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
     * Print some information about the processor's setup.
     */
    virtual void printInfo();
    
    /**
     * Return used texture unit.
     */
    virtual GLuint getTextureUnit() const { return texUnit; }
    
    /**
     * Set output size by scaling down or up the input frame size by factor <scaleFactor>.
     */
    virtual void setOutputSize(float scaleFactor)  { procParamOutScale = scaleFactor; }

    /**
     * Set output size by scaling down or up the input frame to size <outW>x<outH>.
     */
    virtual void setOutputSize(int outW, int outH) { procParamOutW = outW; procParamOutH = outH; }
    
    /**
     * Set the render orientation to <o>. This will set the order of the output texture coordinates.
     */
    virtual void setOutputRenderOrientation(RenderOrientation o) { renderOrientation = o; }
    
    /**
     * Get the render orientation.
     */
    virtual RenderOrientation getOutputRenderOrientation() const { return renderOrientation; }
    
    /**
     * Get the output frame width.
     */
    virtual int getOutFrameW() const { return outFrameW; }
    
    /**
     * Get the output frame height.
     */
    virtual int getOutFrameH() const { return outFrameH; }
    
    /**
     * Returns true if output size < input size.
     */
    virtual bool getWillDownscale() const { return willDownscale; }
    
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
    virtual GLuint getInputTexId() const { return texId; }
    
    /**
     * Return the output texture id (= texture that is attached to the FBO).
     */
    virtual GLuint getOutputTexId() const;
    
protected:
    /**
     * Common initializations with input size <inW>x<inH>, pipeline processing <order>, output size <outW>x<outH> and
     * scaling factor <scaleFactor>. If output size is 0x0, the output size will be calculated by input size * scaling
     * factor.
     */
    virtual void baseInit(int inW, int inH, unsigned int order, bool prepareForExternalInput = false, int outW = 0, int outH = 0, float scaleFactor = 1.0f);
    
    /**
     * Common frame size setter for input size <inW>x<inH> and output size <outW>x<outH> and scaling factor
     * <scaleFactor>. If output size is 0x0, the output size will be calculated by input size * scaling factor.
     */
    virtual void setInOutFrameSizes(int inW, int inH, int outW, int outH, float scaleFactor);
    
    /**
     * Create an FBO for this processor. This will contain the result after rendering in its attached texture.
     */
    virtual void createFBO();
    
    /**
     * Create the shader program from vertex and fragment shader source code
     * <vshSrc> and <fshSrc>. The fragment shader source might be modified, depending
     * on texture target <target>.
     */
    virtual void createShader(const char *vShSrc, const char *fShSrc, GLenum target);
    
    
	static const GLfloat quadTexCoordsStd[];                // default quad texture coordinates
    static const GLfloat quadTexCoordsStdMirrored[];        // default quad texture coordinates (mirrored)
	static const GLfloat quadTexCoordsFlipped[];            // flipped quad texture coordinates
	static const GLfloat quadTexCoordsFlippedMirrored[];    // flipped, mirrored quad texture coordinates
	static const GLfloat quadTexCoordsDiagonal[];           // diagonal quad texture coordinates
	static const GLfloat quadVertices[];                    // default quad vertices
    
    FBO *fbo;       // strong ref.!
	Shader *shader;	// strong ref.!
    
    unsigned int orderNum;  // position of this processor in the pipeline
    
	GLuint texId;       // input texture id
    GLuint texUnit;     // input texture unit (glActiveTexture())
    GLenum texTarget;   // input texture target
    
    GLint shParamUInputTex;     // shader uniform input texture sampler
    
    int procParamOutW;          // output frame width parameter
    int procParamOutH;          // output frame height parameter
    float procParamOutScale;    // output frame scaling parameter
    
    RenderOrientation renderOrientation;    // output render orientation
    
    bool willDownscale; // is true if output size < input size.
    
    GLenum inputDataFmt;    // input pixel data format

	int inFrameW;   // input frame width
	int inFrameH;   // input frame height
    
	int outFrameW;  // output frame width
	int outFrameH;  // output frame height
};

}

#endif