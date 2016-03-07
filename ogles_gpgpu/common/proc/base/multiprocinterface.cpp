#include "multiprocinterface.h"

BEGIN_OGLES_GPGPU

// ######### MultiProcInterface

void MultiProcInterface::setOutputRenderOrientation(RenderOrientation o) { getOutputFilter()->setOutputRenderOrientation(o); }
RenderOrientation MultiProcInterface::getOutputRenderOrientation() const { return getOutputFilter()->getOutputRenderOrientation(); }     

void MultiProcInterface::setExternalInputDataFormat(GLenum fmt) { return getInputFilter()->setExternalInputDataFormat(fmt); }
void MultiProcInterface::setExternalInputData(const unsigned char *data) { return getInputFilter()->setExternalInputData(data); }
GLuint MultiProcInterface::getTextureUnit() const { return getInputFilter()->getTextureUnit(); }
void MultiProcInterface::setOutputSize(float scaleFactor) { getInputFilter()->setOutputSize(scaleFactor); }
void MultiProcInterface::setOutputSize(int outW, int outH) { return getInputFilter()->setOutputSize(outW, outH); }
int MultiProcInterface::getOutFrameW() const { return getOutputFilter()->getOutFrameW(); }
int MultiProcInterface::getOutFrameH() const { return getOutputFilter()->getOutFrameH(); }
int MultiProcInterface::getInFrameW() const { return getInputFilter()->getInFrameW(); }
int MultiProcInterface::getInFrameH() const { return getInputFilter()->getInFrameH(); }
bool MultiProcInterface::getWillDownscale() const { return getInputFilter()->getWillDownscale(); }
void MultiProcInterface::getResultData(unsigned char *data) const { getOutputFilter()->getResultData(data); }
void MultiProcInterface::getResultData(FrameDelegate &delegate) const { getOutputFilter()->getResultData(delegate); }
MemTransfer *MultiProcInterface::getMemTransferObj() const { return getOutputFilter()->getMemTransferObj(); }
MemTransfer *MultiProcInterface::getInputMemTransferObj() const { return getInputFilter()->getMemTransferObj(); }
GLuint MultiProcInterface::getInputTexId() const { return getInputFilter()->getInputTexId(); }
GLuint MultiProcInterface::getOutputTexId() const { return getOutputFilter()->getOutputTexId(); }
void MultiProcInterface::printInfo() {
    OG_LOGINF(getProcName(), "begin info for %u passes", (unsigned int)size());
    for(int i = 0; i < size(); i++) {
        (*this)[i]->printInfo();
    }
    OG_LOGINF(getProcName(), "end info");
}

END_OGLES_GPGPU
