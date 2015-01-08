//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * ogles_gpgpu main processing handler
 */

#ifndef OGLES_GPGPU_COMMON_CORE
#define OGLES_GPGPU_COMMON_CORE

#include "common_includes.h"
#include "proc/base/procinterface.h"
#include "gl/memtransfer.h"

#include <list>
#include <vector>

using namespace std;

namespace ogles_gpgpu {

class Disp;

/**
 * main processing handler. set up and initialize processing pipeline.
 * set processing input, run the processing tasks, get the processing output.
 * is a singleton class that returns an object via getInstance().
 */
class Core {
public:
    /**
     * Get singleton instance.
     */
    static Core *getInstance();
    
    /**
     * Destroy singleton instance.
     */
    static void destroy();
    
    /**
     * Deconstructor. Will call cleanup().
     */
    ~Core();
    
    /**
     * Reset the complete processing pipeline (will also call cleanup()).
     */
    void reset();
    
    /**
     * Add a weak ref pointer to a GPGPU processor object to the pipeline.
     * Note: OpenGL context must be initialized before a ProcInterface object
     * was created!
     */
    void addProcToPipeline(ProcInterface *proc);
    
    /**
     * Create an object that renders the last processor's output to the screen.
     * Return it as weak ref.
     * Parameters <dispW>, <dispH> and <orientation> set the render display properties.
     * They do not have to be set at this point, you can later use the methods of the
     * returned "Disp" object to adjust these properties.
     * Note: Must be called after last addProcToPipeline() call and before
     * init() / prepare()!
     */
    Disp *createRenderDisplay(int dispW = 0, int dispH = 0, RenderOrientation orientation = RenderOrientationStd);
    
    /**
     * Initialize OpenGL settings and the pipeline.
     * Optionally pass the OpenGL context (possibly needed for platform specific optimizations).
     * Needs to be called after addProcToPipeline().
     * Can be called only once per instance.
     * Note OpenGL context must be initialized before the pipeline was
     * defined!
     */
    void init(void *glContext = NULL);
    
    /**
     * Prepare the processing pipeline for incoming frames of size <inW> x <inH>
     * and pixel format <inFmt>.
     * Can be called several times (will re-initialize the pipeline).
     * Note OpenGL context must be initialized before the pipeline was
     * defined!
     * Note that init() must have been called before.
     */
    void prepare(int inW, int inH, GLenum inFmt = GL_RGBA);
    
    /**
     * Return the render display object as weak ref.
     */
    Disp *getRenderDisplay() const { return renderDisp; }
    
    /**
     * Get pointer to input memory transfer handler
     */
    MemTransfer *getInputMemTransfer() const;
    
    /**
     * Get pointer to output memory transfer handler
     */
    MemTransfer *getOutputMemTransfer() const;
    
    /**
     * Use mipmaps: <use>.
     * Note that some hardware only supports mipmapping for POT images.
     */
    void setUseMipmaps(bool use) { useMipmaps = use; }
    
    /**
     * Get "use mipmaps" status.
     */
    bool getUseMipmaps() const { return useMipmaps; }
    
    /**
     * Set input as OpenGL texture id.
     */
    void setInputTexId(GLuint inTexId, GLenum inTexTarget = GL_TEXTURE_2D);
    
    /**
     * Set input as RGBA byte data of size <w> x <h>.
     */
    void setInputData(const unsigned char *data);
    
    /**
     * Process input data by executing the GPGPU processors defined in
     * the pipeline.
     */
    void process();
    
    /**
     * Get output as OpenGL texture id.
     */
    GLuint getOutputTexId() const { assert(lastProc); return lastProc->getOutputTexId(); }
    
    /**
     * Get output as bytes. Will copy the output texture from the GPU to <buf>.
     */
    void getOutputData(unsigned char *buf);
    
    /**
     * Get output frame width.
     */
    int getOutputFrameW() const { return outputFrameW; }
    
    /**
     * Get output frame height.
     */
    int getOutputFrameH() const { return outputFrameH; }
    
    /**
     * Get pointer to OpenGL context (platform specific type).
     */
    void *getGLContextPtr() const { return glContextPtr; }
    
#ifdef OGLES_GPGPU_BENCHMARK
    vector<double> getTimeMeasurements() const {  return Tools::getTimeMeasurements(); }
#endif
    
    /**
     * Global switch for platform optimizations. Should be set before calling init().
     */
    static bool tryEnablePlatformOptimizations();
    
private:
    /**
     * Private constructor for singleton instance.
     */
    Core();
    
    /**
     * Empty copy constructor.
     */
    Core (const Core&) {}
    
    /**
     * Check which OpenGL extensions are available.
     */
    void checkGLExtensions();
    
    /**
     * Free owned objects.
     * Will clear the processor pipeline. This only calls cleanup() on all processors and
     * deletes the pointers to the processors that are stored in the pipeline. the processor
     * objects are not deleted in this class, because it only stores weak references.
     */
    void cleanup();
    
    
    static Core *instance;  // singleton instance
    
    void *glContextPtr;     // pointer to OpenGL context (platform specific type), weak ref.
    
    list<ProcInterface *> pipeline;  // contains weak refs to ProcBase objects
    
    ProcInterface *firstProc;    // pointer to first processor in pipeline
    ProcInterface *lastProc;     // pointer to last processor in pipeline
    
    Disp *renderDisp;       // render-to-display object. strong ref.
    
    bool initialized;       // pipeline initialized?
    bool prepared;          // input prepared?
    
    bool useMipmaps;        // use mipmaps?
    bool glExtNPOTMipmaps;  // hardware supports NPOT mipmapping?
    
    bool inputSizeIsPOT;    // input frame size is POT?
    
    int inputFrameW;        // input frame width
    int inputFrameH;        // input frame height
    int outputFrameW;       // output frame width
    int outputFrameH;       // output frame width

    GLuint inputTexId;      // input texture id
    GLenum inputTexTarget;  // input texture target
    GLuint outputTexId;     // output texture id
};
    
}

#endif
