//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "../common_includes.h"
#include "base/filterprocbase.h"
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

// negative modulo arithmetic
static int modulo(int a, int b) { return (((a % b) + b) % b); }

FifoProc::FifoProc(int size) {
    m_inputIndex = m_outputIndex = 0;
    for(int i = 0; i < size; i++) {
        procPasses.push_back( new NoopProc );
    }
    
    delayedSubscribers.resize(size);
}

FifoProc::~FifoProc() {
    // remove all pass instances
    for(auto &it : procPasses) {
        delete it;
    }
    procPasses.clear();
}

void FifoProc::addWithDelay(ProcInterface *filter, int position, int time)
{
    assert(time >= 0 && time < size());
    delayedSubscribers[time].emplace_back(filter, position);
}

void FifoProc::prepare(int inW, int inH, int index, int position)
{
    assert(position == 0);
    ProcInterface::prepare(inW, inH, index, position);
    
    for(int i = 0; i < delayedSubscribers.size(); i++)
    {
        for(auto &subscriber : delayedSubscribers[i])
        {
            // At startup we have to initialize with our main processor output
            subscriber.first->prepare(getOutFrameW(), getOutFrameH(), index+1, subscriber.second);
            subscriber.first->useTexture(getOutputTexId(), getTextureUnit(), GL_TEXTURE_2D, subscriber.second);
        }
    }
}

void FifoProc::process(int position, Logger logger)
{
    assert(position == 0);
    ProcInterface::process(position, logger);
    
    if(isFull())
    {
        // Trigger delayed subscribers:
        for(int i = 0; i < delayedSubscribers.size(); i++)
        {
            auto producer = (*this)[i];
            for(auto &subscriber : delayedSubscribers[i])
            {
                subscriber.first->useTexture(producer->getOutputTexId(), producer->getTextureUnit(), GL_TEXTURE_2D, subscriber.second);
                subscriber.first->process(subscriber.second, logger);
            }
        }
    }
}

ProcInterface * FifoProc::operator[](int i) const
{
    int index = modulo(m_outputIndex+i, procPasses.size());
    return procPasses[index];
}

int FifoProc::getIn() const {
    assert(m_inputIndex >= 0 && m_inputIndex < procPasses.size());
    return m_inputIndex;
}

int FifoProc::getOut() const {
    assert(m_outputIndex >= 0 && m_outputIndex < procPasses.size());
    return m_outputIndex;
}

ProcInterface* FifoProc::getInputFilter() const { return procPasses[getIn()]; }
ProcInterface* FifoProc::getOutputFilter() const { return procPasses[getOut()]; }

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

int FifoProc::render(int position) {
    // Render into input FBO
    if(m_count == int(size())) {
        m_outputIndex = modulo(m_inputIndex + 1, size());
    }

    getInputFilter()->render();
    
    m_count = std::min(m_count + 1, int(size()));
    m_inputIndex = modulo(m_inputIndex + 1, size());
    
    return 0;
}

void FifoProc::useTexture(GLuint id, GLuint useTexUnit, GLenum target, int position) {
    assert(position == 0); // no multi-input filters for FIFO
    for(auto &it : procPasses) {
        it->useTexture(id, useTexUnit, target, position);
    }
}

// All output sizes will be the same
void FifoProc::setOutputSize(float scaleFactor) {
    for(auto &it : procPasses) {
        it->setOutputSize(scaleFactor);
    }
}

// All output sizes will be the same
void FifoProc::setOutputSize(int outW, int outH) {
    for(auto &it : procPasses) {
        it->setOutputSize(outW, outH);
    }
}
