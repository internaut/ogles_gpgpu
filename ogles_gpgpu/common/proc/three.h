/**
 * GPGPU grayscale processor.
 */
#ifndef OGLES_GPGPU_COMMON_THREEINPUT_PROC
#define OGLES_GPGPU_COMMON_THREEINPUT_PROC

#include "base/filterprocbase.h"

namespace ogles_gpgpu {

/**
 * GPGPU ThreeInput texture buffer
 */
class ThreeInputProc : public FilterProcBase {
public:
    /**
     * Constructor.
     */
    ThreeInputProc();

    /**
     * Return the processors name.
     */
    virtual const char *getProcName() { return "ThreeInputProc";  }
    
    /**
     * Use texture id <id> as input texture at texture <useTexUnit> with texture target <target>.
     */
    virtual void useTexture(GLuint id, GLuint useTexUnit = 1, GLenum target = GL_TEXTURE_2D, int position = 0);
    
    /**
     * Use texture id <id> as input texture at texture <useTexUnit> with texture target <target>.
     */
    virtual void useTexture2(GLuint id, GLuint useTexUnit = 1, GLenum target = GL_TEXTURE_2D);
    void setWaitForSecondTexture(bool flag) { waitForSecondTexture = flag; }

    virtual void useTexture3(GLuint id, GLuint useTexUnit, GLenum target = GL_TEXTURE_2D);
    void setWaitForThirdTexture(bool flag) { waitForThirdTexture = flag; }
    
    virtual int render(int position=0);
    
protected:
    
    virtual void filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target);
    
    /**
     * Get the fragment shader source.
     */
    virtual const char *getFragmentShaderSource() { return fshaderThreeInputSrc; }
    
    /**
     * Get the vertex shader source.
     */
    virtual const char *getVertexShaderSource() { return vshaderThreeInputSrc; }
    
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
    
    bool hasTex1 = false;
    bool waitForFirstTexture = true;

    bool hasTex2 = false;
    bool waitForSecondTexture = true;
    GLint shParamUInputTex2;
    GLuint texId2;       // input texture id
    GLuint texUnit2;     // input texture unit (glActiveTexture())
    GLenum texTarget2;   // input texture target

    bool hasTex3 = false;
    bool waitForThirdTexture = true;
    GLint shParamUInputTex3;
    GLuint texId3;       // input texture id
    GLuint texUnit3;     // input texture unit (glActiveTexture())
    GLenum texTarget3;   // input texture target
    
    static const char *fshaderThreeInputSrc; // fragment shader source
    static const char *vshaderThreeInputSrc; // vertex shader source
};
}

#endif
