//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "../../common_includes.h"
#include "gauss_pass.h"

using namespace ogles_gpgpu;

// #### 7 tap filters ####

const char *GaussProcPass::vshaderGauss7Src = OG_TO_STR
(
 attribute vec4 position;
 attribute vec4 inputTextureCoordinate;
 
 uniform float texelWidth;
 uniform float texelHeight;
 
 varying vec2 textureCoordinateN3;
 varying vec2 textureCoordinateN2;
 varying vec2 textureCoordinateN1;
 varying vec2 textureCoordinate;
 varying vec2 textureCoordinateP1;
 varying vec2 textureCoordinateP2;
 varying vec2 textureCoordinateP3;
 
 void main()
 {
     gl_Position = position;
     
     vec2 texelStep = vec2(texelWidth, texelHeight);

     textureCoordinate = inputTextureCoordinate.xy;
     
     textureCoordinateN3 = textureCoordinate - texelStep * 3.0;
     textureCoordinateN2 = textureCoordinate - texelStep * 2.0;
     textureCoordinateN1 = textureCoordinate - texelStep;
     
     textureCoordinateP1 = textureCoordinate + texelStep;
     textureCoordinateP2 = textureCoordinate + texelStep * 2.0;
     textureCoordinateP3 = textureCoordinate + texelStep * 3.0;
 });

const char *GaussProcPass::fshaderGauss7Src = OG_TO_STR
(
#if defined(OGLES_GPGPU_OPENGLES)
precision mediump float;
#endif

 uniform sampler2D inputImageTexture;
 
 varying vec2 textureCoordinateN3;
 varying vec2 textureCoordinateN2;
 varying vec2 textureCoordinateN1;
 varying vec2 textureCoordinate;
 varying vec2 textureCoordinateP1;
 varying vec2 textureCoordinateP3;
 varying vec2 textureCoordinateP2;
 
// 7x1 Gauss kernel
void main()
{
    vec4 pxL3 = texture2D(inputImageTexture, textureCoordinateN3);
    vec4 pxL2 = texture2D(inputImageTexture, textureCoordinateN2);
    vec4 pxL1 = texture2D(inputImageTexture, textureCoordinateN1);
    vec4 pxC  = texture2D(inputImageTexture, textureCoordinate);
    vec4 pxR1 = texture2D(inputImageTexture, textureCoordinateP1);
    vec4 pxR2 = texture2D(inputImageTexture, textureCoordinateP2);
    vec4 pxR3 = texture2D(inputImageTexture, textureCoordinateP3);
    
    gl_FragColor = 0.015625 * (pxL3 + pxR3) + 0.09375 * (pxL2 + pxR2) + 0.234375 * (pxL1 + pxR1) + 0.3125 * pxC;
}
);

// 1 6 15 20 15 6 1
// 20+30+12+2 = 64
// 20/64 = 0.3125
// 15/64 = 0.234375
// 6/64 = 0.09375
// 1/64 = 0.015625

const char *GaussProcPass::fshaderGauss7SrcR = OG_TO_STR
(
#if defined(OGLES_GPGPU_OPENGLES)
precision mediump float;
#endif

 uniform sampler2D inputImageTexture;

 varying vec2 textureCoordinateN3;
 varying vec2 textureCoordinateN2;
 varying vec2 textureCoordinateN1;
 varying vec2 textureCoordinate;
 varying vec2 textureCoordinateP1;
 varying vec2 textureCoordinateP2;
 varying vec2 textureCoordinateP3;
 
// 7x1 Gauss kernel
void main()
{
    float pxL3 = texture2D(inputImageTexture, textureCoordinateN3).r;
    float pxL2 = texture2D(inputImageTexture, textureCoordinateN2).r;
    float pxL1 = texture2D(inputImageTexture, textureCoordinateN1).r;
    vec4 pxC  = texture2D(inputImageTexture, textureCoordinate);
    float pxR1 = texture2D(inputImageTexture, textureCoordinateP1).r;
    float pxR2 = texture2D(inputImageTexture, textureCoordinateP2).r;
    float pxR3 = texture2D(inputImageTexture, textureCoordinateP3).r;
    
    pxC.r = (0.006 * (pxL3 + pxR3) + 0.061 * (pxL2 + pxR2) + 0.242 * (pxL1 + pxR1) + 0.382 * pxC.r);
    gl_FragColor = pxC;
});

// #### 5 tap filters ####

