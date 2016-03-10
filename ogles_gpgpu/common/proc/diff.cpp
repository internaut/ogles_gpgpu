#include "../common_includes.h"
#include "diff.h"

using namespace ogles_gpgpu;

void DiffProc::getUniforms()
{
    TwoInputProc::getUniforms();
    texelWidthUniform = shader->getParam(UNIF, "texelWidth");
    texelHeightUniform = shader->getParam(UNIF, "texelHeight");
    shParamUStrength = shader->getParam(UNIF, "strength");
}

void DiffProc::setUniforms()
{
    TwoInputProc::setUniforms();
    glUniform1f(texelWidthUniform, (1.0f/ float(outFrameW)));
    glUniform1f(texelHeightUniform, (1.0f/ float(outFrameH)));
    glUniform1f(shParamUStrength, strength);
}

const char *DiffProc::fshaderDiffSrc = OG_TO_STR
(
#if defined(OGLES_GPGPU_OPENGLES)
 precision highp float;
#endif
 varying vec2 textureCoordinate;
 uniform sampler2D inputImageTexture;
 uniform sampler2D inputImageTexture2;
 uniform float strength;
 void main()
 {
     vec4 centerIntensity = texture2D(inputImageTexture, textureCoordinate);
     vec4 centerIntensity2 = texture2D(inputImageTexture2, textureCoordinate);
     vec4 dt = (centerIntensity-centerIntensity2);
     dt = (dt * strength + 1.0) / 2.0;
     gl_FragColor = clamp(dt, 0.0, 1.0);
 });

const char *DiffProc::fshaderDiff2Src = OG_TO_STR
(
#if defined(OGLES_GPGPU_OPENGLES)
 precision highp float;
#endif
 varying vec2 textureCoordinate;
 uniform sampler2D inputImageTexture;
 uniform sampler2D inputImageTexture2;
 uniform float strength;
 void main()
 {
     vec4 centerIntensity = texture2D(inputImageTexture, textureCoordinate);
     vec4 centerIntensity2 = texture2D(inputImageTexture2, textureCoordinate);
     vec4 dt = (centerIntensity-centerIntensity2);
     vec4 dt2 = (dt * dt);
     dt2 = (dt2 * strength + 1.0) / 2.0;
     gl_FragColor = clamp(dt2, 0.0, 1.0);
 });