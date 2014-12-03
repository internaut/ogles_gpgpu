#include "og_jni_wrapper.h"

JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_init(JNIEnv *env, jobject obj) {

}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    cleanup
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_cleanup(JNIEnv *env, jobject obj) {

}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    prepare
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_prepare(JNIEnv *env, jobject obj, jint w, jint h) {

}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    setInputPixels
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_setInputPixels(JNIEnv *env, jobject obj, jintArray pxData) {

}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    getOutputPixels
 * Signature: ()[I
 */
JNIEXPORT jintArray JNICALL Java_ogles_1gpgpu_OGJNIWrapper_getOutputPixels(JNIEnv *env, jobject obj) {
    jintArray dummy = env->NewIntArray(10);
    
    return dummy;
}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    process
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_process(JNIEnv *env, jobject obj) {

}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    getOutputFrameW
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_ogles_1gpgpu_OGJNIWrapper_getOutputFrameW(JNIEnv *env, jobject obj) {
    return 0;
}

/*
 * Class:     ogles_gpgpu_OGJNIWrapper
 * Method:    getOutputFrameH
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_ogles_1gpgpu_OGJNIWrapper_getOutputFrameH(JNIEnv *env, jobject obj) {
    return 0;
}
