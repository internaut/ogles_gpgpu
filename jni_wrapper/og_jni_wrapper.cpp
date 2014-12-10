//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "og_jni_wrapper.h"

#include "ogles_gpgpu/ogles_gpgpu.h"

#include "og_pipeline.h"

#include <cstdlib>
#include <cassert>
#include <vector>

static ogles_gpgpu::Core *ogCore = NULL;
static jlong outputPxBufNumBytes = 0;
static jobject outputPxBuf = NULL;
static unsigned char *outputPxBufData = NULL;
static jint outputFrameSize[] = { 0, 0 };	// width x height

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

	// initialize EGL context
	if (!ogles_gpgpu::EGL::setup()) {
		OG_LOGERR("OGJNIWrapper", "EGL setup failed!");
	}
}

JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_cleanup(JNIEnv *env, jobject obj) {
	assert(ogCore);

	OG_LOGINF("OGJNIWrapper", "destroying instance of ogles_gpgpu::Core");

	ogles_gpgpu::Core::destroy();
	ogCore = NULL;

	ogCleanupHelper(env);

	ogles_gpgpu::EGL::shutdown();
}

JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_prepare(JNIEnv *env, jobject obj, jint w, jint h) {
	assert(ogCore);

	// set up EGL pixelbuffer surface
	if (!ogles_gpgpu::EGL::createPBufferSurface(w, h)) {
		OG_LOGERR("OGJNIWrapper", "EGL pbuffer creation failed. Aborting!");
		return;
	}

	// activate the EGL context
	if (!ogles_gpgpu::EGL::activate()) {
		OG_LOGERR("OGJNIWrapper", "EGL context activation failed. Aborting!");
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

JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_setInputPixels(JNIEnv *env, jobject obj, jintArray pxData) {
	assert(ogCore);

	jint *pxInts = env->GetIntArrayElements(pxData, 0);

	assert(pxInts);

	// cast to bytes and set as input data
	ogCore->setInputData((const unsigned char *)pxInts);

	env->ReleaseIntArrayElements(pxData, pxInts, 0);
}

JNIEXPORT jobject JNICALL Java_ogles_1gpgpu_OGJNIWrapper_getOutputPixels(JNIEnv *env, jobject obj) {
	assert(ogCore);

	// write to the output buffer
	ogCore->getOutputData(outputPxBufData);
    
    return outputPxBuf;
}

JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_process(JNIEnv *env, jobject obj) {
	assert(ogCore);

	// run the processing operations
	ogCore->process();
}

JNIEXPORT jint JNICALL Java_ogles_1gpgpu_OGJNIWrapper_getOutputFrameW(JNIEnv *env, jobject obj) {
    return outputFrameSize[0];
}

JNIEXPORT jint JNICALL Java_ogles_1gpgpu_OGJNIWrapper_getOutputFrameH(JNIEnv *env, jobject obj) {
	return outputFrameSize[1];
}

JNIEXPORT jdoubleArray JNICALL Java_ogles_1gpgpu_OGJNIWrapper_getTimeMeasurements(JNIEnv *env, jobject obj) {
#ifdef OGLES_GPGPU_BENCHMARK
	std::vector<double> msrmnts = ogCore->getTimeMeasurements();
	size_t num = msrmnts.size();
	jdoubleArray res = env->NewDoubleArray(num);
	jdouble msrmntsArr[num];
	for (size_t i = 0; i < num; ++i) {
		msrmntsArr[i] = msrmnts[i];
	}

	env->SetDoubleArrayRegion(res, 0, num, msrmntsArr);

	return res;

#else
	return null;
#endif
}
