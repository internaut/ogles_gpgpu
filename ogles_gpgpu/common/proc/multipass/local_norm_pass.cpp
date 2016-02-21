 //
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015, http://www.mkonrad.net
//         David Hirvonen
//
// See LICENSE file in project repository root for the license.
//

#include "../../common_includes.h"
#include "local_norm_pass.h"

using namespace ogles_gpgpu;

const char *LocalNormPass::fshaderLocalNormPass1Src = OG_TO_STR
(
#if defined(OGLES_GPGPU_OPENGLES)
precision highp float;
#endif

uniform sampler2D uInputTex;
uniform float uPxD;
varying vec2 vTexCoord;
// 7x1 Gauss kernel
void main()
{
    vec4 pxC  = texture2D(uInputTex, vTexCoord);
    float pxL1 = texture2D(uInputTex, vTexCoord - vec2(uPxD, 0.0)).r;
    float pxL2 = texture2D(uInputTex, vTexCoord - vec2(2.0 * uPxD, 0.0)).r;
    float pxL3 = texture2D(uInputTex, vTexCoord - vec2(3.0 * uPxD, 0.0)).r;
    float pxR1 = texture2D(uInputTex, vTexCoord + vec2(uPxD, 0.0)).r;
    float pxR2 = texture2D(uInputTex, vTexCoord + vec2(2.0 * uPxD, 0.0)).r;
    float pxR3 = texture2D(uInputTex, vTexCoord + vec2(3.0 * uPxD, 0.0)).r;
    float val = 0.006 * (pxL3 + pxR3) + 0.061 * (pxL2 + pxR2) + 0.242 * (pxL1 + pxR1) + 0.382 * pxC.r;

    gl_FragColor = vec4(pxC.rgb, val); // {r,g,b,r_mean}
});

const char *LocalNormPass::fshaderLocalNormPass2Src = OG_TO_STR
(
#if defined(OGLES_GPGPU_OPENGLES)
precision highp float;
#endif

uniform sampler2D uInputTex;
uniform float uPxD;
uniform float normConst;
varying vec2 vTexCoord;
// 7x1 Gauss kernel
void main()
{
    vec4 pxC  = texture2D(uInputTex, vTexCoord);
    float pxL1 = texture2D(uInputTex, vTexCoord - vec2(uPxD, 0.0)).a;
    float pxL2 = texture2D(uInputTex, vTexCoord - vec2(2.0 * uPxD, 0.0)).a;
    float pxL3 = texture2D(uInputTex, vTexCoord - vec2(3.0 * uPxD, 0.0)).a;
    float pxR1 = texture2D(uInputTex, vTexCoord + vec2(uPxD, 0.0)).a;
    float pxR2 = texture2D(uInputTex, vTexCoord + vec2(2.0 * uPxD, 0.0)).a;
    float pxR3 = texture2D(uInputTex, vTexCoord + vec2(3.0 * uPxD, 0.0)).a;
    float val = 0.006 * (pxL3 + pxR3) + 0.061 * (pxL2 + pxR2) + 0.242 * (pxL1 + pxR1) + 0.382 * pxC.a;
    
    float rNorm = pxC.r/(val + normConst);
    
    gl_FragColor = vec4(clamp(0.5 * rNorm, 0.0, 1.0), pxC.g, pxC.r, val); // {r_norm,g,r,r_mean}
});

int LocalNormPass::init(int inW, int inH, unsigned int order, bool prepareForExternalInput) {
    OG_LOGINF(getProcName(), "render pass %d", renderPass);

    // create fbo for output
    createFBO();

    // parent init - set defaults
    baseInit(inW, inH, order, prepareForExternalInput, procParamOutW, procParamOutH, procParamOutScale);

    // calculate pixel delta values
    pxDx = 1.0f / (float)outFrameW;
    pxDy = 1.0f / (float)outFrameH;
    
    // get necessary fragment shader source
    const char *shSrc = renderPass == 1 ? fshaderLocalNormPass1Src : fshaderLocalNormPass2Src;

    // FilterProcBase init - create shaders, get shader params, set buffers for OpenGL
    filterInit(vshaderDefault, shSrc, RenderOrientationDiagonal);

    if(renderPass == 2) {
        shParamUNormConst = shader->getParam(UNIF, "normConst");
    }
    
    // get additional shader params
    shParamUPxD = shader->getParam(UNIF, "uPxD");

    return 1;
}

void LocalNormPass::createFBOTex(bool genMipmap) {
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

void LocalNormPass::render() {
    OG_LOGINF(getProcName(), "input tex %d, target %d, render pass %d, framebuffer of size %dx%d", texId, texTarget, renderPass, outFrameW, outFrameH);

    filterRenderPrepare();

    glUniform1f(shParamUPxD, renderPass == 1 ? pxDy : pxDx);	// texture pixel delta values
    if(renderPass == 2) {
        glUniform1f (shParamUNormConst, normConst);
    }

    Tools::checkGLErr(getProcName(), "render prepare");

    filterRenderSetCoords();
    Tools::checkGLErr(getProcName(), "render set coords");

    filterRenderDraw();
    Tools::checkGLErr(getProcName(),  "render draw");

    filterRenderCleanup();
    Tools::checkGLErr(getProcName(), "render cleanup");
}
