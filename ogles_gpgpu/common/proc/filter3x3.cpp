//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
//         David Hirvonen
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "../common_includes.h"
#include "filter3x3.h"

using namespace std;
using namespace ogles_gpgpu;

const char *Filter3x3Proc::vshaderFilter3x3Src = OG_TO_STR(

 attribute vec4 position;
 attribute vec4 inputTextureCoordinate;
 
 uniform float texelWidth;
 uniform float texelHeight;
 
 varying vec2 textureCoordinate;
 varying vec2 leftTextureCoordinate;
 varying vec2 rightTextureCoordinate;
 
 varying vec2 topTextureCoordinate;
 varying vec2 topLeftTextureCoordinate;
 varying vec2 topRightTextureCoordinate;
 
 varying vec2 bottomTextureCoordinate;
 varying vec2 bottomLeftTextureCoordinate;
 varying vec2 bottomRightTextureCoordinate;
 
 void main()
 {
     gl_Position = position;
     
     vec2 widthStep = vec2(texelWidth, 0.0);
     vec2 heightStep = vec2(0.0, texelHeight);
     vec2 widthHeightStep = vec2(texelWidth, texelHeight);
     vec2 widthNegativeHeightStep = vec2(texelWidth, -texelHeight);
     
     textureCoordinate = inputTextureCoordinate.xy;
     leftTextureCoordinate = inputTextureCoordinate.xy - widthStep;
     rightTextureCoordinate = inputTextureCoordinate.xy + widthStep;
     
     topTextureCoordinate = inputTextureCoordinate.xy - heightStep;
     topLeftTextureCoordinate = inputTextureCoordinate.xy - widthHeightStep;
     topRightTextureCoordinate = inputTextureCoordinate.xy + widthNegativeHeightStep;
     
     bottomTextureCoordinate = inputTextureCoordinate.xy + heightStep;
     bottomLeftTextureCoordinate = inputTextureCoordinate.xy - widthNegativeHeightStep;
     bottomRightTextureCoordinate = inputTextureCoordinate.xy + widthHeightStep;
 }
 );

Filter3x3Proc::Filter3x3Proc() {

}

void Filter3x3Proc::filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target)
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

void Filter3x3Proc::getUniforms() {
    
}

void Filter3x3Proc::setUniforms() {
    // Set texel width/height uniforms:
    glUniform1f(texelWidthUniform, (1.0f/ float(inFrameW)));
    glUniform1f(texelHeightUniform, (1.0f/ float(inFrameH)));
}

