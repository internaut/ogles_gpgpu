#ifndef OGLES_GPGPU_COMMON_PROC_GRAYSCALE
#define OGLES_GPGPU_COMMON_PROC_GRAYSCALE

#include "../common_includes.h"

#include "procbase.h"

namespace ogles_gpgpu {
class GrayscaleProc : public ProcBase {
public:    
    virtual void init(int inW, int inH, unsigned int order);
    
    virtual void render();
    
private:
    static const char *fshaderGrayscaleSrc;
    
	GLint shParamAPos;
	GLint shParamATexCoord;
    GLint shParamUInputTex;
    
	GLfloat vertexBuf[OGLES_GPGPU_QUAD_VERTEX_BUFSIZE];
	GLfloat texCoordBuf[OGLES_GPGPU_QUAD_TEX_BUFSIZE];
};
}

#endif