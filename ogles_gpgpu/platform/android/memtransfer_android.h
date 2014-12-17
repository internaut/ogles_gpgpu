//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * MemTransferAndroid is a platform specific implementation for fast texture access on Android platforms.
 * Applying the GraphicBuffer Hack also used by Mozilla
 * (see http://dxr.mozilla.org/mozilla-central/source/widget/android/AndroidGraphicBuffer.h)
 */

#ifndef OGLES_GPGPU_ANDROID_GL_MEMTRANSFER_ANDROID
#define OGLES_GPGPU_ANDROID_GL_MEMTRANSFER_ANDROID

#include <EGL/egl.h>
#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#endif
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2ext.h>

#include "../../common/gl/memtransfer.h"
#include "../../common/gl/memtransfer_optimized.h"

// "Really I have no idea, but this should be big enough"
#define OG_ANDROID_GRAPHIC_BUFFER_SIZE 1024

namespace ogles_gpgpu {
    
/**
 * typedefs to Android GraphicBuffer functions
 */

// constructor
typedef void (*GraphicBufferFnCtor)(void *graphicBufHndl, uint32_t w, uint32_t h, uint32_t format, uint32_t usage);
    
// deconstructor
typedef void (*GraphicBufferFnDtor)(void *graphicBufHndl);
    
// getNativeBuffer
typedef void* (*GraphicBufferFnGetNativeBuffer)(void *graphicBufHndl);
    
// lock
typedef int (*GraphicBufferFnLock)(void *graphicBufHndl, uint32_t usage, unsigned char **addr);
    
// unlock
typedef int (*GraphicBufferFnUnlock)(void *graphicBufHndl);

/**
 * typedefs to EGL extension functions for ImageKHR extension
 */
    
// create ImageKHR
typedef EGLImageKHR (*EGLExtFnCreateImage)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attribList);

// destroy ImageKHR
typedef EGLBoolean (*EGLExtFnDestroyImage)(EGLDisplay dpy, EGLImageKHR image);

    
/**
 * MemTransferAndroid is a platform specific implementation for fast texture access on Android platforms.
 */
class MemTransferAndroid : public MemTransfer, public MemTransferOptimized {
public:
    /**
     * Try to initialize platform optimizations. Returns true on success, else false.
     */
    static bool initPlatformOptimizations();

    /**
     * Constructor. Set defaults.
     */
    MemTransferAndroid() :  MemTransfer(),
        inputGraBufHndl(NULL),
        outputGraBufHndl(NULL),
        inputNativeBuf(NULL),
        outputNativeBuf(NULL),
        inputImage(NULL),
        outputImage(NULL)
        { }
    
    /**
     * Deconstructor.
     */
    virtual ~MemTransferAndroid();
    
    /**
     * Initialize method to be called AFTER the OpenGL context was created
     */
    virtual void init();
    
    /**
     * Prepare for input frames of size <inTexW>x<inTexH>. Return a texture id for the input frames.
     */
    virtual GLuint prepareInput(int inTexW, int inTexH, GLenum inputPxFormat = GL_RGBA, void *inputDataPtr = NULL);
    
    /**
     * Prepare for output frames of size <outTexW>x<outTexH>. Return a texture id for the output frames.
     */
    virtual GLuint prepareOutput(int outTexW, int outTexH);
    
    /**
     * Delete input texture.
     */
    virtual void releaseInput();
    
    /**
     * Delete output texture.
     */
    virtual void releaseOutput();
    
    /**
     * Map data in <buf> to GPU.
     */
    virtual void toGPU(const unsigned char *buf);
    
    /**
     * Map data from GPU to <buf>
     */
    virtual void fromGPU(unsigned char *buf);
    
    /**
     * Lock the input or output buffer and return its base address.
     * The input buffer will be locked for reading AND writing, while the
     * output buffer will be locked for reading only.
     */
    virtual void *lockBufferAndGetPtr(BufType bufType);
    
    /**
     * Unlock the input or output buffer.
     */
    virtual void unlockBuffer(BufType bufType);
    
private:
    static GraphicBufferFnCtor graBufCreate;        // function pointer to GraphicBufferFnCtor
    static GraphicBufferFnDtor graBufDestroy;       // function pointer to GraphicBufferFnDtor
    static GraphicBufferFnGetNativeBuffer graBufGetNativeBuffer;  // function pointer to GraphicBufferFnGetNativeBuffer
    static GraphicBufferFnLock graBufLock;          // function pointer to GraphicBufferFnLock
    static GraphicBufferFnUnlock graBufUnlock;      // function pointer to GraphicBufferFnUnlock
    
    static EGLExtFnCreateImage  imageKHRCreate;     // function pointer to EGLExtFnCreateImage
    static EGLExtFnDestroyImage  imageKHRDestroy;   // function pointer to EGLExtFnDestroyImage
    
	void *inputGraBufHndl;      // Android GraphicBuffer handle for input
	void *outputGraBufHndl;     // Android GraphicBuffer handle for output
    
	struct ANativeWindowBuffer *inputNativeBuf;     // pointer to native window buffer for input (weak ref - do not free()!)
	struct ANativeWindowBuffer *outputNativeBuf;	// pointer to native window buffer for output (weak ref - do not free()!)
    
	EGLImageKHR inputImage;     // ImageKHR handle for input
	EGLImageKHR outputImage;    // ImageKHR handle for output
};
    
}
#endif
