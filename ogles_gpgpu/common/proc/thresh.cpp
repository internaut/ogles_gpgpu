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

// Adaptive thresholding - Pass 1 fragment shader
// Perform a vertical 5x1 average gray pixel value calculation
// Requires a grayscale image as input!
const char *ThreshProc::fshaderAdaptThreshPass1Src = TO_STR(
precision mediump float;
varying vec2 vTexCoord;
uniform vec2 uPxD;
uniform sampler2D uInputTex;
void main() {
    // get center pixel value
    float centerGray = texture2D(uInputTex, vTexCoord).r;
    // get the sum
    float sum = texture2D(uInputTex, vTexCoord + vec2(uPxD.x * -2.0, 0.0)).r +
        texture2D(uInputTex, vTexCoord + vec2(uPxD.x * -1.0, 0.0)).r +
        centerGray +
        texture2D(uInputTex, vTexCoord + vec2(uPxD.x *  1.0, 0.0)).r +
        texture2D(uInputTex, vTexCoord + vec2(uPxD.x *  2.0, 0.0)).r;
    // get the average
    float avg = sum / 5.0;
    // Result stores average pixel value (R) and original gray value (G)
    gl_FragColor = vec4(avg, centerGray, 0.0, 1.0);
}
);

// Adaptive thresholding - Pass 2
// Perform a horizontal 7x1 or 5x1 average gray pixel value calculation and
// the final binarization
const char *ThreshProc::fshaderAdaptThreshPass2Src = TO_STR(
precision mediump float;
varying vec2 vTexCoord;
uniform vec2 uPxD;
uniform sampler2D uInputTex;
void main() {
    vec4 centerPx = texture2D(uInputTex, vTexCoord); // stores: horizontal avg, orig. gray value, 0, 1
    const float bigC = 9.5 / 255.0;
    // get the sum
    float sum = texture2D(uInputTex, vTexCoord + vec2(uPxD.y * -2.0, 0.0)).r +
    texture2D(uInputTex, vTexCoord + vec2(uPxD.y * -1.0, 0.0)).r +
    centerPx.r +
    texture2D(uInputTex, vTexCoord + vec2(uPxD.y *  1.0, 0.0)).r +
    texture2D(uInputTex, vTexCoord + vec2(uPxD.y *  2.0, 0.0)).r;
    // get the average
    float avg = sum / 5.0;
    // create inverted binary value
    float bin = 1.0 - step(avg - bigC, centerPx.g); // centerPx.g is orig. gray value at current position
    // store thresholded values
    gl_FragColor = vec4(bin, bin, bin, 1.0);
}
);

ThreshProc::ThreshProc() {
    // set defaults
    threshType = THRESH_SIMPLE;
    threshVal = 0.5f;
    pxDx = pxDy = 0.0f;
}

void ThreshProc::init(int inW, int inH, unsigned int order, bool prepareForExternalInput) {
    cout << "ogles_gpgpu::ThreshProc - init" << endl;
    
    // create fbo for output
    createFBO();
    
    // parent init - set defaults
    baseInit(inW, inH, order, prepareForExternalInput, procParamOutW, procParamOutH, procParamOutScale);
    
    if (threshType == THRESH_ADAPTIVE_PASS_1 || threshType == THRESH_ADAPTIVE_PASS_2) {
        // calculate pixel delta values
        pxDx = 1.0f / (float)outFrameW;
        pxDy = 1.0f / (float)outFrameH;
    }

    // get necessary fragment shader source
    const char *shSrc = NULL;
    if (threshType == THRESH_SIMPLE) {
        shSrc = fshaderSimpleThreshSrc;
    } else if (threshType == THRESH_ADAPTIVE_PASS_1) {
        shSrc = fshaderAdaptThreshPass1Src;
    } else if (threshType == THRESH_ADAPTIVE_PASS_2) {
        shSrc = fshaderAdaptThreshPass2Src;
    }
    
    // FilterProcBase init - create shaders, get shader params, set buffers for OpenGL
    filterInit(shSrc, threshType == THRESH_SIMPLE ? RenderOrientationStd : RenderOrientationDiagonal);
    
    // get additional shader params
    if (threshType == THRESH_SIMPLE) {
        shParamUThresh = shader->getParam(UNIF, "uThresh");
    } else {
        shParamUPxD = shader->getParam(UNIF, "uPxD");
    }
}

void ThreshProc::createFBOTex(bool genMipmap) {
    assert(fbo);
    
    if (threshType == THRESH_ADAPTIVE_PASS_1) {
        fbo->createAttachedTex(outFrameH, outFrameW, genMipmap);   // swapped
    } else {
        fbo->createAttachedTex(outFrameW, outFrameH, genMipmap);
    }
    
    // update frame size, because it might be set to a POT size because of mipmapping
    outFrameW = fbo->getTexWidth();
    outFrameH = fbo->getTexHeight();
}

void ThreshProc::render() {
    cout << "ogles_gpgpu::ThreshProc - render (thresh type " << threshType << ") to framebuffer of size " << outFrameW << "x" << outFrameH << endl;
    
    filterRenderPrepare();
	
    if (threshType == THRESH_SIMPLE) {
		glUniform1f(shParamUThresh, threshVal);	// thresholding value for simple thresholding
	} else {
		glUniform2f(shParamUPxD, pxDx, pxDy);	// texture pixel delta values
	}
    
    Tools::checkGLErr("ogles_gpgpu::ThreshProc - render prepare");
    
    filterRenderSetCoords();
    Tools::checkGLErr("ogles_gpgpu::ThreshProc - render set coords");
    
    filterRenderDraw();
    Tools::checkGLErr("ogles_gpgpu::ThreshProc - render draw");
    
    filterRenderCleanup();
    Tools::checkGLErr("ogles_gpgpu::ThreshProc - render cleanup");
}