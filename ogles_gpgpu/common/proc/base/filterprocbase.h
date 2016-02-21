//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * Base class for filter (fragment shader only) processors.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_FILTERPROCBASE
#define OGLES_GPGPU_COMMON_PROC_FILTERPROCBASE

#include "../../common_includes.h"

#include "procbase.h"

namespace ogles_gpgpu {

/**
 * Base class for filter processors. Such processors implement image processing
 * tasks with fragment shaders. They output is rendered on a fullscreen quad.
 */
class FilterProcBase : public ProcBase {
public:
    FilterProcBase() : ProcBase(),
        fragShaderSrcForCompilation(NULL) {
    }

    /**
     * Set output orientation to <o>.
     */
    virtual void setOutputRenderOrientation(RenderOrientation o);

    /**
     * Use texture id <id> as input texture at texture <useTexUnit> with texture target <target>.
     */
    virtual void useTexture(GLuint id, GLuint useTexUnit = 1, GLenum target = GL_TEXTURE_2D);

    /**
     * Render a result, i.e. run the shader on the input texture.
     * Abstract method.
     */
    virtual void render();
    
protected:
    
    /**
     * Perform a standard shader initialization.
     */
    virtual int init(int inW, int inH, unsigned int order, bool prepareForExternalInput);
    
    /**
     * Get the vertex shader source.
     */
    virtual const char *getVertexShaderSource() { return vshaderDefault; }
    
    /**
     * Get the fragment shader source.
     */
    virtual const char *getFragmentShaderSource() { return 0; }
    
    /**
     * Set additional uniforms.
     */
    virtual void setUniforms() {}
    
    /**
     * Get uniform indices.
     */
    virtual void getUniforms();
    
    /**
     * Common initialization method for filters with vertex shader source <vShaderSrc>
     * fragment shader source <fShaderSrc> and render output orientation <o>.
     */
    void filterInit(const char *vShaderSrc, const char *fShaderSrc, RenderOrientation o = RenderOrientationNone);

    /**
     * Common filter shader creation method with vertexshader source <vSaderSrc> 
     * fragment shader source <fShaderSrc> and texture target <target>.
     */
    virtual void filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target);

    /**
     * Initialize texture coordinate buffer according to member variable
     * <renderOrientation> or override member variable by <overrideRenderOrientation>.
     */
    void initTexCoordBuf(RenderOrientation overrideRenderOrientation = RenderOrientationNone);

    virtual void filterRenderPrepare();
    virtual void filterRenderSetCoords();
    virtual void filterRenderDraw();
    virtual void filterRenderCleanup();

    static const char *vshaderGPUImage; // GPUImage vertex shader (shader compatibility)
    static const char *vshaderDefault;  // default vertex shader to render a fullscreen quad

    const char *vertexShaderSrcForCompilation = nullptr;  // used vertex shader source for shader compilation
    const char *fragShaderSrcForCompilation = nullptr;	  // used fragment shader source for shader compilation

    GLint shParamAPos;          // shader attribute vertex positions
    GLint shParamATexCoord;     // shader attribute texture coordinates

    GLfloat vertexBuf[OGLES_GPGPU_QUAD_VERTEX_BUFSIZE]; // vertex data buffer for a quad
    GLfloat texCoordBuf[OGLES_GPGPU_QUAD_TEX_BUFSIZE];  // texture coordinate data buffer for a quad
};

}

#endif