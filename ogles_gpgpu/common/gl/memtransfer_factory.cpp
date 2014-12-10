//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "memtransfer_factory.h"
#include "../core.h"

#ifdef __APPLE__
#include "../../platform/ios/memtransfer_ios.h"
#endif

using namespace ogles_gpgpu;

MemTransfer *MemTransferFactory::createInstance() {
    MemTransfer *instance = NULL;
    
    if (Core::usePlatformOptimizations) {   // create specialized instance
#ifdef __APPLE__
#if TARGET_IPHONE_SIMULATOR
#warning ogles_gpgpu platform optimizations are not available in the simulator
        OG_LOGERR("MemTransferFactory", "platform optimizations are not available in the simulator");
#else
        instance = (MemTransfer *)new MemTransferIOS();
#endif
#endif
    }
    
    if (!instance) {    // create default instance
        instance = new MemTransfer();
    }
    
    return instance;
}