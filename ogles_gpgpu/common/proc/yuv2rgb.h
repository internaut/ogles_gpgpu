//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
//         David Hirvonen <dhirvonen@elucideye.com>
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU yuv2rgb processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_YUV2RGB
#define OGLES_GPGPU_COMMON_PROC_YUV2RGB

#include "../common_includes.h"

#include "base/filterprocbase.h"

namespace ogles_gpgpu {

/**
 * GPGPU yuv2rgb processor will apply 2d parametric yuv2rgbations.
 */
class Yuv2RgbProc : public FilterProcBase {
public:
    
    /**
     * Constructor.
     */
    Yuv2RgbProc();

    /**
     * Return the processors name.
     */
    virtual const char *getProcName() { return "Yuv2RgbProc"; }

    /**
     * Init the processor for input frames of size <inW>x<inH> which is at
     * position <order> in the processing pipeline.
     */
    virtual int init(int inW, int inH, unsigned int order, bool prepareForExternalInput = false);

    /**
     * Render the output.
     */
    virtual int render(int position=0);

      /**
     * Create the shader program
     */
    virtual void filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target);

    void setTextures(GLuint luminanceTexture, GLuint chrominanceTexture);
    
private:
    
    virtual void filterRenderPrepare();
   
    static const char *vshaderYuv2RgbSrc;   // fragment shader source
    static const char *fshaderYuv2RgbSrc;   // fragment shader source
    
    //GLProgram *yuvConversionProgram;

    GLuint luminanceTexture;
    GLuint chrominanceTexture;
    
    GLint yuvConversionPositionAttribute;
    GLint yuvConversionTextureCoordinateAttribute;
    
    GLint yuvConversionLuminanceTextureUniform;
    GLint yuvConversionChrominanceTextureUniform;
    GLint yuvConversionMatrixUniform;

    const GLfloat *_preferredConversion;
    
    bool isFullYUVRange = true;
};
}

#endif
