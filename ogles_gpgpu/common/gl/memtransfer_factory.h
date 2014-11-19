#ifndef OGLES_GPGPU_COMMON_GL_MEMTRANSFER_FACTORY
#define OGLES_GPGPU_COMMON_GL_MEMTRANSFER_FACTORY

#include "../common_includes.h"
#include "memtransfer.h"

namespace ogles_gpgpu {

class MemTransferFactory {
public:
    static MemTransfer *createInstance();
};
    
}

#endif