const char *GaussProcPass::vshaderGauss5Src = OG_TO_STR
(
 attribute vec4 position;
 attribute vec4 inputTextureCoordinate;
 
 uniform float texelWidth;
 uniform float texelHeight;
 
 varying vec2 textureCoordinateN2;
 varying vec2 textureCoordinateN1;
 varying vec2 textureCoordinate;
 varying vec2 textureCoordinateP1;
 varying vec2 textureCoordinateP2;
 
 void main()
 {
     gl_Position = position;
     
     vec2 texelStep = vec2(texelWidth, texelHeight);
     
     textureCoordinate = inputTextureCoordinate.xy;
     
     textureCoordinateN2 = textureCoordinate - texelStep * 2.0;
     textureCoordinateN1 = textureCoordinate - texelStep;
     
     textureCoordinateP1 = textureCoordinate + texelStep;
     textureCoordinateP2 = textureCoordinate + texelStep * 2.0;
 });

const char *GaussProcPass::fshaderGauss5Src = OG_TO_STR
(
 
#if defined(OGLES_GPGPU_OPENGLES)
 precision highp float;
#endif
 
 uniform sampler2D inputImageTexture;
 
 varying vec2 textureCoordinateN2;
 varying vec2 textureCoordinateN1;
 varying vec2 textureCoordinate;
 varying vec2 textureCoordinateP1;
 varying vec2 textureCoordinateP2;
 
 // 5x1 Gauss kernel
 void main()
{
    vec4 pxL2 = texture2D(inputImageTexture, textureCoordinateN2);
    vec4 pxL1 = texture2D(inputImageTexture, textureCoordinateN1);
    vec4 pxC  = texture2D(inputImageTexture, textureCoordinate);
    vec4 pxR1 = texture2D(inputImageTexture, textureCoordinateP1);
    vec4 pxR2 = texture2D(inputImageTexture, textureCoordinateP2);
    
    vec4 result = (0.0625 * (pxL2 + pxR2) + 0.25 * (pxL1 + pxR1) + 0.375 * pxC);
    gl_FragColor = result;
});

// 1 4 6 4 1
// 1+4+6+4+1 = 16
// 6/16 = 0.375
// 4/16 = 0.25
// 1/16 = 0.0625

const char *GaussProcPass::fshaderGauss5SrcR = OG_TO_STR
(
 
#if defined(OGLES_GPGPU_OPENGLES)
 precision mediump float;
#endif
 
 uniform sampler2D inputImageTexture;

 varying vec2 textureCoordinateN2;
 varying vec2 textureCoordinateN1;
 varying vec2 textureCoordinate;
 varying vec2 textureCoordinateP1;
 varying vec2 textureCoordinateP2;
 
 // 5x1 Gauss kernel
 void main()
{
    float pxL2 = texture2D(inputImageTexture, textureCoordinateN2).r;
    float pxL1 = texture2D(inputImageTexture, textureCoordinateN1).r;
    vec4 pxC  = texture2D(inputImageTexture, textureCoordinate);
    float pxR1 = texture2D(inputImageTexture, textureCoordinateP1).r;
    float pxR2 = texture2D(inputImageTexture, textureCoordinateP2).r;
    
    pxC.r = (0.0625 * (pxL2 + pxR2) + 0.25 * (pxL1 + pxR1) + 0.375 * pxC.r);
    gl_FragColor = pxC;
});

void GaussProcPass::filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target)
{
    // create shader object
    ProcBase::createShader(vShaderSrc, fShaderSrc, target);
    
    // get shader params
    shParamAPos = shader->getParam(ATTR, "position");
    shParamATexCoord = shader->getParam(ATTR, "inputTextureCoordinate");
    
    texelWidthUniform = shader->getParam(UNIF, "texelWidth");
    texelHeightUniform = shader->getParam(UNIF, "texelHeight");
    
    Tools::checkGLErr(getProcName(), "filterShaderSetup");
}

void GaussProcPass::setUniforms()
{
    FilterProcBase::setUniforms();

    glUniform1f(texelWidthUniform, (renderPass == 1) * texelWidth);
    glUniform1f(texelHeightUniform, (renderPass == 2) * texelHeight);
}

void GaussProcPass::getUniforms()
{
    FilterProcBase::getUniforms();
    
    // calculate pixel delta values
    texelWidth = 1.0f / (float)outFrameW; // input or output?
    texelHeight = 1.0f / (float)outFrameH;
    shParamUInputTex = shader->getParam(UNIF, "inputImageTexture");
}

const char *GaussProcPass::getFragmentShaderSource()
{
    switch(kernel)
    {
        case k5Tap: return doR ? fshaderGauss5SrcR : fshaderGauss5Src;
        case k7Tap: return doR ? fshaderGauss7SrcR : fshaderGauss7Src;
        default: assert(false);
    }
}

const char *GaussProcPass::getVertexShaderSource()
{
    switch(kernel)
    {
        case k5Tap: return vshaderGauss5Src;
        case k7Tap: return vshaderGauss7Src;
        default: assert(false);
    }
}

