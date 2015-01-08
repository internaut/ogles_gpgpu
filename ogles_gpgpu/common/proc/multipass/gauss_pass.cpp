//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "gauss_pass.h"

using namespace ogles_gpgpu;

const char *GaussProcPass::fshaderGaussSrc = OG_TO_STR(
precision mediump float;

uniform sampler2D uInputTex;
uniform float uPxD;
varying vec2 vTexCoord;
// 7x1 Gauss kernel
void main() {
    vec4 pxC  = texture2D(uInputTex, vTexCoord);
    vec4 pxL1 = texture2D(uInputTex, vTexCoord - vec2(uPxD, 0.0));
    vec4 pxL2 = texture2D(uInputTex, vTexCoord - vec2(2.0 * uPxD, 0.0));
    vec4 pxL3 = texture2D(uInputTex, vTexCoord - vec2(3.0 * uPxD, 0.0));
    vec4 pxR1 = texture2D(uInputTex, vTexCoord + vec2(uPxD, 0.0));
    vec4 pxR2 = texture2D(uInputTex, vTexCoord + vec2(2.0 * uPxD, 0.0));
    vec4 pxR3 = texture2D(uInputTex, vTexCoord + vec2(3.0 * uPxD, 0.0));
    gl_FragColor = 0.006 * (pxL3 + pxR3)
                 + 0.061 * (pxL2 + pxR2)
                 + 0.242 * (pxL1 + pxR1)
                 + 0.382 * pxC;
}
);

int GaussProcPass::init(int inW, int inH, unsigned int order, bool prepareForExternalInput) {
    OG_LOGINF(getProcName(), "render pass %d", renderPass);
    
    // create fbo for output
    createFBO();
    
    // parent init - set defaults
    baseInit(inW, inH, order, prepareForExternalInput, procParamOutW, procParamOutH, procParamOutScale);
    
    // calculate pixel delta values
    pxDx = 1.0f / (float)outFrameW;
    pxDy = 1.0f / (float)outFrameH;
    
    // FilterProcBase init - create shaders, get shader params, set buffers for OpenGL
    filterInit(fshaderGaussSrc, RenderOrientationDiagonal);
    
    // get additional shader params
    shParamUPxD = shader->getParam(UNIF, "uPxD");
    
    return 1;
}

void GaussProcPass::createFBOTex(bool genMipmap) {
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

void GaussProcPass::render() {
    OG_LOGINF(getProcName(), "input tex %d, target %d, render pass %d, framebuffer of size %dx%d", texId, texTarget, renderPass, outFrameW, outFrameH);
    
    filterRenderPrepare();
	
	glUniform1f(shParamUPxD, renderPass == 1 ? pxDy : pxDx);	// texture pixel delta values
    
    Tools::checkGLErr(getProcName(), "render prepare");
    
    filterRenderSetCoords();
    Tools::checkGLErr(getProcName(), "render set coords");
    
    filterRenderDraw();
    Tools::checkGLErr(getProcName(),  "render draw");
    
    filterRenderCleanup();
    Tools::checkGLErr(getProcName(), "render cleanup");
}