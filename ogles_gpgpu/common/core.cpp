#include "core.h"

#include "disp.h"

#include <string>
#include <algorithm>

using namespace std;
using namespace ogles_gpgpu;

#pragma mark singleton stuff

// initialize static variables

Core *Core::instance = NULL;                // no instance
bool Core::usePlatformOptimizations = true; // turn on platform optimizations

Core *Core::getInstance() {
    if (!Core::instance) {
        Core::instance = new Core();
    }
    
    return Core::instance;
}

void Core::destroy() {
    if (Core::instance) {
        delete Core::instance;
        Core::instance = NULL;
    }
}

#pragma mark constructor and setup methods

Core::Core() {
    // set defaults
    initialized = false;
    useMipmaps = false;
    glExtNPOTMipmaps = false;
    renderDisp = NULL;
    glContextPtr = NULL;
    
    // reset to defaults
    reset();
}

Core::~Core() {
    cleanup();
}

void Core::reset() {
    cleanup();
    
    // call cleanup() on processors
    for (list<ProcInterface *>::iterator it = pipeline.begin();
         it != pipeline.end();
         ++it)
    {
        (*it)->cleanup();
    }
    
    pipeline.clear();
    
    // reset defaults
    prepared = false;
    
    inputSizeIsPOT = false;
    inputFrameW = inputFrameH = 0;
    outputFrameW = outputFrameH = 0;
    inputTexId = outputTexId = 0;
    firstProc = lastProc = NULL;
}

void Core::addProcToPipeline(ProcInterface *proc) {
    // pipeline needs to be set up before calling init()
    if (initialized) {
        cerr << "ogles_gpgpu::Core - adding processor failed: pipeline already initialized" << endl;
    }
    
    cout << "ogles_gpgpu::Core - adding processor #" << (pipeline.size() + 1) << " to pipeline" << endl;
    
    // add not processor to pipeline
    pipeline.push_back(proc);
}

Disp *Core::createRenderDisplay(int dispW, int dispH, RenderOrientation orientation) {
    assert(!renderDisp);
    
    renderDisp = new Disp();
    renderDisp->setOutputRenderOrientation(orientation);
    
    if (dispW > 0 && dispH > 0) {
        renderDisp->setOutputSize(dispW, dispH);
    }
    
    return renderDisp;
}

void Core::init(void *glContext) {
    assert(!initialized);
    
    checkGLExtensions();
    
    // set OpenGL context pointer
    glContextPtr = glContext;
    
    // init opengl
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE1);
    
    Tools::checkGLErr("ogles_gpgpu::Core - init");
    
    initialized = true;
}

void Core::prepare(int inW, int inH, GLenum inFmt) {
    assert(initialized && inW > 0 && inH > 0 && pipeline.size() > 0);
    
    if (prepared && inputFrameW == inW && inputFrameH == inH) return;   // no change
    
    // set input frame size
    inputSizeIsPOT = Tools::isPOT(inW) && Tools::isPOT(inH);
    inputFrameW = inW;
    inputFrameH = inH;

    cout << "ogles_gpgpu::Core - prepare with input frame size "
         << inputFrameW << "x" << inputFrameH
         << " (POT: " <<  inputSizeIsPOT << ")" << endl;

    // initialize the pipeline
    ProcInterface *prevProc = NULL;
    unsigned int num = 0;
    int numInitialized = 0;
    for (list<ProcInterface *>::iterator it = pipeline.begin();
         it != pipeline.end();
         ++it)
    {
        // find out the input frame size for the proc
        int pipelineFrameW, pipelineFrameH;
        
        if (num == 0) { // special set up for first pipeline processor
            firstProc = *it;
            
            // first pipeline processor will get input data (e.g. RGBA pixel data)
            firstProc->setExternalInputDataFormat(inFmt);
            
            // first pipeline's frame size is the input frame size
            pipelineFrameW = inputFrameW;
            pipelineFrameH = inputFrameH;
        } else {
            // subsequent pipeline's frame size is the previous processor's output frame size
            pipelineFrameW = prevProc->getOutFrameW();
            pipelineFrameH = prevProc->getOutFrameH();
        }
        
        if (!prepared) {    // for first time preparation
            // initialize current proc
            numInitialized = (*it)->init(pipelineFrameW, pipelineFrameH, num, num == 0 && inFmt != GL_NONE);
        } else {    // for reinitialization with different frame size
            numInitialized = (*it)->reinit(pipelineFrameW, pipelineFrameH, num == 0 && inFmt != GL_NONE);
        }
        
        // if this proc will downscale, we should generate a mipmap for the previous output
        if (num > 0) {
            // create a texture that is attached to an FBO for the output
            prevProc->createFBOTex(useMipmaps && (*it)->getWillDownscale());
        }
        
        // set pointer to previous proc
        prevProc = *it;
        
        num += numInitialized;
    }
    
    // create the FBO texture for the last processor, too
    prevProc->createFBOTex(false);
    
    // concatenate all processors
    prevProc = NULL;
    for (list<ProcInterface *>::iterator it = pipeline.begin();
         it != pipeline.end();
         ++it)
    {
        if (prevProc) {
            // set input texture id
            (*it)->useTexture(prevProc->getOutputTexId());  // previous output is this proc's input
        }
        
        // set pointer to previous proc
        prevProc = *it;
    }
    
    // set last processor
    lastProc = prevProc;
    
    // get input texture id
    inputTexId = firstProc->getInputTexId();
    
    // set output texture id and size
    outputTexId = lastProc->getOutputTexId();
    outputFrameW = lastProc->getOutFrameW();
    outputFrameH = lastProc->getOutFrameH();
    
    // initialize render display if necessary
    if (renderDisp) {
        if (!prepared) {
            renderDisp->init(outputFrameW, outputFrameH, num);
        } else {
            renderDisp->reinit(outputFrameW, outputFrameH);
        }
        
        renderDisp->useTexture(outputTexId);
    }
    
    cout << "ogles_gpgpu::Core - prepared (input tex "
         << inputTexId << " / output tex " << outputTexId << ")" << endl;
    
    // print report (to spot errors in the pipeline)
    for (list<ProcInterface *>::iterator it = pipeline.begin();
         it != pipeline.end();
         ++it)
    {
        (*it)->printInfo();
    }

    prepared = true;
}

