#include "grayscale.h"

using namespace std;
using namespace ogles_gpgpu;

const GLfloat GrayscaleProc::grayscaleConvVecRGB[3] = {
    0.299, 0.587, 0.114
};

const GLfloat GrayscaleProc::grayscaleConvVecBGR[3] = {
    0.114, 0.587, 0.299
};

const char *GrayscaleProc::fshaderGrayscaleSrc = TO_STR(
precision mediump float;
varying vec2 vTexCoord;
uniform sampler2D uInputTex;
uniform vec3 uInputConvVec;
void main() {
    float gray = dot(texture2D(uInputTex, vTexCoord).rgb, uInputConvVec);
    gl_FragColor = vec4(gray, gray, gray, 1.0);
}
);

GrayscaleProc::GrayscaleProc() {
    // set defaults
    inputConvType = GRAYSCALE_INPUT_CONVERSION_NONE;
    setGrayscaleConvType(GRAYSCALE_INPUT_CONVERSION_RGB);
}

int GrayscaleProc::init(int inW, int inH, unsigned int order, bool prepareForExternalInput) {
    cout << "ogles_gpgpu::GrayscaleProc - init" << endl;
    
    // create fbo for output
    createFBO();
    
    // ProcBase init - set defaults
    baseInit(inW, inH, order, prepareForExternalInput, procParamOutW, procParamOutH, procParamOutScale);
    
    // FilterProcBase init - create shaders, get shader params, set buffers for OpenGL
    filterInit(fshaderGrayscaleSrc);
    
    // get additional shader params
    shParamUInputConvVec = shader->getParam(UNIF, "uInputConvVec");
    
    return 1;
}

void GrayscaleProc::render() {
    cout << "ogles_gpgpu::GrayscaleProc - input tex " << texId << " / render to framebuffer of size " << outFrameW << "x" << outFrameH << endl;
    
    filterRenderPrepare();
    glUniform3fv(shParamUInputConvVec, 1, grayscaleConvVec);        // set additional uniforms
    Tools::checkGLErr("ogles_gpgpu::GrayscaleProc - render prepare");
    
    filterRenderSetCoords();
    Tools::checkGLErr("ogles_gpgpu::GrayscaleProc - render set coords");
    
    filterRenderDraw();
    Tools::checkGLErr("ogles_gpgpu::GrayscaleProc - render draw");
    
    filterRenderCleanup();
    Tools::checkGLErr("ogles_gpgpu::GrayscaleProc - render cleanup");
}

void GrayscaleProc::setGrayscaleConvVec(const GLfloat v[3]) {
    inputConvType = GRAYSCALE_INPUT_CONVERSION_CUSTOM;
    memcpy(grayscaleConvVec, v, sizeof(GLfloat) * 3);
}

void GrayscaleProc::setGrayscaleConvType(GrayscaleInputConversionType type) {
    if (inputConvType == type) return;  // no change
    
    const GLfloat *v = NULL;
    
    if (type == GRAYSCALE_INPUT_CONVERSION_RGB) {
        v = &grayscaleConvVecRGB[0];
    } else if (type == GRAYSCALE_INPUT_CONVERSION_BGR) {
        v = &grayscaleConvVecBGR[0];
    } else {
        cerr << "ogles_gpgpu::GrayscaleProc - unknown grayscale input conversion type " << type << endl;
        v = &grayscaleConvVecRGB[0];    // set default
    }
    
    memcpy(grayscaleConvVec, v, sizeof(GLfloat) * 3);
    
    inputConvType = type;
}