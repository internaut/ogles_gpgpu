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
 attribute vec4 inputTextureCoordinate2;
 
 varying vec2 textureCoordinate;
 varying vec2 textureCoordinate2;
 
 void main()
 {
     gl_Position = position;
     textureCoordinate = inputTextureCoordinate.xy;
     textureCoordinate2 = inputTextureCoordinate2.xy;
 });

const char *TwoInputProc::fshaderTwoInputSrc = OG_TO_STR(

#if defined(OGLES_GPGPU_OPENGLES)
precision mediump float;
#endif

 varying highp vec2 textureCoordinate;
 varying highp vec2 textureCoordinate2;

 uniform sampler2D inputImageTexture;
 uniform sampler2D inputImageTexture2;
 
 void main()
 {
	 lowp vec4 textureColor = texture2D(inputImageTexture, textureCoordinate);
	 lowp vec4 textureColor2 = texture2D(inputImageTexture2, textureCoordinate2);
	 
	 gl_FragColor = vec4(textureColor.rgb - textureColor2.rgb, textureColor.a);
});

TwoInputProc::TwoInputProc()
{

}

void TwoInputProc::setUniforms()
{

}

void TwoInputProc::getUniforms()
{

}

int TwoInputProc::init(int inW, int inH, unsigned int order, bool prepareForExternalInput)
{
    return FilterProcBase::init(inW, inH, order, prepareForExternalInput);
}

void TwoInputProc::render()
{
    OG_LOGINF(getProcName(), "input tex %d, target %d, framebuffer of size %dx%d", texId, texTarget, outFrameW, outFrameH);
    filterRenderPrepare();
    
    setUniforms();
    
    Tools::checkGLErr(getProcName(), "render prepare");
    
    filterRenderSetCoords(); //   if (fbo) fbo->bind();
    Tools::checkGLErr(getProcName(), "render set coords");
    
    filterRenderCleanup();
    Tools::checkGLErr(getProcName(), "render cleanup");
}
