//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "disp.h"

using namespace std;
using namespace ogles_gpgpu;

const char *Disp::fshaderDispSrc = OG_TO_STR(
precision mediump float;
varying vec2 vTexCoord;
uniform sampler2D uInputTex;
void main() {
    gl_FragColor = vec4(texture2D(uInputTex, vTexCoord).rgba);
//    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
);

int Disp::init(int inW, int inH, unsigned int order, bool prepareForExternalInput) {
    OG_LOGINF(getProcName(), "initialize");
    
    // ProcBase init - set defaults
    baseInit(inW, inH, order, prepareForExternalInput, procParamOutW, procParamOutH, procParamOutScale);
    
    // FilterProcBase init - create shaders, get shader params, set buffers for OpenGL
    filterInit(fshaderDispSrc);
    
    return 1;
}

void Disp::render() {
    OG_LOGINF(getProcName(), "input tex %d, target %d, framebuffer of size %dx%d", texId, texTarget, outFrameW, outFrameH);
    
    filterRenderPrepare();
    Tools::checkGLErr(getProcName(), "render prepare");
    
    filterRenderSetCoords();
    Tools::checkGLErr(getProcName(), "render set coords");
    
    filterRenderDraw();
    Tools::checkGLErr(getProcName(), "render draw");
    
    filterRenderCleanup();
    Tools::checkGLErr(getProcName(), "render cleanup");
}
