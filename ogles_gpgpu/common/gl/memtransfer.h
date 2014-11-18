#ifndef OGLES_GPGPU_COMMON_GL_MEMTRANSFER
#define OGLES_GPGPU_COMMON_GL_MEMTRANSFER

#include "../common_includes.h"

namespace ogles_gpgpu {

class MemTransfer {
public:
    /**
     * Get singleton instance.
     */
    static MemTransfer *getInstance();
    
    /**
     * Destroy singleton instance.
     */
    static void destroy();
    
    /**
     * Map data in <buf> to GPU.
     */
    virtual void toGPU(GLuint texId, int texW, int texH, const unsigned char *buf);
    
    /**
     * Map data from GPU to <buf>
     */
    virtual void fromGPU(GLuint texId, int texW, int texH, unsigned char *buf);
    
protected:
    /**
     * Private constructor for singleton instance.
     */
    MemTransfer() {}
    
    /**
     * Empty copy constructor.
     */
    MemTransfer (const MemTransfer&) {}
    
private:
    static MemTransfer *instance;  // singleton instance
};

}
    
#endif