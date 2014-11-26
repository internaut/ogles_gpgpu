#include "thresh.h"

using namespace std;
using namespace ogles_gpgpu;

// Simple thresholding fragment shader
// Requires a grayscale image as input!
const char *ThreshProc::fshaderSimpleThreshSrc = TO_STR(
precision mediump float;
varying vec2 vTexCoord;
uniform float uThresh;
uniform sampler2D uInputTex;
void main() {
    float gray = texture2D(uInputTex, vTexCoord).r;
    float bin = step(uThresh, gray);
    gl_FragColor = vec4(bin, bin, bin, 1.0);
}
);

ThreshProc::ThreshProc() {
    // set defaults
    threshVal = 0.5f;
}

int ThreshProc::init(int inW, int inH, unsigned int order, bool prepareForExternalInput) {
    cout << "ogles_gpgpu::ThreshProc - init" << endl;
    
    // create fbo for output
    createFBO();
    
    // parent init - set defaults
    baseInit(inW, inH, order, prepareForExternalInput, procParamOutW, procParamOutH, procParamOutScale);
    
    // FilterProcBase init - create shaders, get shader params, set buffers for OpenGL
    filterInit(fshaderSimpleThreshSrc);
    
    // get additional shader params
    shParamUThresh = shader->getParam(UNIF, "uThresh");

    return 1;
}

void ThreshProc::render() {
    cout << "ogles_gpgpu::ThreshProc - render to framebuffer of size " << outFrameW << "x" << outFrameH << endl;
    
    filterRenderPrepare();
	
	glUniform1f(shParamUThresh, threshVal);	// thresholding value for simple thresholding
    
    Tools::checkGLErr("ogles_gpgpu::ThreshProc - render prepare");
    
    filterRenderSetCoords();
    Tools::checkGLErr("ogles_gpgpu::ThreshProc - render set coords");
    
    filterRenderDraw();
    Tools::checkGLErr("ogles_gpgpu::ThreshProc - render draw");
    
    filterRenderCleanup();
    Tools::checkGLErr("ogles_gpgpu::ThreshProc - render cleanup");
}