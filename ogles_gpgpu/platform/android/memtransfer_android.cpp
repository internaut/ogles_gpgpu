#include "memtransfer_android.h"

#include <dlfcn.h>

using namespace ogles_gpgpu;

// necessary definitions for Android GraphicBuffer

enum {
    /* buffer is never read in software */
    GRALLOC_USAGE_SW_READ_NEVER   = 0x00000000,
    /* buffer is rarely read in software */
    GRALLOC_USAGE_SW_READ_RARELY  = 0x00000002,
    /* buffer is often read in software */
    GRALLOC_USAGE_SW_READ_OFTEN   = 0x00000003,
    /* mask for the software read values */
    GRALLOC_USAGE_SW_READ_MASK    = 0x0000000F,

    /* buffer is never written in software */
    GRALLOC_USAGE_SW_WRITE_NEVER  = 0x00000000,
    /* buffer is never written in software */
    GRALLOC_USAGE_SW_WRITE_RARELY = 0x00000020,
    /* buffer is never written in software */
    GRALLOC_USAGE_SW_WRITE_OFTEN  = 0x00000030,
    /* mask for the software write values */
    GRALLOC_USAGE_SW_WRITE_MASK   = 0x000000F0,

    /* buffer will be used as an OpenGL ES texture */
    GRALLOC_USAGE_HW_TEXTURE      = 0x00000100,
    /* buffer will be used as an OpenGL ES render target */
    GRALLOC_USAGE_HW_RENDER       = 0x00000200,
    /* buffer will be used by the 2D hardware blitter */
    GRALLOC_USAGE_HW_2D           = 0x00000400,
    /* buffer will be used with the framebuffer device */
    GRALLOC_USAGE_HW_FB           = 0x00001000,
    /* mask for the software usage bit-mask */
    GRALLOC_USAGE_HW_MASK         = 0x00001F00,
};

enum {
    HAL_PIXEL_FORMAT_RGBA_8888          = 1,
    HAL_PIXEL_FORMAT_RGBX_8888          = 2,
    HAL_PIXEL_FORMAT_RGB_888            = 3,
    HAL_PIXEL_FORMAT_RGB_565            = 4,
    HAL_PIXEL_FORMAT_BGRA_8888          = 5,
    HAL_PIXEL_FORMAT_RGBA_5551          = 6,
    HAL_PIXEL_FORMAT_RGBA_4444          = 7,
};

#define OG_DL_FUNC(hndl, fn, type) (type)dlsym(hndl, fn)
#define OG_DL_FUNC_CHECK(hndl, fn_ptr, fn) if (!fn_ptr) { OG_LOGERR("MemTransferAndroid", "could not dynamically link func '%s': %s", fn, dlerror()); dlclose(hndl); return false; }

#pragma mark static initializations and methods

GraphicBufferFnCtor MemTransferAndroid::graBufCreate = NULL;
GraphicBufferFnDtor MemTransferAndroid::graBufDestroy = NULL;
GraphicBufferFnGetNativeBuffer MemTransferAndroid::graBufGetNativeBuffer = NULL;
GraphicBufferFnLock MemTransferAndroid::graBufLock = NULL;
GraphicBufferFnUnlock MemTransferAndroid::graBufUnlock = NULL;

EGLExtFnCreateImage MemTransferAndroid::imageKHRCreate = NULL;
EGLExtFnDestroyImage MemTransferAndroid::imageKHRDestroy = NULL;

