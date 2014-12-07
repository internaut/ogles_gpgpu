#include "og_jni_wrapper.h"

#include "ogles_gpgpu/ogles_gpgpu.h"

#include "og_pipeline.h"

#include <cstdlib>
#include <cassert>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

static ogles_gpgpu::Core *ogCore = NULL;
static jlong outputPxBufNumBytes = 0;
static jobject outputPxBuf = NULL;
static unsigned char *outputPxBufData = NULL;
static jint outputFrameSize[] = { 0, 0 };	// width x height

static EGLConfig eglConf;
static EGLSurface eglSurface;
static EGLContext eglCtx;
static EGLDisplay eglDisp;

bool ogEGLSetup(int w, int h) {
	// EGL config attributes
	const EGLint confAttr[] = {
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,	// very important!
	        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,			// we will create a pixelbuffer surface
	        EGL_RED_SIZE, 	8,
	        EGL_GREEN_SIZE, 8,
	        EGL_BLUE_SIZE, 	8,
	        EGL_ALPHA_SIZE, 8,		// we will need the alpha channel
	        EGL_DEPTH_SIZE, 16,
	        EGL_NONE
	};

	// EGL context attributes
	const EGLint ctxAttr[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2,				// very important!
			EGL_NONE
	};

	// surface attributes
	// the surface size is set to the input frame size
	const EGLint surfaceAttr[] = {
			 EGL_WIDTH, w,
			 EGL_HEIGHT, h,
			 EGL_NONE
	};

	EGLint eglMajVers, eglMinVers;
	EGLint numConfigs;

	eglDisp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (eglDisp == NULL) {
		OG_LOGERR("OGJNIWrapper", "eglGetDisplay failed: %d", eglGetError());
		return false;
	}

	if (!eglInitialize(eglDisp, &eglMajVers, &eglMinVers)) {
		OG_LOGERR("OGJNIWrapper", "eglInitialize failed: %d", eglGetError());
		return false;
	}

	OG_LOGINF("OGJNIWrapper", "EGL init with version %d.%d", eglMajVers, eglMinVers);

	if (!eglChooseConfig(eglDisp, confAttr, &eglConf, 1, &numConfigs)) {	// choose the first config
		OG_LOGERR("OGJNIWrapper", "eglChooseConfig failed: %d", eglGetError());
		return false;
	}

	eglCtx = eglCreateContext(eglDisp, eglConf, EGL_NO_CONTEXT, ctxAttr);
	if (eglCtx == EGL_NO_CONTEXT) {
		OG_LOGERR("OGJNIWrapper", "eglCreateContext failed: %d", eglGetError());
		return false;
	}

	eglSurface = eglCreatePbufferSurface(eglDisp, eglConf, surfaceAttr);	// create a pixelbuffer surface
	if (eglSurface == NULL) {
		OG_LOGERR("OGJNIWrapper", "eglCreatePbufferSurface failed: %d", eglGetError());
		return false;
	}

	if (!eglMakeCurrent(eglDisp, eglSurface, eglSurface, eglCtx)) {
		OG_LOGERR("OGJNIWrapper", "eglMakeCurrent failed: %d", eglGetError());
		return false;
	}

	return true;
}

void ogEGLTeardown() {
    eglMakeCurrent(eglDisp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(eglDisp, eglCtx);
    eglDestroySurface(eglDisp, eglSurface);
    eglTerminate(eglDisp);

    eglDisp = EGL_NO_DISPLAY;
    eglSurface = EGL_NO_SURFACE;
    eglCtx = EGL_NO_CONTEXT;
}

void ogCleanupHelper(JNIEnv *env) {
	if (outputPxBuf && outputPxBufData) {	// buffer is already set, release it first
		env->DeleteGlobalRef(outputPxBuf);
		delete outputPxBufData;

		outputPxBuf = NULL;
		outputPxBufData = NULL;
	}
}

JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_init(JNIEnv *env, jobject obj) {
	assert(ogCore == NULL);
	OG_LOGINF("OGJNIWrapper", "creating instance of ogles_gpgpu::Core");

	ogCore = ogles_gpgpu::Core::getInstance();

	// this method is user-defined and sets up the processing pipeline
	ogPipelineSetup(ogCore);
}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    cleanup
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_cleanup(JNIEnv *env, jobject obj) {
	assert(ogCore);

	OG_LOGINF("OGJNIWrapper", "destroying instance of ogles_gpgpu::Core");

	ogles_gpgpu::Core::destroy();
	ogCore = NULL;

	ogCleanupHelper(env);

	ogEGLTeardown();
}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    prepare
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_prepare(JNIEnv *env, jobject obj, jint w, jint h) {
	assert(ogCore);

	// set up EGL display, context and pixelbuffer surface
	if (!ogEGLSetup(w, h)) {
		OG_LOGERR("OGJNIWrapper", "EGL setup failed. Aborting!");
		return;
	}

	// prepare for frames of size w by h
	ogCore->prepare(w, h);

	ogCleanupHelper(env);

	// get the output frame size
	outputFrameSize[0] = ogCore->getOutputFrameW();
	outputFrameSize[1] = ogCore->getOutputFrameH();

	// create the output buffer as NIO direct byte buffer
	outputPxBufNumBytes = outputFrameSize[0] * outputFrameSize[1] * 4;
	outputPxBufData = new unsigned char[outputPxBufNumBytes];
	outputPxBuf = env->NewDirectByteBuffer(outputPxBufData, outputPxBufNumBytes);
	outputPxBuf = env->NewGlobalRef(outputPxBuf);	// we will hold a reference on this global variable until cleanup is called

	OG_LOGINF("OGJNIWrapper", "preparation successful. input size is %dx%d, output size is %dx%d",
			w, h, outputFrameSize[0], outputFrameSize[1]);
}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    setInputPixels
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_setInputPixels(JNIEnv *env, jobject obj, jintArray pxData) {
	assert(ogCore);

	jint *pxInts = env->GetIntArrayElements(pxData, 0);

	assert(pxInts);

	// cast to bytes and set as input data
	ogCore->setInputData((const unsigned char *)pxInts);

	env->ReleaseIntArrayElements(pxData, pxInts, 0);
}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    getOutputPixels
 * Signature: ()[I
 */
JNIEXPORT jobject JNICALL Java_ogles_1gpgpu_OGJNIWrapper_getOutputPixels(JNIEnv *env, jobject obj) {
	assert(ogCore);

	// write to the output buffer
	ogCore->getOutputData(outputPxBufData);
    
    return outputPxBuf;
}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    process
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_process(JNIEnv *env, jobject obj) {
	assert(ogCore);

	// run the processing operations
	ogCore->process();
}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    getOutputFrameW
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_ogles_1gpgpu_OGJNIWrapper_getOutputFrameW(JNIEnv *env, jobject obj) {
    return outputFrameSize[0];
}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    getOutputFrameH
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_ogles_1gpgpu_OGJNIWrapper_getOutputFrameH(JNIEnv *env, jobject obj) {
	return outputFrameSize[1];
}
