#include "../common_includes.h"
#include "diff.h"

using namespace ogles_gpgpu;

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
     vec3 dt = (centerIntensity.rgb-centerIntensity2.rgb) * strength;
     gl_FragColor = vec4(vec3(clamp(dt, 0.0, 1.0)), 1.0);
 });


DiffProc::DiffProc(float strength) : strength(strength)
{
    
}

void DiffProc::getUniforms()
{
    TwoInputProc::getUniforms();
    shParamUStrength = shader->getParam(UNIF, "strength");
}

void DiffProc::setUniforms()
{
    TwoInputProc::setUniforms();
    glUniform1f(shParamUStrength, strength);
}

int DiffProc::render(int position)
{
    return TwoInputProc::render(position);
}
