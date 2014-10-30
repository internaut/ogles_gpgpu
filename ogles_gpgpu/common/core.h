#ifndef OGLES_GPGPU_COMMON_CORE
#define OGLES_GPGPU_COMMON_CORE

#include "common_includes.h"
#include "proc/procbase.h"

#include <list>

using namespace std;

namespace ogles_gpgpu {

class Core {
public:
    Core();
    
    /**
     * Add a pointer to a GPGPU processor object to the pipeline.
     * Note: OpenGL context must be initialized before a ProcBase object
     * was created!
     */
    void addProcToPipeline(ProcBase *proc);
    
    /**
     * Initialize the processing pipeline.
     * Note OpenGL context must be initialized before the pipeline was
     * defined!
     */
    void init(int inW, int inH, bool genInputTexId);
    
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
    
private:
    list<ProcBase *> pipeline;  // contains weak refs to ProcBase objects
    
    ProcBase *firstProc;
    ProcBase *lastProc;
    
    bool initialized;
    
    int inputFrameW;
    int inputFrameH;
    int outputFrameW;
    int outputFrameH;

    GLuint inputTexId;
    GLuint outputTexId;
};
    
}

#endif