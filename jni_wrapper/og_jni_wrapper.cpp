#include "og_jni_wrapper.h"

#include "ogles_gpgpu/ogles_gpgpu.h"

#include <cstdlib>
#include <cassert>

static ogles_gpgpu::Core *ogCore = NULL;

JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_init(JNIEnv *env, jobject obj) {
	assert(ogCore == NULL);
	OG_LOGINF("OGJNIWrapper", "creating instance of ogles_gpgpu::Core");

	ogCore = ogles_gpgpu::Core::getInstance();
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
}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    prepare
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_prepare(JNIEnv *env, jobject obj, jint w, jint h) {
	assert(ogCore);
	ogCore->prepare(w, h);
}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    setInputPixels
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_setInputPixels(JNIEnv *env, jobject obj, jintArray pxData) {
	assert(ogCore);
}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    getOutputPixels
 * Signature: ()[I
 */
JNIEXPORT jintArray JNICALL Java_ogles_1gpgpu_OGJNIWrapper_getOutputPixels(JNIEnv *env, jobject obj) {
	assert(ogCore);

    jintArray dummy = env->NewIntArray(10);
    
    return dummy;
}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    process
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_process(JNIEnv *env, jobject obj) {
	assert(ogCore);
}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    getOutputFrameW
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_ogles_1gpgpu_OGJNIWrapper_getOutputFrameW(JNIEnv *env, jobject obj) {
	assert(ogCore);

    return ogCore->getOutputFrameW();
}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    getOutputFrameH
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_ogles_1gpgpu_OGJNIWrapper_getOutputFrameH(JNIEnv *env, jobject obj) {
	assert(ogCore);

    return ogCore->getOutputFrameH();
}
