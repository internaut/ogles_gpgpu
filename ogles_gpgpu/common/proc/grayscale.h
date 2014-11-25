/**
 * GPGPU grayscale processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_GRAYSCALE
#define OGLES_GPGPU_COMMON_PROC_GRAYSCALE

#include "../common_includes.h"

#include "procbase.h"

namespace ogles_gpgpu {
    
/**
 * Define grayscale conversion types
 */
typedef enum {
    GRAYSCALE_INPUT_CONVERSION_NONE     = -2,
    GRAYSCALE_INPUT_CONVERSION_CUSTOM   = -1,
    GRAYSCALE_INPUT_CONVERSION_RGB      = 0,
    GRAYSCALE_INPUT_CONVERSION_BGR,
} GrayscaleInputConversionType;

/**
 * GPGPU grayscale processor will convert a RGB or BGR input image to grayscale
 * output image using a weighted channel conversion vector.
 */
class GrayscaleProc : public ProcBase {
public:
    /**
     * Constructor.
     */
    GrayscaleProc();
    
    /**
     * Init the processor for input frames of size <inW>x<inH> which is at
     * position <order> in the processing pipeline.
     */
    virtual void init(int inW, int inH, unsigned int order, bool prepareForExternalInput = false);
    
    /**
     * Render the output.
     */
    virtual void render();
    
    /**
     * Set weighted channel grayscale conversion vector directly to <v>.
     */
    void setGrayscaleConvVec(const GLfloat v[3]);
    
    /**
     * Get weighted channel grayscale conversion vector.
     */
    const GLfloat *getGrayscaleConvVec() const { return grayscaleConvVec; }
    
    /**
     * Set weighted channel grayscale conversion vector by choosing a <type>.
     */
    void setGrayscaleConvType(GrayscaleInputConversionType type);
    
    /**
     * Get grayscale conversion type.
     */
    GrayscaleInputConversionType getGrayscaleConvType() const { return inputConvType; }
    
private:
    static const char *fshaderGrayscaleSrc;         // fragment shader source
    static const GLfloat grayscaleConvVecRGB[3];    // weighted channel grayscale conversion for RGB input (default)
    static const GLfloat grayscaleConvVecBGR[3];    // weighted channel grayscale conversion for BGR input
    
	GLint shParamAPos;          // shader attribute vertex positions
	GLint shParamATexCoord;     // shader attribute texture coordinates
    GLint shParamUInputTex;     // shader uniform input texture sampler
    GLint shParamUInputConvVec; // shader uniform weighted channel grayscale conversion vector
    
    GLfloat grayscaleConvVec[3];                // currently set weighted channel grayscale conversion vector
    GrayscaleInputConversionType inputConvType; // grayscale conversion type
    
	GLfloat vertexBuf[OGLES_GPGPU_QUAD_VERTEX_BUFSIZE]; // vertex data buffer for a quad
	GLfloat texCoordBuf[OGLES_GPGPU_QUAD_TEX_BUFSIZE];  // texture coordinate data buffer for a quad
};
}

#endif