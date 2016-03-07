//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "../common_includes.h"
#include "two.h"

using namespace std;
using namespace ogles_gpgpu;

const char *TwoInputProc::vshaderTwoInputSrc = OG_TO_STR(

#if defined(OGLES_GPGPU_OPENGLES)
precision mediump float;
#endif
    
 attribute vec4 position;
 attribute vec4 inputTextureCoordinate;
 
 varying vec2 textureCoordinate;
 
 void main()
 {
     gl_Position = position;
     textureCoordinate = inputTextureCoordinate.xy;
 });

const char *TwoInputProc::fshaderTwoInputSrc = OG_TO_STR(

#if defined(OGLES_GPGPU_OPENGLES)
precision mediump float;
#endif

 varying vec2 textureCoordinate;

 uniform sampler2D inputImageTexture;
 uniform sampler2D inputImageTexture2;
 
 void main()
 {
	 vec4 textureColor = texture2D(inputImageTexture, textureCoordinate);
	 vec4 textureColor2 = texture2D(inputImageTexture2, textureCoordinate);
	 
	 gl_FragColor = vec4(textureColor.rgb - textureColor2.rgb, textureColor.a);
});

TwoInputProc::TwoInputProc()
{
    
}

/**
 * Use texture id <id> as input texture at texture <useTexUnit> with texture target <target>.
 */

void TwoInputProc::useTexture(GLuint id, GLuint useTexUnit, GLenum target, int position)
{
    switch(position)
    {
        case 0: FilterProcBase::useTexture(id, useTexUnit, target, position); break;
        case 1: useTexture2(id, useTexUnit, target); break;
        default: assert(false);
    }
}


void TwoInputProc::useTexture2(GLuint id, GLuint useTexUnit, GLenum target)
{
    texId2 = id;
    texUnit2 = useTexUnit;
    texTarget2 = target;
}

void TwoInputProc::filterRenderPrepare()
{
    shader->use();
    Tools::checkGLErr(getProcName(), "shader->use()");
    
    // set the viewport
    glViewport(0, 0, outFrameW, outFrameH);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Bind input textures
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(texTarget, texId);
    glUniform1i(shParamUInputTex, texUnit);
    Tools::checkGLErr(getProcName(), "A");
    
    texUnit2 = texUnit + 1;
    glActiveTexture(GL_TEXTURE0 + texUnit2);
    glBindTexture(texTarget2, texId2);
    glUniform1i(shParamUInputTex2, texUnit2);
    
    Tools::checkGLErr(getProcName(), "B");
}

void TwoInputProc::filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target)
{
    // create shader object
    FilterProcBase::createShader(vShaderSrc, fShaderSrc, target);
    
    // TODO: Support uniform reads through virtual API call:
    shParamAPos = shader->getParam(ATTR, "position");
    shParamATexCoord = shader->getParam(ATTR, "inputTextureCoordinate");
    
    // remember used shader source
    vertexShaderSrcForCompilation = vShaderSrc;
    fragShaderSrcForCompilation = fShaderSrc;
}

void TwoInputProc::getUniforms()
{
    FilterProcBase::getUniforms();
    shParamUInputTex = shader->getParam(UNIF, "inputImageTexture");
    shParamUInputTex2 = shader->getParam(UNIF, "inputImageTexture2");
}

void TwoInputProc::setUniforms()
{
    FilterProcBase::setUniforms();
}


