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
    				   fragShaderSrcForCompilation(NULL)
    				   {}
    
    /**
     * Set output orientation to <o>.
     */
    virtual void setOutputRenderOrientation(RenderOrientation o);
    
    /**
     * Use texture id <id> as input texture at texture <useTexUnit> with texture target <target>.
     */
    virtual void useTexture(GLuint id, GLuint useTexUnit = 1, GLenum target = GL_TEXTURE_2D);
    
protected:
    /**
     * Common initialization method for filters with fragment shader source <fShaderSrc>
     * and render output orientation <o>.
     */
    void filterInit(const char *fShaderSrc, RenderOrientation o = RenderOrientationNone);
    
    /**
     * Common filter shader creation method with fragment shader source <fShaderSrc> and
     * texture target <target>.
     */
    void filterShaderSetup(const char *fShaderSrc, GLenum target);
    
    /**
     * Initialize texture coordinate buffer according to member variable
     * <renderOrientation> or override member variable by <overrideRenderOrientation>.
     */
    void initTexCoordBuf(RenderOrientation overrideRenderOrientation = RenderOrientationNone);
    
    void filterRenderPrepare();
    void filterRenderSetCoords();
    void filterRenderDraw();
    void filterRenderCleanup();
    
    
    static const char *vshaderDefault;  // default vertex shader to render a fullscreen quad
    
    const char *fragShaderSrcForCompilation;	// used fragment shader source for shader compilation
    
	GLint shParamAPos;          // shader attribute vertex positions
	GLint shParamATexCoord;     // shader attribute texture coordinates
    
	GLfloat vertexBuf[OGLES_GPGPU_QUAD_VERTEX_BUFSIZE]; // vertex data buffer for a quad
	GLfloat texCoordBuf[OGLES_GPGPU_QUAD_TEX_BUFSIZE];  // texture coordinate data buffer for a quad
};

}

#endif