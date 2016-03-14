//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015, http://www.mkonrad.net
//         David Hirvonen <dhirvonen@elucideye.com>
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU grayscale processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_TWOINPUT
#define OGLES_GPGPU_COMMON_PROC_TWOINPUT

#include "../common_includes.h"
#include "base/filterprocbase.h"

namespace ogles_gpgpu {

/**
 * GPGPU TwoInput texture buffer
 */
class TwoInputProc : public FilterProcBase {
public:
    /**
     * Constructor.
     */
    TwoInputProc();

    /**
     * Return the processors name.
     */
    virtual const char *getProcName() { return "TwoInputProc";  }
    
    /**
     * Use texture id <id> as input texture at texture <useTexUnit> with texture target <target>.
     */
    virtual void useTexture(GLuint id, GLuint useTexUnit = 1, GLenum target = GL_TEXTURE_2D, int position = 0);
    
    /**
     * Use texture id <id> as input texture at texture <useTexUnit> with texture target <target>.
     */
    virtual void useTexture2(GLuint id, GLuint useTexUnit = 1, GLenum target = GL_TEXTURE_2D);
    
    virtual int render(int position=0);
    
    void setWaitForSecondTexture(bool flag) { waitForSecondTexture = flag; }
    
protected:
    
    virtual void filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target);
    
    /**
     * Get the fragment shader source.
     */
    virtual const char *getFragmentShaderSource() { return fshaderTwoInputSrc; }
    
    /**
     * Get the vertex shader source.
     */
    virtual const char *getVertexShaderSource() { return vshaderTwoInputSrc; }
    
    /**
     * Set additional uniforms.
     */
    virtual void setUniforms();
    
    /**
     * Get uniform indices.
     */
    virtual void getUniforms();

    /**
     * Bind all input textures
     */
    virtual void filterRenderPrepare();
    
    bool waitForSecondTexture = true;
    bool hasTex1 = false;
    bool hasTex2 = false;

    GLuint texId2;       // input texture id
    GLuint texUnit2;     // input texture unit (glActiveTexture())
    GLenum texTarget2;   // input texture target
    
    GLint shParamUInputTex2;
    
    static const char *fshaderTwoInputSrc; // fragment shader source

    static const char *vshaderTwoInputSrc; // vertex shader source
};
}

#endif
