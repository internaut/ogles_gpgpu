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

bool MemTransferFactory::usePlatformOptimizations = false;

MemTransfer *MemTransferFactory::createInstance() {
    MemTransfer *instance = NULL;
    
    if (usePlatformOptimizations) {   // create specialized instance
#ifdef __APPLE__
        instance = (MemTransfer *)new MemTransferIOS();
#endif
    }
    
    if (!instance) {    // create default instance
        instance = new MemTransfer();
    }
    
    return instance;
}

bool MemTransferFactory::tryEnablePlatformOptimizations() {
#ifdef __APPLE__
    return MemTransferIOS::initPlatformOptimizations();
#else
    return false;
#endif
}