//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015  http://www.mkonrad.net
//         David Hirvonen
// See LICENSE file in project repository root for the license.
//

#include "../common_includes.h"
#include "nms.h"

using namespace std;
using namespace ogles_gpgpu;

const char *NmsProc::fshaderNmsSrc = OG_TO_STR(

#if defined(OGLES_GPGPU_OPENGLES)
precision OGLES_GPGPU_HIGHP float;
#endif

 uniform sampler2D inputImageTexture;
 
 varying OGLES_GPGPU_HIGHP vec2 textureCoordinate;
 varying OGLES_GPGPU_HIGHP vec2 leftTextureCoordinate;
 varying OGLES_GPGPU_HIGHP vec2 rightTextureCoordinate;
 
 varying OGLES_GPGPU_HIGHP vec2 topTextureCoordinate;
 varying OGLES_GPGPU_HIGHP vec2 topLeftTextureCoordinate;
 varying OGLES_GPGPU_HIGHP vec2 topRightTextureCoordinate;
 
 varying OGLES_GPGPU_HIGHP vec2 bottomTextureCoordinate;
 varying OGLES_GPGPU_HIGHP vec2 bottomLeftTextureCoordinate;
 varying OGLES_GPGPU_HIGHP vec2 bottomRightTextureCoordinate;
 
 uniform OGLES_GPGPU_HIGHP float threshold;
 
 void main()
 {
     OGLES_GPGPU_LOWP float bottomColor = texture2D(inputImageTexture, bottomTextureCoordinate).r;
     OGLES_GPGPU_LOWP float bottomLeftColor = texture2D(inputImageTexture, bottomLeftTextureCoordinate).r;
     OGLES_GPGPU_LOWP float bottomRightColor = texture2D(inputImageTexture, bottomRightTextureCoordinate).r;
     OGLES_GPGPU_LOWP vec4 centerColor = texture2D(inputImageTexture, textureCoordinate);
     OGLES_GPGPU_LOWP float leftColor = texture2D(inputImageTexture, leftTextureCoordinate).r;
     OGLES_GPGPU_LOWP float rightColor = texture2D(inputImageTexture, rightTextureCoordinate).r;
     OGLES_GPGPU_LOWP float topColor = texture2D(inputImageTexture, topTextureCoordinate).r;
     OGLES_GPGPU_LOWP float topRightColor = texture2D(inputImageTexture, topRightTextureCoordinate).r;
     OGLES_GPGPU_LOWP float topLeftColor = texture2D(inputImageTexture, topLeftTextureCoordinate).r;
     
     // Use a tiebreaker for pixels to the left and immediately above this one
     OGLES_GPGPU_LOWP float multiplier = 1.0 - step(centerColor.r, topColor);
     multiplier = multiplier * (1.0 - step(centerColor.r, topLeftColor));
     multiplier = multiplier * (1.0 - step(centerColor.r, leftColor));
     multiplier = multiplier * (1.0 - step(centerColor.r, bottomLeftColor));
     
     OGLES_GPGPU_LOWP float maxValue = max(centerColor.r, bottomColor);
     maxValue = max(maxValue, bottomRightColor);
     maxValue = max(maxValue, rightColor);
     maxValue = max(maxValue, topRightColor);
     
     OGLES_GPGPU_LOWP float finalValue = centerColor.r * step(maxValue, centerColor.r) * multiplier;
     finalValue = step(threshold, finalValue);
     
     gl_FragColor = vec4(vec3(finalValue), 1.0);
});

NmsProc::NmsProc() {

}

void NmsProc::setUniforms() {
    Filter3x3Proc::setUniforms();
    glUniform1f(shParamUThreshold, threshold);
}

void NmsProc::getUniforms() {
    Filter3x3Proc::getUniforms();
    shParamUInputTex = shader->getParam(UNIF, "inputImageTexture");
    shParamUThreshold = shader->getParam(UNIF, "threshold");
}

