//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

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
    
    /**
     * Try to enable platform optimizations. Returns true on success, else false.
     */
    static bool tryEnablePlatformOptimizations();
    
private:
    static bool usePlatformOptimizations;   // is true if tryEnablePlatformOptimizations() was called and succeeded
};
    
}

#endif