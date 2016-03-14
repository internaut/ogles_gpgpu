#include "procinterface.h"

using namespace ogles_gpgpu;

// ########## Filter chain

void ProcInterface::setActive(bool state) {
    active = state;
}

void ProcInterface::add(ProcInterface *filter, int position) {
    subscribers.emplace_back(filter, position);
}

// Top level recursive filter chain processing, set input texture for first filter as needed
void ProcInterface::process(GLuint id, GLuint useTexUnit, GLenum target, int index, int position, Logger logger) {
    
    if(!active) return;
    
    if(logger) logger(std::string(getProcName()) + " begin");
    if(index == 0)
    {
        // set input texture id
        useTexture(id, useTexUnit, target, position);
        Tools::checkGLErr(getProcName(), "useTexture");
    }
        
    render(position);
    //glFinish();
    if(logger) logger(std::string(getProcName()) + " end");
        
    for(auto &subscriber : subscribers) {
        subscriber.first->process(subscriber.second, logger);
    }
}

// Recursive helper method for process() where index >= 1
void ProcInterface::process(int position, Logger logger) {
    
    if(logger) logger(std::string(getProcName()) + " begin");
    int result = render(position);
    if(logger) logger(std::string(getProcName()) + " end");
    
    if(result == 0) {
        // Only trigger subscribers 1x 
        for(auto &subscriber : subscribers) {
            // Update: FIFO and other filters may change the output texture id on each step:
            subscriber.first->useTexture(getOutputTexId(), getTextureUnit(), GL_TEXTURE_2D, subscriber.second);
            subscriber.first->process(subscriber.second, logger);
        }
    }
}

#define DO_MIPMAP_TEST 0

// Top level filter chain preparation, set input format for first filter
void ProcInterface::prepare(int inW, int inH, GLenum inFmt, int index, int position) {
    if(index == 0) {
        setExternalInputDataFormat(inFmt);
    }
    
    // In case of multi-input textures, only (re)init for the first one
    if(position == 0) {
        if((getInFrameW() == 0) && (getInFrameH() == 0)) {
            init(inW, inH, index, (index == 0) && (inFmt != GL_NONE));
        }
        else {
            reinit(inW, inH, (index == 0) && (inFmt != GL_NONE));
        }
        
        bool willDownScale = false;
        
#if DO_MIPMAP_TEST
        if(subscribers.size() != 0) {
            for(auto &subscriber : subscribers) {
                willDownScale |= subscriber.first->getWillDownscale();
            }
        }
#endif
        
        // Create FBO for out single output texture
        createFBOTex(useMipmaps && willDownScale); // last one is false
        
        for(auto &subscriber : subscribers) {
            subscriber.first->prepare(getOutFrameW(), getOutFrameH(), index+1, subscriber.second);
            subscriber.first->useTexture(getOutputTexId(), getTextureUnit(), GL_TEXTURE_2D, subscriber.second);
        }
    }
}

// Recursive helper method for prepare() where index >= 1
void ProcInterface::prepare(int inW, int inH, int index, int position) {

    if(position == 0) {
        if((getInFrameW() == 0) && (getInFrameH() == 0)) {
            init(inW, inH, index, false);
        }
        else {
            reinit(inW, inH, false);
        }
        
        bool willDownScale = false;
        
#if DO_MIPMAP_TEST
        if(subscribers.size() != 0) {
            for(auto &subscriber : subscribers) {
                willDownScale |= subscriber.first->getWillDownscale();
            }
        }
#endif
        
        // Create FBO for out single output texture
        createFBOTex(useMipmaps && willDownScale); // last one is false
        
        for(auto &subscriber : subscribers) {
            subscriber.first->prepare(getOutFrameW(), getOutFrameH(), index+1, subscriber.second);
            subscriber.first->useTexture(getOutputTexId(), getTextureUnit(), GL_TEXTURE_2D, subscriber.second);
        }
    }
}
