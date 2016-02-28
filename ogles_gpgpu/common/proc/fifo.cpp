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

class NoopProc : public ogles_gpgpu::FilterProcBase {
public:
    NoopProc(float gain=1.f) : gain(gain) {}
    virtual const char *getProcName() { return "NoopProc"; }
private:
    virtual const char *getFragmentShaderSource() { return fshaderNoopSrc; }
    virtual void getUniforms() { shParamUGain = shader->getParam(UNIF, "gain"); }
    virtual void setUniforms() { glUniform1f(shParamUGain, gain); }
    static const char *fshaderNoopSrc; // fragment shader source
    float gain = 1.f;
    GLint shParamUGain;
};

const char * NoopProc::fshaderNoopSrc = OG_TO_STR
(
#if defined(OGLES_GPGPU_OPENGLES)
 precision mediump float;
#endif
 varying vec2 vTexCoord;
 uniform sampler2D uInputTex;
 uniform float gain;
 void main()
 {
     vec4 val = texture2D(uInputTex, vTexCoord);
     gl_FragColor = clamp(val * gain, 0.0, 1.0);
 });

// #################### FIFO ####################

GLuint FifoProc::getTexId(int index) const {
    assert(index >= 0 && index < procPasses.size());
    return procPasses[index]->getOutputTexId();
}

int FifoProc::getIn() const {
    return m_inputIndex;
}

int FifoProc::getOut() const {
    return m_inputIndex;
}

FifoProc::FifoProc(int size) {
    for(int i = 0; i < size; i++) {
        procPasses.push_back( new NoopProc );
    }
}

FifoProc::~FifoProc() {
    // remove all pass instances
    for(auto &it : procPasses) {
        delete it;
    }
    
    procPasses.clear();
}

#pragma mark ProcInterface methods

int FifoProc::init(int inW, int inH, unsigned int order, bool prepareForExternalInput) {
    
    m_inputIndex = m_outputIndex = m_count = 0;
    
    int num = 0;
    for(auto &it : procPasses) {
        num += it->init(inW, inH, num, prepareForExternalInput);
    }
    return num;
}

int FifoProc::reinit(int inW, int inH, bool prepareForExternalInput) {
    
    m_inputIndex = m_outputIndex = m_count = 0;
    
    int num = 0;
    for(auto &it : procPasses) {
        num += it->reinit(inW, inH, prepareForExternalInput);
    }
    return num;
}

void FifoProc::cleanup() {
    for(auto &it : procPasses) {
        it->cleanup();
    }
}

void FifoProc::setExternalInputDataFormat(GLenum fmt) {
    // Render into input FBO
    assert(m_inputIndex >= 0 && m_inputIndex < procPasses.size());
    return procPasses[m_inputIndex]->setExternalInputDataFormat(fmt);
}

void FifoProc::setExternalInputData(const unsigned char *data) {
    // Render into input FBO
    assert(m_inputIndex >= 0 && m_inputIndex < procPasses.size());
    return procPasses[m_inputIndex]->setExternalInputData(data);
}

void FifoProc::createFBOTex(bool genMipmap) {
    for(auto &it : procPasses) {
        it->createFBOTex(genMipmap);
    }
}

// 0 : [0][ ][ ]
//     [I][ ][ ]
//     [O][ ][ ]
//
// 1 : [0][1][ ]
//     [ ][I][ ]
//     [O][ ][ ]
//
// 2 : [0][1][2]
//     [ ][ ][I]
//     [O][ ][ ]
//
// 3 : [3][1][2]
//     [I][ ][ ]
//     [ ][O][ ]
//
// 4 : [3][4][2]
//     [ ][I][ ]
//     [ ][ ][O]
//
// 5 : [3][4][5]
//     [O][ ][ ]
//     [ ][ ][I]

void FifoProc::render() {
    // Render into input FBO
    assert(m_inputIndex >= 0 && m_inputIndex < procPasses.size());
    procPasses[m_inputIndex]->render();
    
    m_inputIndex = (m_inputIndex + 1) % procPasses.size();
    if(m_count == procPasses.size()) {
        m_outputIndex = (m_outputIndex + 1) % procPasses.size();
    }
    m_count = std::min(m_count + 1, int(procPasses.size()));
}

void FifoProc::printInfo() {
    OG_LOGINF(getProcName(), "begin info for %u passes", (unsigned int)procPasses.size());
    for(auto &it : procPasses) {
        it->printInfo();
    }
    OG_LOGINF(getProcName(), "end info");
}

void FifoProc::useTexture(GLuint id, GLuint useTexUnit, GLenum target) {
    assert(m_inputIndex >= 0 && m_inputIndex < procPasses.size());
    return procPasses[m_inputIndex] ->useTexture(id, useTexUnit, target);
}

GLuint FifoProc::getTextureUnit() const {
    assert(m_inputIndex >= 0 && m_inputIndex < procPasses.size());
    return procPasses[m_inputIndex] ->getTextureUnit();
}

void FifoProc::setOutputSize(float scaleFactor) {
    for(auto &it : procPasses) {
        it->setOutputSize(scaleFactor);
    }
}

void FifoProc::setOutputSize(int outW, int outH) {
    for(auto &it : procPasses) {
        it->setOutputSize(outW, outH);
    }
}

int FifoProc::getOutFrameW() const {
    assert(m_outputIndex >= 0 && m_outputIndex < procPasses.size());
    return procPasses[m_outputIndex]->getOutFrameW(); // any will do
}

int FifoProc::getOutFrameH() const {
    assert(m_outputIndex >= 0 && m_outputIndex < procPasses.size());
    return procPasses[m_outputIndex]->getOutFrameH();
}

int FifoProc::getInFrameW() const {
    assert(m_inputIndex >= 0 && m_inputIndex < procPasses.size());
    return procPasses[m_inputIndex]->getInFrameW(); // any will do
}

int FifoProc::getInFrameH() const {
    assert(m_inputIndex >= 0 && m_inputIndex < procPasses.size());
    return procPasses[m_inputIndex]->getInFrameH(); // any will do
}

bool FifoProc::getWillDownscale() const {
    assert(m_inputIndex >= 0 && m_inputIndex < procPasses.size());
    return procPasses[m_inputIndex]->getWillDownscale(); // any will do
}

void FifoProc::getResultData(unsigned char *data) const {
    assert(m_outputIndex >= 0 && m_outputIndex < procPasses.size());
    return procPasses[m_outputIndex]->getResultData(data);
}

void FifoProc::getResultData(FrameDelegate &delegate) const {
    assert(m_outputIndex >= 0 && m_outputIndex < procPasses.size());
    return procPasses[m_outputIndex]->getResultData(delegate);
}

MemTransfer *FifoProc::getMemTransferObj() const {
    assert(m_outputIndex >= 0 && m_outputIndex < procPasses.size());
    return procPasses[m_outputIndex]->getMemTransferObj();
}

MemTransfer *FifoProc::getInputMemTransferObj() const {
    assert(m_inputIndex >= 0 && m_inputIndex < procPasses.size());
    return procPasses[m_inputIndex]->getMemTransferObj();
}

GLuint FifoProc::getInputTexId() const {
    assert(m_inputIndex >= 0 && m_inputIndex < procPasses.size());
    return procPasses[m_inputIndex]->getInputTexId();
}

GLuint FifoProc::getOutputTexId() const {
    assert(m_outputIndex >= 0 && m_outputIndex < procPasses.size());
    return procPasses[m_inputIndex]->getOutputTexId();
}