#pragma mark input, processing and output methods

MemTransfer *Core::getInputMemTransfer() const {
    return firstProc->getMemTransferObj();
}

MemTransfer *Core::getOutputMemTransfer() const {
    return lastProc->getMemTransferObj();
}

void Core::setInputTexId(GLuint inTexId) {
    inputTexId = inTexId;
    firstProc->useTexture(inputTexId);
}

void Core::setInputData(const unsigned char *data) {
    assert(initialized && inputTexId > 0);
    
#ifdef OGLES_GPGPU_BENCHMARK
    Tools::resetTimeMeasurement();
    
    Tools::startTimeMeasurement();
#endif
    
    // check set up and input data
    if (useMipmaps && !inputSizeIsPOT && !glExtNPOTMipmaps) {
        cout << "ogles_gpgpu::Core - setInputData - WARNING: NPOT input image provided but NPOT mipmapping not supported!" << endl
             << "ogles_gpgpu::Core - setInputData - mipmapping disabled!" << endl;
        useMipmaps = false;
    }
    
	// set texture
    glActiveTexture(GL_TEXTURE1);
    
    // copy data as texture to GPU
    firstProc->setExternalInputData(data);
    
    // mipmapping
    if (firstProc->getWillDownscale() && useMipmaps) {
        cout << "ogles_gpgpu::Core - setInputData - generating mipmap for input image" << endl;
        // enabled
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        // disabled
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
	
    Tools::checkGLErr("ogles_gpgpu::Core - setInputData");
    
    glFinish();
    
#ifdef OGLES_GPGPU_BENCHMARK
    Tools::stopTimeMeasurement();
#endif
}

void Core::process() {
    assert(initialized);
    
#ifdef OGLES_GPGPU_BENCHMARK
    Tools::startTimeMeasurement();
#endif
    
    // set input texture id
    firstProc->useTexture(inputTexId);
    
    // set output texture id
    outputTexId = lastProc->getOutputTexId();
    
    // run the processors in the pipeline
    for (list<ProcInterface *>::iterator it = pipeline.begin();
         it != pipeline.end();
         ++it)
    {
        (*it)->render();
        
        glFinish();
    }
    
#ifdef OGLES_GPGPU_BENCHMARK
    Tools::stopTimeMeasurement();
#endif
}

void Core::getOutputData(unsigned char *buf) {
    assert(initialized);
    
#ifdef OGLES_GPGPU_BENCHMARK
    Tools::startTimeMeasurement();
#endif
    
    // will copy the result data from the GPU's memory space to <buf>
    lastProc->getResultData(buf);
    
#ifdef OGLES_GPGPU_BENCHMARK
    Tools::stopTimeMeasurement();
#endif
}

#pragma mark helper methods

void Core::checkGLExtensions() {
    // get string with extensions seperated by a SPACE
    string glExtString((const char *)glGetString(GL_EXTENSIONS));
    
    // get extensions as vector
    vector<string> glExt = Tools::split(glExtString);
    
    // check extensions
//    cout << "ogles_gpgpu::Core - checkGLExtensions - list of extensions:" << endl;
    for (vector<string>::iterator it = glExt.begin();
         it != glExt.end();
         ++it)
    {
        // get lowercase extension string
        string extName = *it;
        transform(extName.begin(), extName.end(), extName.begin(), ::tolower);
        
//        cout << "> " << extName << endl;
        
        // check for NPOT mipmapping support
        if (it->compare("gl_arb_texture_non_power_of_two") == 0
         || it->compare("gl_oes_texture_npot") == 0)
        {
            glExtNPOTMipmaps = true;
        }
    }
    
    cout << "ogles_gpgpu::Core - checkGLExtensions - NPOT mipmaps: " << glExtNPOTMipmaps << endl;
}

void Core::cleanup() {
    if (renderDisp) {
        delete renderDisp;
        renderDisp = NULL;
    }
}