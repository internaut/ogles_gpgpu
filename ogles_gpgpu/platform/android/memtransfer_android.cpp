#include "memtransfer_android.h"

#include <dlfcn.h>

#include "../../common/tools.h"

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
    
    // all done
    OG_LOGINF("MemTransferAndroid", "static init completed");
    
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
    // release input image
    if (inputImage) {
    	OG_LOGINF("MemTransferAndroid", "releasing input image");
        imageKHRDestroy(EGL_DEFAULT_DISPLAY, inputImage);
		free(inputImage);
		inputImage = NULL;
    }
    
    // release android graphic buffer handle for input
	if (inputGraBufHndl) {
    	OG_LOGINF("MemTransferAndroid", "releasing graphic buffer handle for input");
		graBufDestroy(inputGraBufHndl);
		free(inputGraBufHndl);
        
		inputGraBufHndl = NULL;
        inputNativeBuf = NULL;  // reset weak-ref pointer to NULL
	}
}

void MemTransferAndroid::releaseOutput() {
    // release output image
    if (outputImage) {
    	OG_LOGINF("MemTransferAndroid", "releasing output image");
        imageKHRDestroy(EGL_DEFAULT_DISPLAY, outputImage);
		free(outputImage);
		outputImage = NULL;
    }
    
    // release android graphic buffer handle for output
	if (outputGraBufHndl) {
    	OG_LOGINF("MemTransferAndroid", "releasing graphic buffer handle for output");
		graBufDestroy(outputGraBufHndl);
		free(outputGraBufHndl);
        
		outputGraBufHndl = NULL;
        outputNativeBuf = NULL;  // reset weak-ref pointer to NULL
	}
}

void MemTransferAndroid::init() {
    assert(!initialized);
    
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
    
    // generate texture id for input
    glGenTextures(1, &inputTexId);

    if (inputTexId <= 0) {
    	OG_LOGERR("MemTransferAndroid", "error generating input texture id");
    	return 0;
    }

    // handle input pixel format
    int nativePxFmt = HAL_PIXEL_FORMAT_RGBA_8888;
    if (inputPixelFormat != GL_RGBA) {
        OG_LOGERR("MemTransferAndroid", "warning: only GL_RGBA is valid as input pixel format");
    }

	// create graphic buffer
	inputGraBufHndl = malloc(OG_ANDROID_GRAPHIC_BUFFER_SIZE);
	graBufCreate(inputGraBufHndl, inputW, inputH, nativePxFmt,
                 GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_SW_WRITE_OFTEN);  // is used as OpenGL texture and will be written often
    
	// get window buffer
	inputNativeBuf = (struct ANativeWindowBuffer *)graBufGetNativeBuffer(inputGraBufHndl);
    
    if (!inputNativeBuf) {
        OG_LOGERR("MemTransferAndroid", "error getting native window buffer for input");
        return 0;
    }
    
	// create image for reading back the results
	EGLint eglImgAttrs[] = { EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE, EGL_NONE };
	inputImage = imageKHRCreate(eglGetDisplay(EGL_DEFAULT_DISPLAY),
                                EGL_NO_CONTEXT,
                                EGL_NATIVE_BUFFER_ANDROID,
                                (EGLClientBuffer)inputNativeBuf,
                                eglImgAttrs);	// or NULL as last param?

    if (!inputImage) {
        OG_LOGERR("MemTransferAndroid", "error creating image KHR for input");
        return 0;
    }
    
    preparedInput = true;
    
    OG_LOGINF("MemTransferAndroid", "successfully prepared input with texture id %d", inputTexId);

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
    
    // generate texture id for input
    glGenTextures(1, &outputTexId);

    if (outputTexId <= 0) {
    	OG_LOGERR("MemTransferAndroid", "error generating output texture id");
    	return 0;
    }
    
    // will bind the texture, too:
    setCommonTextureParams(outputTexId);

    Tools::checkGLErr("MemTransferAndroid", "fbo texture parameters");

    // create empty texture space on GPU
	glTexImage2D(GL_TEXTURE_2D, 0,
				 GL_RGBA,
				 outputW, outputH, 0,
			     inputPixelFormat, GL_UNSIGNED_BYTE,
			     NULL);	// we do not need to pass texture data -> it will be generated!

    Tools::checkGLErr("MemTransferAndroid", "fbo texture creation");

	// create graphic buffer
	outputGraBufHndl = malloc(OG_ANDROID_GRAPHIC_BUFFER_SIZE);
	graBufCreate(outputGraBufHndl, outputW, outputH, HAL_PIXEL_FORMAT_RGBA_8888,
                 GRALLOC_USAGE_HW_RENDER | GRALLOC_USAGE_SW_READ_OFTEN);    // is render target and will be read often
    
	// get window buffer
	outputNativeBuf = (struct ANativeWindowBuffer *)graBufGetNativeBuffer(outputGraBufHndl);
    
    if (!outputNativeBuf) {
        OG_LOGERR("MemTransferAndroid", "error getting native window buffer for output");
        return 0;
    }
    
	// create image for reading back the results
	EGLint eglImgAttrs[] = { EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE, EGL_NONE };
	outputImage = imageKHRCreate(eglGetDisplay(EGL_DEFAULT_DISPLAY),
                                EGL_NO_CONTEXT,
                                EGL_NATIVE_BUFFER_ANDROID,
                                (EGLClientBuffer)outputNativeBuf,
                                eglImgAttrs);	// or NULL as last param?
    
    if (!outputImage) {
        OG_LOGERR("MemTransferAndroid", "error creating image KHR for output");
        return 0;
    }
    
    preparedOutput = true;
    
    OG_LOGINF("MemTransferAndroid", "successfully prepared output with texture id %d", outputTexId);

    return outputTexId;
}

