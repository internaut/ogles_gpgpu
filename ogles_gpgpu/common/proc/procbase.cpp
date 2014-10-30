#include "procbase.h"

using namespace ogles_gpgpu;
using namespace std;

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

const GLfloat ProcBase::quadVertices[] = {
    -1, -1, 0,
     1, -1, 0,
    -1,  1, 0,
     1,  1, 0 };


ProcBase::ProcBase() {
	texId = 0;
	shader = NULL;
    fbo = NULL;
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

void ProcBase::baseInit(int inW, int inH, int outW, int outH, float scaleFactor) {
    assert(inW > 0 && inH > 0);
    
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
    
    cout << "ogles_gpgpu::ProcBase - init with input frame size "
    << inFrameW << "x" << inFrameH
    << ", output frame size " << outFrameW << "x" << outFrameH
    << endl;
}

void ProcBase::createFBO() {
    assert(fbo == NULL);
    
    fbo = new FBO();
    fbo->generateIds();
}

void ProcBase::createShader(const char *vShSrc, const char *fShSrc) {
    assert(shader == NULL);
    
    shader = new Shader();
    bool compiled = shader->buildFromSrc(vShSrc, fShSrc);
    
    if (!compiled) {
        cerr << "ogles_gpgpu::ProcBase - shader could not be compiled" << endl;
    }
}