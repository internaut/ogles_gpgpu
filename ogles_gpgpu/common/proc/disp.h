/**
 * Helper class to simply display an output.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_DISP
#define OGLES_GPGPU_COMMON_PROC_DISP

#include "../common_includes.h"

#include "procbase.h"

namespace ogles_gpgpu {
    
/**
 * Helper class to simply display an output.
 * Render an input texture to a fullscreen quad.
 */
class Disp : public ProcBase {
public:
    /**
     * Init the processor for input frames of size <inW>x<inH> which is at
     * position <order> in the processing pipeline.
     */
    virtual void init(int inW, int inH, unsigned int order);
    
    /**
     * Render the output.
     */
    virtual void render();

private:
    static const char *fshaderDispSrc;         // fragment shader source

	GLint shParamAPos;          // shader attribute vertex positions
	GLint shParamATexCoord;     // shader attribute texture coordinates
    GLint shParamUInputTex;     // shader uniform input texture sampler
    
	GLfloat vertexBuf[OGLES_GPGPU_QUAD_VERTEX_BUFSIZE]; // vertex data buffer for a quad
	GLfloat texCoordBuf[OGLES_GPGPU_QUAD_TEX_BUFSIZE];  // texture coordinate data buffer for a quad
};
    
}

#endif