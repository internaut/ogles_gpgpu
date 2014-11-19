#include "memtransfer_factory.h"

#ifdef __APPLE__
#include "../../platform/ios/memtransfer_ios.h"
#endif

using namespace ogles_gpgpu;

MemTransfer *MemTransferFactory::createInstance() {
    MemTransfer *instance;
    
#ifdef __APPLE__
    instance = (MemTransfer *)new MemTransferIOS();
#else
    instance = new MemTransfer();
#endif
    
    return instance;
}