//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "../common_includes.h"
#include "fifo.h"

using namespace std;
using namespace ogles_gpgpu;

const char *FIFOProc::fshaderFIFOSrc = OG_TO_STR(

#if defined(OGLES_GPGPU_OPENGLES)
precision mediump float;
#endif

uniform sampler2D uInputTex;
varying vec2 vTexCoord;

void main()
{
    gl_FragColor = texture2D(uInputTex, vTexCoord)
}
);

FIFOProc::FIFOProc()
{

}

void FIFOProc::setUniforms()
{

}

void FIFOProc::getUniforms()
{

}

int FIFOProc::init(int inW, int inH, unsigned int order, bool prepareForExternalInput)
{
    // Create FBO to store the previous texture
    prevFBO = std::make_shared<FBO>();
    prevFBO->setGLTexUnit(1);
    
    return FilterProcBase::init(inW, inH, order, prepareForExternalInput);
}

void FIFOProc::render()
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
