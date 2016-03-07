//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 (http://www.mkonrad.net)
//         David Hirvonen
//
// See LICENSE file in project repository root for the license.
//

#include "../common_includes.h"
#include "tensor.h"

using namespace std;
using namespace ogles_gpgpu;

// Source: GPUImageXYDerivativeFilter.m
const char *TensorProc::fshaderTensorSrc = OG_TO_STR(

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

uniform float edgeStrength;

void main()
{
    float topIntensity = texture2D(inputImageTexture, topTextureCoordinate).r;
    float topRightIntensity = texture2D(inputImageTexture, topRightTextureCoordinate).r;
    float topLeftIntensity = texture2D(inputImageTexture, topLeftTextureCoordinate).r;
    float bottomIntensity = texture2D(inputImageTexture, bottomTextureCoordinate).r;
    float centerIntensity = texture2D(inputImageTexture, textureCoordinate).r;
    float bottomLeftIntensity = texture2D(inputImageTexture, bottomLeftTextureCoordinate).r;
    float bottomRightIntensity = texture2D(inputImageTexture, bottomRightTextureCoordinate).r;
    float leftIntensity = texture2D(inputImageTexture, leftTextureCoordinate).r;
    float rightIntensity = texture2D(inputImageTexture, rightTextureCoordinate).r;
    
    float verticalDerivative = -topLeftIntensity - topIntensity - topRightIntensity + bottomLeftIntensity + bottomIntensity + bottomRightIntensity;
    float horizontalDerivative = -bottomLeftIntensity - leftIntensity - topLeftIntensity + bottomRightIntensity + rightIntensity + topRightIntensity;
    verticalDerivative = verticalDerivative * edgeStrength;
    horizontalDerivative = horizontalDerivative * edgeStrength;
    
    float xx = horizontalDerivative * horizontalDerivative;
    float yy = verticalDerivative * verticalDerivative;
    float xy = horizontalDerivative * verticalDerivative;
    // Scaling the X * Y operation so that negative numbers are not clipped in the 0..1 range.
    // This will be expanded in the corner detection filter
    
    // Also pass through the center intensity in teh alpha channel, which
    // is useful for optical flow.
    gl_FragColor = vec4(xx, yy, (xy + 1.0) / 2.0, centerIntensity);
});

TensorProc::TensorProc() {

}

void TensorProc::setUniforms() {
    Filter3x3Proc::setUniforms();
    glUniform1f(shParamUEdgeStrength, edgeStrength);
}

void TensorProc::getUniforms() {
    Filter3x3Proc::getUniforms();
    shParamUInputTex = shader->getParam(UNIF, "inputImageTexture");
    shParamUEdgeStrength = shader->getParam(UNIF, "edgeStrength");
}

