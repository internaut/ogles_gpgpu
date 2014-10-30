#include "core.h"

using namespace std;
using namespace ogles_gpgpu;

Core::Core() {
    initialized = false;
    inputFrameW = inputFrameH = 0;
    outputTexId = 0;
    firstProc = lastProc = NULL;
}

void Core::addProcToPipeline(ProcBase *proc) {
    if (initialized) {
        cerr << "ogles_gpgpu::Core - adding processor failed: pipeline already initialized" << endl;
    }
    
    cout << "ogles_gpgpu::Core - adding processor #" << (pipeline.size() + 1) << " to pipeline" << endl;
    
    pipeline.push_back(proc);
}

void Core::init(int inW, int inH, bool genInputTexId) {
    assert(!initialized && inW > 0 && inH > 0 && pipeline.size() > 0);
    inputFrameW = inW;
    inputFrameH = inH;
    
    // generate input texture id if necessary
    if (genInputTexId) {
        glGenTextures(1, &inputTexId);
    } else {
        inputTexId = 0;
    }
    
    // initialize the pipeline
    ProcBase *prevProc = NULL;
    for (list<ProcBase *>::iterator it = pipeline.begin();
         it != pipeline.end();
         ++it)
    {
        bool isFirstProc = (it == pipeline.begin());
        
        // find out the input frame size for the proc
        int pipelineFrameW, pipelineFrameH;
        
        if (isFirstProc) {
            firstProc = *it;
            
            pipelineFrameW = inputFrameW;
            pipelineFrameH = inputFrameH;
        } else {
            pipelineFrameW = prevProc->getOutFrameW();
            pipelineFrameH = prevProc->getOutFrameH();
        }
        
        // initialize current proc
        (*it)->init(pipelineFrameW, pipelineFrameH);
        
        // set input texture id
        if (!isFirstProc) {
            (*it)->useTexture(prevProc->getOutputTexId());  // chain together
        }
        
        // set pointer to previous proc
        prevProc = *it;
    }
    
    lastProc = prevProc;

    // set output texture id
    outputTexId = lastProc->getOutputTexId();
    
    initialized = true;
}

void Core::setInputData(const unsigned char *data) {
    assert(initialized && inputTexId > 0);
    
	// set texture
	glBindTexture(GL_TEXTURE_2D, inputTexId);	// bind input texture
    
	// set clamping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    // copy data as texture to GPU
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, inputFrameW, inputFrameH, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	// generate mipmap
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Core::process() {
    assert(initialized);
    
    // set input texture id
    firstProc->useTexture(inputTexId);
    
    // set output texture id
    outputTexId = lastProc->getOutputTexId();
    
    // run the processors in the pipeline
    for (list<ProcBase *>::iterator it = pipeline.begin();
         it != pipeline.end();
         ++it)
    {
        (*it)->render();
    }
}

void Core::getOutputData(unsigned char *buf) {
    assert(initialized);
    
    lastProc->getResultData(buf);
}