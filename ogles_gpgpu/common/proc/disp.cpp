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
    OG_LOGINF(getProcName(), "");
    
    // ProcBase init - set defaults
    baseInit(inW, inH, order, prepareForExternalInput, procParamOutW, procParamOutH, procParamOutScale);
    
    // FilterProcBase init - create shaders, get shader params, set buffers for OpenGL
    filterInit(fshaderDispSrc);
    
    return 1;
}

void Disp::render() {
    OG_LOGINF(getProcName(), "input tex %d, framebuffer of size %dx%d", texId, outFrameW, outFrameH);
    
    filterRenderPrepare();
    Tools::checkGLErr(getProcName(), "render prepare");
    
    filterRenderSetCoords();
    Tools::checkGLErr(getProcName(), "render set coords");
    
    filterRenderDraw();
    Tools::checkGLErr(getProcName(), "render draw");
    
    filterRenderCleanup();
    Tools::checkGLErr(getProcName(), "render cleanup");
}
