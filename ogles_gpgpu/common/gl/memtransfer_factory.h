/**
 * MemTransferFactory creates MemTransfer instances according to
 * the platform it was compiled for.
 */

#ifndef OGLES_GPGPU_COMMON_GL_MEMTRANSFER_FACTORY
#define OGLES_GPGPU_COMMON_GL_MEMTRANSFER_FACTORY

#include "../common_includes.h"
#include "memtransfer.h"

namespace ogles_gpgpu {

/**
 * MemTransferFactory creates MemTransfer instances according to
 * the platform it was compiled for.
 */
class MemTransferFactory {
public:
    /**
     * Create a new MemTransfer instance.
     */
    static MemTransfer *createInstance();
};
    
}

#endif