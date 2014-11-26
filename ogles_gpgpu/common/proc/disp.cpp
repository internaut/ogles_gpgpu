#include "disp.h"

using namespace std;
using namespace ogles_gpgpu;

const char *Disp::fshaderDispSrc = TO_STR(
precision mediump float;
varying vec2 vTexCoord;
uniform sampler2D uInputTex;
void main() {
    gl_FragColor = vec4(texture2D(uInputTex, vTexCoord).rgba);
//    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
);

void Disp::init(int inW, int inH, unsigned int order, bool prepareForExternalInput) {
    cout << "ogles_gpgpu::Disp - init" << endl;
    
    // ProcBase init - set defaults
    baseInit(inW, inH, order, prepareForExternalInput, procParamOutW, procParamOutH, procParamOutScale);
    
    // FilterProcBase init - create shaders, get shader params, set buffers for OpenGL
    filterInit(fshaderDispSrc);
}

void Disp::render() {
    cout << "ogles_gpgpu::Disp - input tex " << texId << " / render to framebuffer of size " << outFrameW << "x" << outFrameH << endl;
    
    filterRenderPrepare();
    Tools::checkGLErr("ogles_gpgpu::Disp - render prepare");
    
    filterRenderSetCoords();
    Tools::checkGLErr("ogles_gpgpu::Disp - render set coords");
    
    filterRenderDraw();
    Tools::checkGLErr("ogles_gpgpu::Disp - render draw");
    
    filterRenderCleanup();
    Tools::checkGLErr("ogles_gpgpu::Disp - render cleanup");
}
