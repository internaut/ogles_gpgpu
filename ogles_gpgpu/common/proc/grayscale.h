#ifndef OGLES_GPGPU_COMMON_PROC_GRAYSCALE
#define OGLES_GPGPU_COMMON_PROC_GRAYSCALE

#include "../common_includes.h"

#include "procbase.h"

namespace ogles_gpgpu {
typedef enum {
    GRAYSCALE_INPUT_CONVERSION_NONE     = -2,
    GRAYSCALE_INPUT_CONVERSION_CUSTOM   = -1,
    GRAYSCALE_INPUT_CONVERSION_RGB      = 0,
    GRAYSCALE_INPUT_CONVERSION_BGR,
} GrayscaleInputConversionType;
    
class GrayscaleProc : public ProcBase {
public:
    GrayscaleProc();
    
    virtual void init(int inW, int inH, unsigned int order);
    
    virtual void render();
    
    void setGrayscaleConvVec(const GLfloat v[3]);
    const GLfloat *getGrayscaleConvVec() const { return grayscaleConvVec; }
    
    void setGrayscaleConvType(GrayscaleInputConversionType type);
    GrayscaleInputConversionType getGrayscaleConvType() const { return inputConvType; }
    
private:
    static const char *fshaderGrayscaleSrc;
    static const GLfloat grayscaleConvVecRGB[3];
    static const GLfloat grayscaleConvVecBGR[3];
    
	GLint shParamAPos;
	GLint shParamATexCoord;
    GLint shParamUInputTex;
    GLint shParamUInputConvVec;
    
    GLfloat grayscaleConvVec[3];
    GrayscaleInputConversionType inputConvType;
    
	GLfloat vertexBuf[OGLES_GPGPU_QUAD_VERTEX_BUFSIZE];
	GLfloat texCoordBuf[OGLES_GPGPU_QUAD_TEX_BUFSIZE];
};
}

#endif