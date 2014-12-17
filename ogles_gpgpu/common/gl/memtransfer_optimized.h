//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * MemTransferOptimized specifies a common interface for platform-optimized
 * MemTransfer specializations.
 */

#ifndef OGLES_GPGPU_COMMON_GL_MEMTRANSFER_OPTIMIZED
#define OGLES_GPGPU_COMMON_GL_MEMTRANSFER_OPTIMIZED

#include "memtransfer.h"

namespace ogles_gpgpu {

/**
 * Buffer type (input/output) definition.
 */
typedef enum {
    BUF_TYPE_INPUT = 0,
    BUF_TYPE_OUTPUT
} BufType;

/**
 * MemTransferOptimized specifies a common interface for platform-optimized
 * MemTransfer specializations. These can provide more functions than the
 * MemTransfer base class.
 */
class MemTransferOptimized {
public:
    /**
     * Lock the input or output buffer and return its base address.
     * The input buffer will be locked for reading AND writing, while the
     * output buffer will be locked for reading only.
     */
    virtual void *lockBufferAndGetPtr(BufType bufType) = 0;

    /**
     * Unlock the input or output buffer.
     */
    virtual void unlockBuffer(BufType bufType) = 0;
};

}
#endif
