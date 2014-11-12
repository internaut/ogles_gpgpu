#include "core.h"

using namespace std;
using namespace ogles_gpgpu;

Core::Core() {
    initialized = false;
    inputFrameW = inputFrameH = 0;
    outputFrameW = outputFrameH = 0;
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
    
    cout << "ogles_gpgpu::Core - init - supported OpenGL extensions:" << endl;
    cout << glGetString(GL_EXTENSIONS) << endl << endl;
    
    // init opengl
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE1);
    
    Tools::checkGLErr("ogles_gpgpu::Core - init");
    
    // generate input texture id if necessary
    if (genInputTexId) {
        glGenTextures(1, &inputTexId);
    } else {
        inputTexId = 0;
    }
    
    // initialize the pipeline
    ProcBase *prevProc = NULL;
    unsigned int num = 0;
    for (list<ProcBase *>::iterator it = pipeline.begin();
         it != pipeline.end();
         ++it)
    {
        // find out the input frame size for the proc
        int pipelineFrameW, pipelineFrameH;
        
        if (num == 0) {
            firstProc = *it;
            
            pipelineFrameW = inputFrameW;
            pipelineFrameH = inputFrameH;
        } else {
            pipelineFrameW = prevProc->getOutFrameW();
            pipelineFrameH = prevProc->getOutFrameH();
        }
        
        // set input texture id
        if (num > 0) {
            (*it)->useTexture(prevProc->getOutputTexId());  // chain together
        }
        
        // initialize current proc
        (*it)->init(pipelineFrameW, pipelineFrameH, num);
        
        // set pointer to previous proc
        prevProc = *it;
        
        num++;
    }
    
    lastProc = prevProc;

    // set output texture id and size
    outputTexId = lastProc->getOutputTexId();
    outputFrameW = lastProc->getOutFrameW();
    outputFrameH = lastProc->getOutFrameH();
    
    initialized = true;
}

void Core::setInputData(const unsigned char *data) {
    assert(initialized && inputTexId > 0);
    
	// set texture
    glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, inputTexId);	// bind input texture

	// set clamping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // copy data as texture to GPU
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, inputFrameW, inputFrameH, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    // mipmapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glGenerateMipmap(GL_TEXTURE_2D);
    
    Tools::checkGLErr("ogles_gpgpu::Core - setInputData");
    
    glFinish();
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
        
        glFinish();
    }
}

void Core::getOutputData(unsigned char *buf) {
    assert(initialized);
    
    lastProc->getResultData(buf);
}