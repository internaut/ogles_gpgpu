#ifndef OGLES_GPGPU_COMMON_CORE
#define OGLES_GPGPU_COMMON_CORE

#include "common_includes.h"
#include "proc/procbase.h"
#include "gl/memtransfer.h"

#include <list>
#include <vector>

using namespace std;

namespace ogles_gpgpu {

class ProcBase;
    
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
     * Deconstructor.
     */
    ~Core();
    
    /**
     * Add a pointer to a GPGPU processor object to the pipeline.
     * Note: OpenGL context must be initialized before a ProcBase object
     * was created!
     */
    void addProcToPipeline(ProcBase *proc);
    
    /**
     * Initialize
     * Note OpenGL context must be initialized before the pipeline was
     * defined!
     */
    void init(void *glContext = NULL);
    
    /**
     * Prepare the processing pipeline for incoming frames of size <inW> x <inH>.
     * Note OpenGL context must be initialized before the pipeline was
     * defined!
     * Note that init() must have been called before.
     */
    void prepare(int inW, int inH);
    
    void setUseMipmaps(bool use) { useMipmaps = use; }
    bool getUseMipmaps() const { return useMipmaps; }
    
    /**
     * Set input as OpenGL texture id.
     */
    void setInputTexId(GLuint inTexId) { inputTexId = inTexId; }
    
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
    GLuint getOutputTexId() const { return outputTexId; }
    
    /**
     * Get output as bytes. Will copy the output texture from the GPU to <buf>.
     */
    void getOutputData(unsigned char *buf);
    
    int getOutputFrameW() const { return outputFrameW; }
    int getOutputFrameH() const { return outputFrameH; }
    
    void *getGLContextPtr() const { return glContextPtr; }
    
#ifdef OGLES_GPGPU_BENCHMARK
    vector<float> getTimeMeasurements() const {  return Tools::getTimeMeasurements(); }
#endif
    
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
    
    
    static Core *instance;  // singleton instance
    
    void *glContextPtr;
    
    list<ProcBase *> pipeline;  // contains weak refs to ProcBase objects
    
    ProcBase *firstProc;
    ProcBase *lastProc;
    
    bool initialized;
    bool prepared;
    
    bool useMipmaps;
    bool glExtNPOTMipmaps;
    
    bool inputSizeIsPOT;
    
    int inputFrameW;
    int inputFrameH;
    int outputFrameW;
    int outputFrameH;

    GLuint inputTexId;
    GLuint outputTexId;
};
    
}

#endif