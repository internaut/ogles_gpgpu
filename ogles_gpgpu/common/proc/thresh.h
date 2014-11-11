#ifndef OGLES_GPGPU_COMMON_PROC_THRESH
#define OGLES_GPGPU_COMMON_PROC_THRESH

#include "../common_includes.h"

#include "procbase.h"

// Simple thresholding fragment shader
// Requires a grayscale image as input!
#define OGLES_GPGPU_COMMON_PROC_SIMPLE_THRESH_FSHADER "\
precision mediump float;\
varying vec2 vTexCoord;\
uniform float uThresh;\
uniform sampler2D uInputTex;\
void main() {\
    float gray = texture2D(uInputTex, vTexCoord).r;\
    float bin = step(uThresh, gray);\
    gl_FragColor = vec4(gray, gray, gray, 1.0);\
}\
"

// Adaptive thresholding - Pass 1 fragment shader
// Perform a vertical 5x1 average gray pixel value calculation
// Requires a grayscale image as input!
#define OGLES_GPGPU_COMMON_PROC_ADAPT_THRESH1_FSHADER "\
precision mediump float;\
varying vec2 vTexCoord;\
uniform vec2 uPxD;\
uniform sampler2D sTexture;\
void main() {\
    // get center pixel value\
    float centerGray = texture2D(sTexture, vTexCoord).r;\
    // get the sum\
    float sum = texture2D(sTexture, vTexCoord + vec2(uPxD.x * -2.0, 0.0)).r +\
    texture2D(sTexture, vTexCoord + vec2(uPxD.x * -1.0, 0.0)).r +\
    centerGray +\
    texture2D(sTexture, vTexCoord + vec2(uPxD.x *  1.0, 0.0)).r +\
    texture2D(sTexture, vTexCoord + vec2(uPxD.x *  2.0, 0.0)).r;\
    // get the average\
    float avg = sum / 5.0;\
    // Result stores average pixel value (R) and original gray value (G)\
    gl_FragColor = vec4(avg, centerGray, 0.0, 1.0);\
}\
"

// Adaptive thresholding - Pass 2
// Perform a horizontal 7x1 or 5x1 average gray pixel value calculation and
// the final binarization
#define OGLES_GPGPU_COMMON_PROC_ADAPT_THRESH2_FSHADER "\
precision mediump float;\
varying vec2 vTexCoord;\
uniform vec2 uPxD;\
uniform sampler2D sTexture;\
void main() {\
    vec4 centerPx = texture2D(sTexture, vTexCoord); // stores: horizontal avg, orig. gray value, 0, 1\
    const float bigC = 9.5 / 255.0;\
    // get the sum\
    float sum = texture2D(sTexture, vTexCoord + vec2(uPxD.y * -2.0, 0.0)).r +\
    texture2D(sTexture, vTexCoord + vec2(uPxD.y * -1.0, 0.0)).r +\
    centerPx.r +\
    texture2D(sTexture, vTexCoord + vec2(uPxD.y *  1.0, 0.0)).r +\
    texture2D(sTexture, vTexCoord + vec2(uPxD.y *  2.0, 0.0)).r;\
    // get the average\
    float avg = sum / 5.0;\
    // create inverted binary value\
    float bin = 1.0 - step(avg - bigC, centerPx.g); // centerPx.g is orig. gray value at current position\
    // store thresholded values\
    gl_FragColor = vec4(bin, bin, bin, 1.0);\
}\
"

namespace ogles_gpgpu {
typedef enum {
    SIMPLE = 0,
    ADAPTIVE_PASS_1,
    ADAPTIVE_PASS_2
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
};
}

#endif