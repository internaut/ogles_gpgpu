//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "../common_includes.h"
#include "shitomasi.h"

using namespace std;
using namespace ogles_gpgpu;

#if 0

const char *ShiTomasiProc::fshaderShiTomasiSrc = OG_TO_STR(

#if defined(OGLES_GPGPU_OPENGLES)
precision mediump float;
#endif

varying OGLES_GPGPU_HIGHP vec2 textureCoordinate;

uniform sampler2D inputImageTexture;
uniform OGLES_GPGPU_MEDIUMP float sensitivity;

void main()
{
    vec3 derivativeElements = texture2D(inputImageTexture, textureCoordinate).rgb;
    
    float derivativeDifference = derivativeElements.x - derivativeElements.y;
    float zElement = (derivativeElements.z * 2.0) - 1.0;
    
    // R = Ix^2 + Iy^2 - sqrt( (Ix^2 - Iy^2)^2 + 4 * Ixy * Ixy)
    float cornerness = derivativeElements.x + derivativeElements.y - sqrt(derivativeDifference * derivativeDifference + 4.0 * zElement * zElement);

    gl_FragColor = vec4(vec3(cornerness * sensitivity), 1.0);
});


#else

// Try Harris:
const char *ShiTomasiProc::fshaderShiTomasiSrc = OG_TO_STR(
                                                           
#if defined(OGLES_GPGPU_OPENGLES)
precision mediump float;
#endif

varying OGLES_GPGPU_HIGHP vec2 textureCoordinate;

uniform sampler2D inputImageTexture;
uniform OGLES_GPGPU_LOWP float sensitivity;

const OGLES_GPGPU_MEDIUMP float harrisConstant = 0.04;

void main()
{
    OGLES_GPGPU_MEDIUMP vec3 derivativeElements = texture2D(inputImageTexture, textureCoordinate).rgb;
    OGLES_GPGPU_MEDIUMP float derivativeSum = derivativeElements.x + derivativeElements.y;
    OGLES_GPGPU_MEDIUMP float zElement = (derivativeElements.z * 2.0) - 1.0;
    
    // R = Ix^2 * Iy^2 - Ixy * Ixy - k * (Ix^2 + Iy^2)^2
    OGLES_GPGPU_MEDIUMP float cornerness = derivativeElements.x * derivativeElements.y - (zElement * zElement) - harrisConstant * derivativeSum * derivativeSum;
    
    gl_FragColor = vec4(vec3(cornerness * sensitivity), 1.0);
});

#endif

ShiTomasiProc::ShiTomasiProc() {

}


// TODO: We need to override this if we are using the GPUImage shaders
void ShiTomasiProc::filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target) {
    // create shader object
    ProcBase::createShader(vShaderSrc, fShaderSrc, target);
    
    // get shader params
    shParamAPos = shader->getParam(ATTR, "position");
    shParamATexCoord = shader->getParam(ATTR, "inputTextureCoordinate");
    Tools::checkGLErr(getProcName(), "filterShaderSetup");
}

void ShiTomasiProc::getUniforms() {
    FilterProcBase::getUniforms();
    shParamUInputTex = shader->getParam(UNIF, "inputImageTexture");
    shParamUInputSensitivity =  shader->getParam(UNIF, "sensitivity");
}

void ShiTomasiProc::setUniforms() {
    FilterProcBase::setUniforms(); 
    glUniform1f (shParamUInputSensitivity, sensitivity);   // set additional uniforms
}
