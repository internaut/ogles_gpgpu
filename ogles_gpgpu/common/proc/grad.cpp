//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "../common_includes.h"
#include "grad.h"

using namespace std;
using namespace ogles_gpgpu;

const char *GradProc::fshaderGradSrc = OG_TO_STR
(
#if defined(OGLES_GPGPU_OPENGLES)
precision highp float;
#endif
 
 varying vec2 textureCoordinate;
 varying vec2 leftTextureCoordinate;
 varying vec2 rightTextureCoordinate;
 
 varying vec2 topTextureCoordinate;
 varying vec2 topLeftTextureCoordinate;
 varying vec2 topRightTextureCoordinate;
 
 varying vec2 bottomTextureCoordinate;
 varying vec2 bottomLeftTextureCoordinate;
 varying vec2 bottomRightTextureCoordinate;
 
 uniform sampler2D inputImageTexture;
 
 uniform float strength;
 
 const float pi = 3.14159265359;
 
 void main()
 {
     float bottomLeftIntensity = texture2D(inputImageTexture, bottomLeftTextureCoordinate).r;
     float topRightIntensity = texture2D(inputImageTexture, topRightTextureCoordinate).r;
     float topLeftIntensity = texture2D(inputImageTexture, topLeftTextureCoordinate).r;
     float bottomRightIntensity = texture2D(inputImageTexture, bottomRightTextureCoordinate).r;
     float centerIntenssity = texture2D(inputImageTexture, textureCoordinate).r;
     float leftIntensity = texture2D(inputImageTexture, leftTextureCoordinate).r;
     float rightIntensity = texture2D(inputImageTexture, rightTextureCoordinate).r;
     float bottomIntensity = texture2D(inputImageTexture, bottomTextureCoordinate).r;
     float topIntensity = texture2D(inputImageTexture, topTextureCoordinate).r;

     //float y = -topLeftIntensity - (2.0 * topIntensity) - topRightIntensity + bottomLeftIntensity + (2.0 * bottomIntensity) + bottomRightIntensity;
     //float x = -bottomLeftIntensity - (2.0 * leftIntensity) - topLeftIntensity + bottomRightIntensity + (2.0 * rightIntensity) + topRightIntensity;
     //y = y / 8.0;
     //x = x / 8.0;
     
     float x = (rightIntensity - leftIntensity) / 2.0;
     float y = (bottomIntensity - topIntensity) / 2.0;
     
     float mag = length(vec2(x, y)) * strength;
     float theta = atan(y, x);
     if(theta < 0.0)
     {
         theta = theta + pi;
     }
     
     float dx = (x + 1.0) / 2.0;
     float dy = (y + 1.0) / 2.0;
     
     gl_FragColor = vec4(mag, clamp(theta/pi, 0.0, 1.0), clamp(dx, 0.0, 1.0), clamp(dy, 0.0, 1.0));
 }
 );

GradProc::GradProc(float strength) : strength(strength) {

}

void GradProc::setUniforms() {
    Filter3x3Proc::setUniforms();
    
    glUniform1f(texelWidthUniform, (1.0f/ float(outFrameW)));
    glUniform1f(texelHeightUniform, (1.0f/ float(outFrameH)));
    
    glUniform1f(shParamUStrength, strength);
}

void GradProc::getUniforms() {
    Filter3x3Proc::getUniforms();
    shParamUInputTex = shader->getParam(UNIF, "inputImageTexture");
    shParamUStrength = shader->getParam(UNIF, "strength");
}
