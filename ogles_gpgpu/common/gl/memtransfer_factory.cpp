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

//#ifdef __APPLE__
//#include "../../platform/ios/memtransfer_ios.h"
//#elif __ANDROID__
//#include "../../platform/android/memtransfer_android.h"
//#endif

#ifdef OGLES_GPGPU_IOS
#  include "../../platform/ios/memtransfer_ios.h"
#elif OGLES_GPGPU_OSX
#  include "../../platform/osx/memtransfer_osx.h"
#elif OGLES_GPGPU_ANDROID
#  include "../../platform/android/memtransfer_android.h"
#else
#  include "../../platform/opengl/memtransfer_generic.h"
#endif

using namespace ogles_gpgpu;

bool MemTransferFactory::usePlatformOptimizations = false;

MemTransfer *MemTransferFactory::createInstance() {
    MemTransfer *instance = NULL;
    
    if (usePlatformOptimizations) {   // create specialized instance
#ifdef OGLES_GPGPU_IOS
        instance = (MemTransfer *)new MemTransferIOS();
#elif OGLES_GPGPU_OSX
        instance = (MemTransfer *)new MemTransferOSX();
#elif OGLES_GPGPU_ANDROID
        instance = (MemTransfer *)new MemTransferAndroid();
#else
        instance = (MemTransfer *)new MemTransfer();
#endif
    }
    
    if (!instance) {    // create default instance
        instance = new MemTransfer();
    }
    
    return instance;
}

bool MemTransferFactory::tryEnablePlatformOptimizations() {
#ifdef OGLES_GPGPU_IOS
    usePlatformOptimizations = MemTransferIOS::initPlatformOptimizations();
#elif OGLES_GPGPU_OSX
    usePlatformOptimizations = MemTransferOSX::initPlatformOptimizations();
#elif OGLES_GPGPU_ANDROID
    usePlatformOptimizations = MemTransferAndroid::initPlatformOptimizations();
#else
    usePlatformOptimizations = false;
#endif
    
    return usePlatformOptimizations;
}
