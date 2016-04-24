#include "iir.h"
#include "fifo.h"
#include "blend.h"
#include "diff.h"

/////////////////////////////
//          +===============+
//          |               |
// INPUT ===+= IIR ===+==>(DIFF)
//             ^     |
//             |     |
//           FIFO <==+
/////////////////////////////

BEGIN_OGLES_GPGPU

// Convenience initializer:
class IirFilterProc::Impl
{
public:
    Impl(FilterKind kind, float alpha, float strength)
    : kind(kind)
    , iirProc(alpha)
    , fifoProc(1)
    , diffProc(strength)
    {
        iirProc.add(&fifoProc);
        fifoProc.add(&iirProc, 1);
        if(kind == kHighPass)
        {
            iirProc.add(&diffProc, 1);
            lastProc = &diffProc; // high pass output
        }
        iirProc.setWaitForSecondTexture(false);
    }
    IirFilterProc::FilterKind kind = kLowPass;
    BlendProc iirProc;
    FIFOPRoc fifoProc;
    DiffProc diffProc;
    ProcInterface *lastProc = &iirProc; // default for low pass
};

IirFilterProc::IirFilterProc(FilterKind kind, float alpha, float strength)
{
    // All procPasses filter will be initialized by superclass:
    m_impl = std::unique_ptr<Impl>(new Impl(kind, alpha, strength));
    procPasses.push_back(&m_impl->iirProc);
    procPasses.push_back(&m_impl->fifoProc);
    if(kind == kHighPass)
    {
        procPasses.push_back(&m_impl->diffProc);
    }
}

int IirFilterProc::init(int inW, int inH, unsigned int order, bool prepareForExternalInput)
{
    isFirst = true;
    return MultiPassProc::init(inW, inH, order, prepareForExternalInput);
}

int IirFilterProc::reinit(int inW, int inH, bool prepareForExternalInput)
{
    isFirst = true;
    return MultiPassProc::reinit(inW, inH, prepareForExternalInput);
}

IirFilterProc::~IirFilterProc() {}
ProcInterface* IirFilterProc::getInputFilter() const { return &m_impl->iirProc; }
ProcInterface* IirFilterProc::getOutputFilter() const { return m_impl->lastProc; }

int IirFilterProc::render(int position)
{   // Execute internal filter chain
    m_impl->iirProc.process(0);
    if(m_impl->kind == kHighPass)
    {
        // At this point useTexture() has been called and diffProc should have:
        // texture #1 : <- Input
        // texture #2 : <- IirProc
        m_impl->diffProc.process(0);
    }
    return 0;
}

void IirFilterProc::useTexture(GLuint id, GLuint useTexUnit, GLenum target, int position)
{
    auto &fifoProc = m_impl->fifoProc;
    auto &iirProc = m_impl->iirProc;

    if((m_impl->kind == kHighPass) && isFirst)
    {
        auto &diffProc = m_impl->diffProc;
        
        // (Optional) Diff filter for high pass filter:
        diffProc.useTexture(id, useTexUnit, target, 0);
        diffProc.useTexture2(iirProc.getOutputTexId(), iirProc.getTextureUnit(), GL_TEXTURE_2D);
    }
    
    // IIR filter input (same image for first frame)

    if(isFirst)
    {
        isFirst = false;
        iirProc.useTexture(id, useTexUnit, target, 0);
        iirProc.useTexture2(id, useTexUnit, GL_TEXTURE_2D);
        
        // FIFO input (from IIR)
        fifoProc.useTexture(iirProc.getOutputTexId(), iirProc.getTextureUnit(), GL_TEXTURE_2D, 0);
    }
    else
    {
        // Connect FIFO output to second input of IIR for frames >= 1
        iirProc.useTexture2(fifoProc.getOutputTexId(), fifoProc.getTextureUnit(), GL_TEXTURE_2D);
    }
}

END_OGLES_GPGPU
