#include "procbase.h"

using namespace ogles_gpgpu;
using namespace std;

const char *ProcBase::vshaderDefault = TO_STR
(
attribute vec4 aPos;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;
void main() {
    gl_Position = aPos;
    vTexCoord = aTexCoord;
}
);

const GLfloat ProcBase::quadTexCoordsStd[] = {
    0, 0,
    1, 0,
    0, 1,
    1, 1 };

const GLfloat ProcBase::quadTexCoordsFlipped[] = {
    0, 1,
    1, 1,
    0, 0,
    1, 0 };

const GLfloat ProcBase::quadTexCoordsDiagonal[] = {
    0, 0,
    0, 1,
    1, 0,
    1, 1 };

const GLfloat ProcBase::quadVertices[] = {
    -1, -1, 0,
     1, -1, 0,
    -1,  1, 0,
     1,  1, 0 };


ProcBase::ProcBase() {
	texId = 0;
    texUnit = 1;
	shader = NULL;
    fbo = NULL;
    
    procParamOutW = procParamOutH = 0;
    procParamOutScale = 1.0f;
}

ProcBase::~ProcBase() {
	if (fbo) {
		delete fbo;
	}
    
	if (shader) {
		delete shader;
	}
}

void ProcBase::getResultData(unsigned char *data) const {
    assert(fbo != NULL);
    fbo->readBuffer(data);
}

GLuint ProcBase::getOutputTexId() const {
    assert(fbo != NULL);
    
    return fbo->getAttachedTexId();
}

void ProcBase::baseInit(int inW, int inH, unsigned int order, int outW, int outH, float scaleFactor) {
    assert(inW > 0 && inH > 0);
    
    orderNum = order;
    
    inFrameW = inW;
    inFrameH = inH;
    
    if (outW <= 0) {
        outW = (int)((float)inW * scaleFactor);
    }
    
    if (outH <= 0) {
        outH = (int)((float)inH * scaleFactor);
    }
    
    assert(outW > 0 && outH > 0);
    
    outFrameW = outW;
    outFrameH = outH;
    
    cout << "ogles_gpgpu::ProcBase - init with "
         << "order number " << orderNum
         << ", input texture id " << texId
         << ", input frame size " << inFrameW << "x" << inFrameH
         << ", output frame size " << outFrameW << "x" << outFrameH
         << endl;
}

void ProcBase::createFBO() {
    assert(fbo == NULL);
    
    fbo = new FBO();
    fbo->setGLTexUnit(1);
    fbo->generateIds();
}

void ProcBase::createShader(const char *vShSrc, const char *fShSrc) {
    assert(shader == NULL);
    
    shader = new Shader();
    bool compiled = shader->buildFromSrc(vShSrc, fShSrc);
    
    assert(compiled);
}