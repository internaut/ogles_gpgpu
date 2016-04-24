#include "three.h"

using namespace std;
using namespace ogles_gpgpu;

const char *ThreeInputProc::vshaderThreeInputSrc = OG_TO_STR(

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

const char *ThreeInputProc::fshaderThreeInputSrc = OG_TO_STR(

#if defined(OGLES_GPGPU_OPENGLES)
precision mediump float;
#endif

 varying vec2 textureCoordinate;

 uniform sampler2D inputImageTexture;
 uniform sampler2D inputImageTexture2;
 uniform sampler2D inputImageTexture3;
 
 void main()
 {
	 vec4 textureColor = texture2D(inputImageTexture, textureCoordinate);
	 vec4 textureColor2 = texture2D(inputImageTexture2, textureCoordinate);
     vec4 textureColor3 = texture2D(inputImageTexture3, textureCoordinate);
	 gl_FragColor = vec4(textureColor.rgb - textureColor2.rgb, textureColor.a);
});

ThreeInputProc::ThreeInputProc()
{
    
}

int ThreeInputProc::render(int position)
{
    int result = 1;
    
    switch(position)
    {
        case 0: hasTex1 = true; break;
        case 1: hasTex2 = true; break;
        case 2: hasTex3 = true; break;
        default: assert(false);
    }
    
    if((hasTex1 || !waitForFirstTexture) && (hasTex2 || !waitForSecondTexture) && (hasTex3 || !waitForThirdTexture))
    {
        result = FilterProcBase::render(position);
        hasTex1 = hasTex2 = hasTex3 = false;
    }
    return result; // 0 on success
}

/**
 * Use texture id <id> as input texture at texture <useTexUnit> with texture target <target>.
 */

void ThreeInputProc::useTexture(GLuint id, GLuint useTexUnit, GLenum target, int position)
{
    switch(position)
    {
        case 0: FilterProcBase::useTexture(id, useTexUnit, target, position); break;
        case 1: useTexture2(id, useTexUnit, target); break;
        case 2: useTexture3(id, useTexUnit, target); break;
        default: assert(false);
    }
}

void ThreeInputProc::useTexture2(GLuint id, GLuint useTexUnit, GLenum target)
{
    texId2 = id;
    texUnit2 = useTexUnit;
    texTarget2 = target;
}

void ThreeInputProc::useTexture3(GLuint id, GLuint useTexUnit, GLenum target)
{
    texId3 = id;
    texUnit3 = useTexUnit;
    texTarget3 = target;
}

void ThreeInputProc::filterRenderPrepare()
{
    shader->use();
    
    // set the viewport
    glViewport(0, 0, outFrameW, outFrameH);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Bind input texture 1:
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(texTarget, texId);
    glUniform1i(shParamUInputTex, texUnit);
    
    // Bind input texture 2:
    texUnit2 = texUnit + 1;
    glActiveTexture(GL_TEXTURE0 + texUnit2);
    glBindTexture(texTarget2, texId2);
    glUniform1i(shParamUInputTex2, texUnit2);

    // Bind input texture 3:
    texUnit3 = texUnit + 2;
    glActiveTexture(GL_TEXTURE0 + texUnit3);
    glBindTexture(texTarget3, texId3);
    glUniform1i(shParamUInputTex3, texUnit3);
}

void ThreeInputProc::filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target)
{
    // create shader object
    FilterProcBase::createShader(vShaderSrc, fShaderSrc, target);
    
    shParamAPos = shader->getParam(ATTR, "position");
    shParamATexCoord = shader->getParam(ATTR, "inputTextureCoordinate");
    
    // remember used shader source
    vertexShaderSrcForCompilation = vShaderSrc;
    fragShaderSrcForCompilation = fShaderSrc;
}

void ThreeInputProc::getUniforms()
{
    FilterProcBase::getUniforms();
    shParamUInputTex = shader->getParam(UNIF, "inputImageTexture");
    shParamUInputTex2 = shader->getParam(UNIF, "inputImageTexture2");
    shParamUInputTex3 = shader->getParam(UNIF, "inputImageTexture3");
}

void ThreeInputProc::setUniforms()
{
    FilterProcBase::setUniforms();
}