bool MemTransferAndroid::initPlatformOptimizations() {
    // load necessary EGL extension functions
	void *dlEGLhndl = dlopen("libEGL.so", RTLD_LAZY);
	if (!dlEGLhndl) {
		OG_LOGERR("MemTransferAndroid", "could not load EGL library: %s", dlerror());
		return false;
	}
	
    imageKHRCreate = OG_DL_FUNC(dlEGLhndl, "eglCreateImageKHR", EGLExtFnCreateImage);
    OG_DL_FUNC_CHECK(dlEGLhndl, imageKHRCreate, "eglCreateImageKHR");
    
    imageKHRDestroy = OG_DL_FUNC(dlEGLhndl, "eglDestroyImageKHR", EGLExtFnDestroyImage);
    OG_DL_FUNC_CHECK(dlEGLhndl, imageKHRDestroy, "eglDestroyImageKHR");
    
	dlclose(dlEGLhndl);
    
    // load necessary Android GraphicBuffer functions
	void *dlUIhndl = dlopen("libui.so", RTLD_LAZY);
	if (!dlUIhndl) {
		OG_LOGERR("MemTransferAndroid", "could not load Android UI library: %s", dlerror());
		return false;
	}
    
    graBufCreate = OG_DL_FUNC(dlUIhndl, "_ZN7android13GraphicBufferC1Ejjij", GraphicBufferFnCtor);
    OG_DL_FUNC_CHECK(dlUIhndl, graBufCreate, "_ZN7android13GraphicBufferC1Ejjij");
    
    graBufDestroy = OG_DL_FUNC(dlUIhndl, "_ZN7android13GraphicBufferD1Ev", GraphicBufferFnDtor);
    OG_DL_FUNC_CHECK(dlUIhndl, graBufDestroy, "_ZN7android13GraphicBufferD1Ev");
    
    graBufGetNativeBuffer = OG_DL_FUNC(dlUIhndl, "_ZNK7android13GraphicBuffer15getNativeBufferEv", GraphicBufferFnGetNativeBuffer);
    OG_DL_FUNC_CHECK(dlUIhndl, graBufGetNativeBuffer, "_ZNK7android13GraphicBuffer15getNativeBufferEv");
    
    graBufLock = OG_DL_FUNC(dlUIhndl, "_ZN7android13GraphicBuffer4lockEjPPv", GraphicBufferFnLock);
    OG_DL_FUNC_CHECK(dlUIhndl, graBufLock, "_ZN7android13GraphicBuffer4lockEjPPv");
    
    graBufUnlock = OG_DL_FUNC(dlUIhndl, "_ZN7android13GraphicBuffer6unlockEv", GraphicBufferFnUnlock);
    OG_DL_FUNC_CHECK(dlUIhndl, graBufUnlock, "_ZN7android13GraphicBuffer6unlockEv");
    
	dlclose(dlUIhndl);
    
    return true;
}

#pragma mark constructor / deconstructor

MemTransferAndroid::~MemTransferAndroid() {
    // release in- and outputs
    releaseInput();
    releaseOutput();
    
    
}

#pragma mark public methods

void MemTransferAndroid::releaseInput() {

}

void MemTransferAndroid::releaseOutput() {

}

void MemTransferAndroid::init() {
    assert(!initialized);
    
    // ...
    
    // call parent init
    MemTransfer::init();
}

GLuint MemTransferAndroid::prepareInput(int inTexW, int inTexH, GLenum inputPxFormat, void *inputDataPtr) {
    assert(initialized && inTexW > 0 && inTexH > 0);
    
    if (inputDataPtr == NULL && inputW == inTexW && inputH == inTexH && inputPixelFormat == inputPxFormat) {
        return inputTexId; // no change
    }
    
    if (preparedInput) {    // already prepared -- release buffers!
        releaseInput();
    }
    
    // set attributes
    inputW = inTexW;
    inputH = inTexH;
    inputPixelFormat = inputPxFormat;
    
    // prepare

    // ...
    
    preparedInput = true;
    
    return inputTexId;
}

GLuint MemTransferAndroid::prepareOutput(int outTexW, int outTexH) {
    assert(initialized && outTexW > 0 && outTexH > 0);
    
    if (outputW == outTexW && outputH == outTexH) {
        return outputTexId; // no change
    }
    
    if (preparedOutput) {    // already prepared -- release buffers!
        releaseOutput();
    }
    
    // set attributes
    outputW = outTexW;
    outputH = outTexH;
    
    // prepare

    // ...
    
    preparedOutput = true;
    
    return outputTexId;
}

void MemTransferAndroid::toGPU(const unsigned char *buf) {
    assert(preparedInput && inputPixelBuffer && inputTexId > 0 && buf);
    
    // ...
}

void MemTransferAndroid::fromGPU(unsigned char *buf) {
    assert(preparedOutput && outputPixelBuffer && outputTexId > 0 && buf);
    
    // ...
}