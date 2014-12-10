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