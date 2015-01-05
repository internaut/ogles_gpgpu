//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "adapt_thresh_pass.h"

using namespace ogles_gpgpu;

// Adaptive thresholding - Pass 1 fragment shader
// Perform a vertical 5x1 average gray pixel value calculation
// Requires a grayscale image as input!
const char *AdaptThreshProcPass::fshaderAdaptThreshPass1Src = OG_TO_STR(
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
const char *AdaptThreshProcPass::fshaderAdaptThreshPass2Src = OG_TO_STR(
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

int AdaptThreshProcPass::init(int inW, int inH, unsigned int order, bool prepareForExternalInput) {
    OG_LOGINF(getProcName(), "render pass %d", renderPass);
    
    // create fbo for output
    createFBO();
    
    // parent init - set defaults
    baseInit(inW, inH, order, prepareForExternalInput, procParamOutW, procParamOutH, procParamOutScale);
    
    // calculate pixel delta values
    pxDx = 1.0f / (float)outFrameW;
    pxDy = 1.0f / (float)outFrameH;
    
    // get necessary fragment shader source
    const char *shSrc = renderPass == 1 ? fshaderAdaptThreshPass1Src : fshaderAdaptThreshPass2Src;
    
    // FilterProcBase init - create shaders, get shader params, set buffers for OpenGL
    filterInit(shSrc, RenderOrientationDiagonal);
    
    // get additional shader params
    shParamUPxD = shader->getParam(UNIF, "uPxD");
    
    return 1;
}

void AdaptThreshProcPass::createFBOTex(bool genMipmap) {
    assert(fbo);
    
    if (renderPass == 1) {
        fbo->createAttachedTex(outFrameH, outFrameW, genMipmap);   // swapped
    } else {
        fbo->createAttachedTex(outFrameW, outFrameH, genMipmap);
    }
    
    // update frame size, because it might be set to a POT size because of mipmapping
    outFrameW = fbo->getTexWidth();
    outFrameH = fbo->getTexHeight();
}

void AdaptThreshProcPass::render() {
    OG_LOGINF(getProcName(), "input tex %d, target %d, render pass %d, framebuffer of size %dx%d", texId, texTarget, renderPass, outFrameW, outFrameH);
    
    filterRenderPrepare();
	
	glUniform2f(shParamUPxD, pxDx, pxDy);	// texture pixel delta values
    
    Tools::checkGLErr(getProcName(), "render prepare");
    
    filterRenderSetCoords();
    Tools::checkGLErr(getProcName(), "render set coords");
    
    filterRenderDraw();
    Tools::checkGLErr(getProcName(), "render draw");
    
    filterRenderCleanup();
    Tools::checkGLErr(getProcName(), "render cleanup");
}