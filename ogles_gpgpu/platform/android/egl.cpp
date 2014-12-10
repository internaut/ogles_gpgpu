//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "egl.h"

#include "../../common/common_includes.h"

using namespace ogles_gpgpu;

EGLConfig EGL::conf = NULL;
EGLSurface EGL::surface = EGL_NO_SURFACE;
EGLContext EGL::ctx = EGL_NO_CONTEXT;
EGLDisplay EGL::disp = EGL_NO_DISPLAY;

bool EGL::setup(int rSize, int gSize, int bSize, int aSize, int depthSize) {
	// EGL config attributes
	const EGLint confAttr[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,	// use OpenGL ES 2.0, very important!
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,			// we will create a pixelbuffer surface
        EGL_RED_SIZE, 	rSize,
        EGL_GREEN_SIZE, gSize,
        EGL_BLUE_SIZE, 	bSize,
        EGL_ALPHA_SIZE, aSize,
        EGL_DEPTH_SIZE, depthSize,
        EGL_NONE
	};
    
	// EGL context attributes
	const EGLint ctxAttr[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,				// use OpenGL ES 2.0, very important!
        EGL_NONE
	};
    
	EGLint eglMajVers, eglMinVers;
	EGLint numConfigs;
    
	disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (disp == EGL_NO_DISPLAY) {
		OG_LOGERR("EGL", "eglGetDisplay failed: %d", eglGetError());
		return false;
	}
    
	if (!eglInitialize(disp, &eglMajVers, &eglMinVers)) {
		OG_LOGERR("EGL", "eglInitialize failed: %d", eglGetError());
		return false;
	}
    
	OG_LOGINF("EGL", "EGL init with version %d.%d", eglMajVers, eglMinVers);
    
	if (!eglChooseConfig(disp, confAttr, &conf, 1, &numConfigs)) {	// choose the first config
		OG_LOGERR("EGL", "eglChooseConfig failed: %d", eglGetError());
		return false;
	}
    
	ctx = eglCreateContext(disp, conf, EGL_NO_CONTEXT, ctxAttr);
	if (ctx == EGL_NO_CONTEXT) {
		OG_LOGERR("EGL", "eglCreateContext failed: %d", eglGetError());
		return false;
	}
    
    return true;
}

bool EGL::createPBufferSurface(int w, int h) {
    assert(disp != EGL_NO_DISPLAY && conf != NULL && ctx != EGL_NO_CONTEXT);
    assert(w > 0 && h > 0);
    
    destroySurface();
    
	// surface attributes
	// the surface size is set to the input frame size
	const EGLint surfaceAttr[] = {
        EGL_WIDTH, w,
        EGL_HEIGHT, h,
        EGL_NONE
	};
    
	surface = eglCreatePbufferSurface(disp, conf, surfaceAttr);	// create a pixelbuffer surface
	if (surface == EGL_NO_SURFACE) {
		OG_LOGERR("EGL", "eglCreatePbufferSurface failed: %d", eglGetError());
		return false;
	}
    
    return true;
}

bool EGL::activate() {
    assert(disp != EGL_NO_DISPLAY && conf != NULL && ctx != EGL_NO_CONTEXT && surface != EGL_NO_SURFACE);
    
	if (!eglMakeCurrent(disp, surface, surface, ctx)) {
		OG_LOGERR("EGL", "eglMakeCurrent failed: %d", eglGetError());
		return false;
	}
    
    return true;
}

bool EGL::deactivate() {
	if (!eglMakeCurrent(EGL_NO_DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
		OG_LOGERR("EGL", "eglMakeCurrent failed: %d", eglGetError());
		return false;
	}
    
    return true;
}

void EGL::shutdown() {
    deactivate();
    
    destroySurface();
    eglDestroyContext(disp, ctx);
    eglTerminate(disp);
    
    disp = EGL_NO_DISPLAY;
    ctx = EGL_NO_CONTEXT;
}

void EGL::destroySurface() {
    if (surface == EGL_NO_SURFACE) return;
    
    eglDestroySurface(disp, surface);
    
    surface = EGL_NO_SURFACE;
}