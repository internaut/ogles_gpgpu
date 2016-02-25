#include "procinterface.h"

using namespace ogles_gpgpu;

// ########## Filter chain
    
void ProcInterface::add(ProcInterface *filter) {
    subscribers.push_back(filter);
}
    
void ProcInterface::process(GLuint id, GLuint useTexUnit, GLenum target, int index, Logger logger) {
    
    if(logger) logger(std::string(getProcName()) + " begin");
    if(index == 0) {
        // set input texture id
        useTexture(id, useTexUnit, target);
    }
        
    render();
    glFinish();
    if(logger) logger(std::string(getProcName()) + " end");
        
    for(auto &subscriber : subscribers) {
        subscriber->process(id, useTexUnit, target, index + 1, logger);
    }
}
    
void ProcInterface::prepare(int inW, int inH, GLenum inFmt, int index) {
    if(index == 0) {
        setExternalInputDataFormat(inFmt);
    }
    
    if((getInFrameW() == 0) && (getInFrameH() == 0)) {
        init(inW, inH, index, (index == 0) && (inFmt != GL_NONE));
    }
    else {
        reinit(inW, inH, (index == 0) && (inFmt != GL_NONE));
    }
    
    bool willDownScale = false;
    if(subscribers.size() != 0) {
        for(auto &subscriber : subscribers) {
            willDownScale |= subscriber->getWillDownscale();
        }
    }
        
    createFBOTex(useMipmaps && willDownScale); // last one is false
        
    for(auto &subscriber : subscribers) {
        subscriber->prepare(getOutFrameW(), getOutFrameH(), inFmt, index+1);
        subscriber->useTexture(getOutputTexId());
    }
}
    
