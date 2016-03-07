//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "multipassproc.h"

using namespace ogles_gpgpu;

ProcInterface* MultiPassProc::getInputFilter() const { return procPasses.front(); }
ProcInterface* MultiPassProc::getOutputFilter() const { return procPasses.back(); }
ProcInterface * MultiPassProc::operator[](int i) const { return procPasses[i]; }
size_t MultiPassProc::size() const { return procPasses.size(); }

#pragma mark constructor/deconstructor

MultiPassProc::~MultiPassProc() {
    // remove all pass instances
    for(auto &it : procPasses) {
        delete it;
    }

    procPasses.clear();
}

#pragma mark ProcInterface methods

int MultiPassProc::init(int inW, int inH, unsigned int order, bool prepareForExternalInput) {
    ProcInterface *prevProc = NULL;
    int num = 0;
    int numInitialized = 0;
    
    for(auto &it : procPasses) {

        // find out the input frame size for the proc
        int pipelineFrameW, pipelineFrameH;

        if (num == 0) {
            // first pipeline's frame size is the input frame size
            pipelineFrameW = inW;
            pipelineFrameH = inH;
        } else {
            // subsequent pipeline's frame size is the previous processor's output frame size
            pipelineFrameW = prevProc->getOutFrameW();
            pipelineFrameH = prevProc->getOutFrameH();
        }

        numInitialized = it->init(pipelineFrameW, pipelineFrameH, num, num == 0 ? prepareForExternalInput : false);

        // set pointer to previous proc
        prevProc = it;

        num += numInitialized;
    }

    return num;
}

int MultiPassProc::reinit(int inW, int inH, bool prepareForExternalInput) {
    ProcInterface *prevProc = NULL;
    int num = 0;
    int numInitialized = 0;
    
    for(auto &it : procPasses) {

        // find out the input frame size for the proc
        int pipelineFrameW, pipelineFrameH;

        if (num == 0) {
            // first pipeline's frame size is the input frame size
            pipelineFrameW = inW;
            pipelineFrameH = inH;
        } else {
            // subsequent pipeline's frame size is the previous processor's output frame size
            pipelineFrameW = prevProc->getOutFrameW();
            pipelineFrameH = prevProc->getOutFrameH();
        }

        numInitialized = it->reinit(pipelineFrameW, pipelineFrameH, num == 0 ? prepareForExternalInput : false);

        // set pointer to previous proc
        prevProc = it;

        num += numInitialized;
    }

    return num;
}

void MultiPassProc::cleanup() {
    for(auto &it : procPasses) {
        it->cleanup();
    }
}

void MultiPassProc::createFBOTex(bool genMipmap) {
    bool first = true;
    for(auto &it : procPasses) {
        it->createFBOTex(first ? genMipmap : false);
        first = false;
    }
}

void MultiPassProc::render(int position) {
    for(auto &it : procPasses) {
        it->render(position);
    }
}

void MultiPassProc::useTexture(GLuint id, GLuint useTexUnit, GLenum target, int position) {
    ProcInterface *prevProc = NULL;

    assert(position == 0); // for now no multi-texture multi-pass filters are supported
    
    for(auto &it : procPasses) {
        if (!prevProc) {    // means this is the first proc pass
            it->useTexture(id, useTexUnit, target);
        } else {            // all other passes
            it->useTexture(prevProc->getOutputTexId(), prevProc->getTextureUnit(), prevProc->getTextureTarget());
        }
        prevProc = it;
    }
}

bool MultiPassProc::getWillDownscale() const {
    for (auto &it : procPasses) {
        if(it->getWillDownscale()) {
            return true;
        }
    }
    return false;
}
