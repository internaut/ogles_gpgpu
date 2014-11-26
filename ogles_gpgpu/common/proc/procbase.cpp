#include "procbase.h"

using namespace ogles_gpgpu;
using namespace std;

const GLfloat ProcBase::quadTexCoordsStd[] = {
    0, 0,
    1, 0,
    0, 1,
    1, 1 };

const GLfloat ProcBase::quadTexCoordsStdMirrored[] = {
    1, 0,
    0, 0,
    1, 1,
    0, 1 };

const GLfloat ProcBase::quadTexCoordsFlipped[] = {
    0, 1,
    1, 1,
    0, 0,
    1, 0 };

const GLfloat ProcBase::quadTexCoordsFlippedMirrored[] = {
    1, 1,
    0, 1,
    1, 0,
    0, 0 };

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
    willDownscale = false;
    
    procParamOutW = procParamOutH = 0;
    procParamOutScale = 1.0f;
    
    renderOrientation = RenderOrientationStd;
}

ProcBase::~ProcBase() {
	if (fbo) {
		delete fbo;
	}
    
	if (shader) {
		delete shader;
	}
}

void ProcBase::printInfo() {
    assert(fbo);
    
    cout << "ogles_gpgpu::ProcBase - info: "
         << "order num " << orderNum
         << ", input tex id " << texId
         << ", input tex size " << inFrameW << "x" << inFrameH
         << ", output tex id " << fbo->getAttachedTexId()
         << ", output tex size " << outFrameW << "x" << outFrameH
         << ", will downscale: " << willDownscale
         << endl;
}

void ProcBase::getResultData(unsigned char *data) const {
    assert(fbo != NULL);
    fbo->readBuffer(data);
}

MemTransfer *ProcBase::getMemTransferObj() const {
    assert(fbo);
    
    return fbo->getMemTransfer();
}

GLuint ProcBase::getOutputTexId() const {
    assert(fbo != NULL);
    
    return fbo->getAttachedTexId();
}

void ProcBase::createFBOTex(bool genMipmap) {
    assert(fbo != NULL);
    
    fbo->createAttachedTex(outFrameW, outFrameH, genMipmap);
    
    // update frame size, because it might be set to a POT size because of mipmapping
    outFrameW = fbo->getTexWidth();
    outFrameH = fbo->getTexHeight();
}

void ProcBase::reinit(int inW, int inH, bool prepareForExternalInput) {
    assert(fbo != NULL);
    
    setInOutFrameSizes(inW, inH, procParamOutW, procParamOutH, procParamOutScale);
    
    fbo->destroyAttachedTex();  // needs to be recreated later!
    
    if (prepareForExternalInput) {    // recreate input
        useTexture(fbo->getMemTransfer()->prepareInput(inFrameW, inFrameH, inputDataFmt));
    }
    
    cout << "ogles_gpgpu::ProcBase - reinit with "
         << "input tex size " << inFrameW << "x" << inFrameH
         << ", output tex size " << outFrameW << "x" << outFrameH
         << ", will downscale: " << willDownscale
         << endl;
}

void ProcBase::baseInit(int inW, int inH, unsigned int order, bool prepareForExternalInput, int outW, int outH, float scaleFactor) {
    assert(inW > 0 && inH > 0);
    
    orderNum = order;
    
    setInOutFrameSizes(inW, inH, outW, outH, scaleFactor);
    
    // prepare for external input data
    if (prepareForExternalInput) {
        assert(fbo != NULL);
        useTexture(fbo->getMemTransfer()->prepareInput(inW, inH, inputDataFmt));
        cout << "ogles_gpgpu::ProcBase - init - prepared for external input data" << endl;
    }
    
//    cout << "ogles_gpgpu::ProcBase - init with "
//         << "order num " << orderNum
//         << ", input tex size " << inFrameW << "x" << inFrameH
//         << ", output tex size " << outFrameW << "x" << outFrameH
//         << ", will downscale: " << willDownscale
//         << endl;
}

void ProcBase::setExternalInputData(const unsigned char *data) {
    fbo->getMemTransfer()->toGPU(data);
}

void ProcBase::setInOutFrameSizes(int inW, int inH, int outW, int outH, float scaleFactor) {
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
    
    willDownscale = (outFrameW < inFrameW || outFrameH < inFrameH);
}

void ProcBase::createFBO() {
    assert(fbo == NULL);
    
    fbo = new FBO();
    fbo->setGLTexUnit(1);
}

void ProcBase::createShader(const char *vShSrc, const char *fShSrc) {
    assert(shader == NULL);
    
    shader = new Shader();
    bool compiled = shader->buildFromSrc(vShSrc, fShSrc);
    
    assert(compiled);
}