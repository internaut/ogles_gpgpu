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
    
    FilterProcBase() : ProcBase() {}
    
    virtual void setOutputRenderOrientation(RenderOrientation o);
    
protected:
    void filterInit(const char *fShaderSrc, RenderOrientation o = RenderOrientationNone);
    
    void initTexCoordBuf(RenderOrientation overrideRenderOrientation = RenderOrientationNone);
    
    void filterRenderPrepare();
    void filterRenderSetCoords();
    void filterRenderDraw();
    void filterRenderCleanup();
    
    static const char *vshaderDefault;  // default vertex shader to render a fullscreen quad
    
	GLint shParamAPos;          // shader attribute vertex positions
	GLint shParamATexCoord;     // shader attribute texture coordinates
    
	GLfloat vertexBuf[OGLES_GPGPU_QUAD_VERTEX_BUFSIZE]; // vertex data buffer for a quad
	GLfloat texCoordBuf[OGLES_GPGPU_QUAD_TEX_BUFSIZE];  // texture coordinate data buffer for a quad
};

}

#endif