void MemTransferAndroid::toGPU(const unsigned char *buf) {
    assert(preparedInput && inputPixelBuffer && inputTexId > 0 && buf);
    
    // bind the input texture
    glBindTexture(GL_TEXTURE_2D, inputTexId);
    
    // activate the image KHR for the input
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, inputImage);
    
    Tools::checkGLErr("MemTransferAndroid", "call to glEGLImageTargetTexture2DOES() for input");
    
	// lock the graphics buffer at graphicsPtr
	unsigned char *graphicsPtr = (unsigned char *)lockBufferAndGetPtr(BUF_TYPE_INPUT);
    
	// copy whole image from "buf" to "graphicsPtr"
    memcpy(graphicsPtr, buf, inputW * inputH * 4);
    
	// unlock the graphics buffer again
	unlockBuffer(BUF_TYPE_INPUT);

}

void MemTransferAndroid::fromGPU(unsigned char *buf) {
    assert(preparedOutput && outputPixelBuffer && outputTexId > 0 && buf);
    
    // bind the output texture
    glBindTexture(GL_TEXTURE_2D, outputTexId);
    
    // activate the image KHR for the output
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, outputImage);
    
    Tools::checkGLErr("MemTransferAndroid", "call to glEGLImageTargetTexture2DOES() for output");
    
	// lock the graphics buffer at graphicsPtr
	const unsigned char *graphicsPtr = (const unsigned char *)lockBufferAndGetPtr(BUF_TYPE_OUTPUT);
    
	// copy whole image from "graphicsPtr" to "buf"
    memcpy(buf, graphicsPtr, outputW * outputH * 4);
    
	// unlock the graphics buffer again
	unlockBuffer(BUF_TYPE_OUTPUT);
}

void *MemTransferAndroid::lockBufferAndGetPtr(BufType bufType) {
    void *hndl;
    int usage;
    unsigned char *memPtr;
    
    if (bufType == BUF_TYPE_INPUT) {
        hndl = inputGraBufHndl;
        usage = GRALLOC_USAGE_SW_WRITE_OFTEN;
    } else {
        hndl = outputGraBufHndl;
        usage = GRALLOC_USAGE_SW_READ_OFTEN;
    }
    
    // lock and get pointer
	graBufLock(hndl, usage, &memPtr);
    
    // check for valid pointer
    if (!memPtr) {
        OG_LOGERR("MemTransferAndroid", "GraphicBuffer lock returned invalid pointer");
    }
    
    return (void *)memPtr;
}

void MemTransferAndroid::unlockBuffer(BufType bufType) {
    void *hndl = (bufType == BUF_TYPE_INPUT) ? inputGraBufHndl : outputGraBufHndl;
    
    graBufUnlock(hndl);
}
