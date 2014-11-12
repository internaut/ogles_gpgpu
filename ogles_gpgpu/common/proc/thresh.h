#ifndef OGLES_GPGPU_COMMON_PROC_THRESH
#define OGLES_GPGPU_COMMON_PROC_THRESH

#include "../common_includes.h"

#include "procbase.h"

namespace ogles_gpgpu {
typedef enum {
    THRESH_SIMPLE = 0,
    THRESH_ADAPTIVE_PASS_1,
    THRESH_ADAPTIVE_PASS_2
} ThreshProcType;
    
class ThreshProc : public ProcBase {
public:
    ThreshProc();
    
    void setThreshType(ThreshProcType t) { threshType = t; }
    ThreshProcType getThreshType() const { return threshType; }
    
    void setThreshVal8Bit(int v) { threshVal = (float)v  / 255.0f; }
    void setThreshVal(float v) { threshVal = v; }
    float getThreshVal() const { return threshVal; }
    
    virtual void init(int inW, int inH, unsigned int order);
    
    virtual void createFBOTex(bool genMipmap);
    
    virtual void render();
    
private:
    ThreshProcType threshType;
    float threshVal;    // only used for simple thresholding [0.0 .. 1.0]
    
    GLint shParamAPos;
    GLint shParamATexCoord;
    GLint shParamUInputTex;
	GLint shParamUPxD;		// only used for adapt. thresholding
	GLint shParamUThresh;	// only used for simple thresholding
    
    GLfloat vertexBuf[OGLES_GPGPU_QUAD_VERTEX_BUFSIZE];
    GLfloat texCoordBuf[OGLES_GPGPU_QUAD_TEX_BUFSIZE];
    
	float pxDx;	// pixel delta value for texture access. only used for adapt. thresholding
	float pxDy;	// pixel delta value for texture access. only used for adapt. thresholding
    
    static const char *fshaderSimpleThreshSrc;
    static const char *fshaderAdaptThreshPass1Src;
    static const char *fshaderAdaptThreshPass2Src;
};
}

#endif