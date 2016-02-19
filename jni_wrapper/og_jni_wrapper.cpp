//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "og_jni_wrapper.h"

// include ogles_gpgpu library
#include "ogles_gpgpu/ogles_gpgpu.h"

// in "og_pipeline", a custom definition of the processing pipeline can be written
#include "og_pipeline.h"

#include <cstdlib>
#include <cassert>
#include <vector>

/**
 * common objects and variables
 */

static ogles_gpgpu::Core *ogCore = NULL;		// ogles_gpgpu core manager instance
static ogles_gpgpu::Disp *ogDisp = NULL;		// ogles_gpgpu render-to-display object (weak ref)

static bool eglInitRequested = false;			// is true if init() is called with initEGL = true
static bool ogInitialized = false;				// is true after init() was called

static jlong outputPxBufNumBytes = 0;			// number of bytes in output buffer
static jobject outputPxBuf = NULL;				// DirectByteBuffer object pointing to <outputPxBufData>
static unsigned char *outputPxBufData = NULL;	// pointer to data in DirectByteBuffer <outputPxBuf>
static jint outputFrameSize[] = { 0, 0 };		// width x height

static GLuint ogInputTexId	= 0;				// input texture id
static GLuint ogOutputTexId	= 0;				// output texture id

/**
 * Cleanup helper. Will release the output pixel buffer.
 */
void ogCleanupHelper(JNIEnv *env) {
    if (outputPxBuf && outputPxBufData) {	// buffer is already set, release it first
        env->DeleteGlobalRef(outputPxBuf);
        delete outputPxBufData;

        outputPxBuf = NULL;
        outputPxBufData = NULL;
    }
}

/**
 * Initialize ogles_gpgpu. Call this function at first to use ogles_gpgpu.
 * @param platOpt try to enable platform optimizations
 * @param initEGL initialize EGL system on the native side
 * @param createRenderDisp create a render display which will render to output to screen
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_init(JNIEnv *env, jobject obj, jboolean platOpt, jboolean initEGL, jboolean createRenderDisp) {
    assert(ogCore == NULL);
    OG_LOGINF("OGJNIWrapper", "creating instance of ogles_gpgpu::Core");

    ogCore = ogles_gpgpu::Core::getInstance();

    if (platOpt) {
        ogles_gpgpu::Core::tryEnablePlatformOptimizations();
    }

    // this method is user-defined and sets up the processing pipeline
    ogPipelineSetup(ogCore);

    // create a render display output
    if (createRenderDisp) {
        ogDisp = ogCore->createRenderDisplay();
    }

    // initialize EGL context
    if (initEGL && !ogles_gpgpu::EGL::setup()) {
        OG_LOGERR("OGJNIWrapper", "EGL setup failed!");
    }

    eglInitRequested = initEGL;
}

/**
 * Cleanup the ogles_gpgpu resources. Call this only once when you quit using ogles_gpgpu.
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_cleanup(JNIEnv *env, jobject obj) {
    assert(ogCore);

    OG_LOGINF("OGJNIWrapper", "destroying instance of ogles_gpgpu::Core");

    ogles_gpgpu::Core::destroy();
    ogCore = NULL;

    ogCleanupHelper(env);

    if (eglInitRequested) {
        ogles_gpgpu::EGL::shutdown();
    }
}

/**
 * Prepare ogles_gpgpu for incoming images of size <inW> x <inH>. Do this
 * each time you change the input image size (and of course at the beginning
 * for the initial input image size).
 *
 * @param inW input frame width
 * @param inH input frame height
 * @param prepareDataInput set to true if you later want to copy data to ogles_gpgpu
 *						   by using setInputPixels(). set to false if you submit
 *						   input data by texture via setInputTexture().
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_prepare(JNIEnv *env, jobject obj, jint w, jint h, jboolean prepareDataInput) {
    assert(ogCore);

    if (eglInitRequested) {
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
    }

    // initialize ogles_gpgpu
    if (!ogInitialized) {
        ogCore->init();
        ogInitialized = true;
    }

    // prepare for frames of size w by h
    ogCore->prepare(w, h, prepareDataInput ? GL_RGBA : GL_NONE);

    ogCleanupHelper(env);

    // get the output frame size
    outputFrameSize[0] = ogCore->getOutputFrameW();
    outputFrameSize[1] = ogCore->getOutputFrameH();

    // create the output buffer as NIO direct byte buffer
    outputPxBufNumBytes = outputFrameSize[0] * outputFrameSize[1] * 4;
    outputPxBufData = new unsigned char[outputPxBufNumBytes];
    outputPxBuf = env->NewDirectByteBuffer(outputPxBufData, outputPxBufNumBytes);
    outputPxBuf = env->NewGlobalRef(outputPxBuf);	// we will hold a reference on this global variable until cleanup is called

    // get output texture id
    ogOutputTexId = ogCore->getOutputTexId();

    OG_LOGINF("OGJNIWrapper", "preparation successful. input size is %dx%d, output size is %dx%d",
              w, h, outputFrameSize[0], outputFrameSize[1]);
}

/**
 * Specify render display properties. Before that, <init()> must have been called with
 * "createRenderDisp" = true.
 *
 * @param w render display width
 * @param h render display height
 * @param orientation render orientation (one of "ORIENTATION_")
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_setRenderDisp(JNIEnv *env, jobject obj, jint w, jint h, jint orientation) {
    assert(ogInitialized && ogDisp);

    ogDisp->setOutputSize(w, h);
    ogDisp->setOutputRenderOrientation((ogles_gpgpu::RenderOrientation)orientation);
}

/**
 * Set render display to either show camera input or output.
 *
 * @param mode one of "RENDER_DISP_MODE_"
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_setRenderDispShowMode(JNIEnv *env, jobject obj, jint mode) {
    assert(ogInitialized && ogDisp);
    assert(ogInputTexId > 0);
    assert(ogOutputTexId > 0);

    if (mode == ogles_gpgpu_OGJNIWrapper_RENDER_DISP_MODE_INPUT) {
        ogDisp->useTexture(ogInputTexId, 1, GL_TEXTURE_EXTERNAL_OES);
    } else {
        ogDisp->useTexture(ogOutputTexId, 1, GL_TEXTURE_2D);
    }
}

/**
 * Set the raw input pixel data as ARGB integer array. The size of this array
 * must equal <inW> * <inH> (set via <prepare()>).
 *
 * @param pixels    pixel data with ARGB integers
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_setInputPixels(JNIEnv *env, jobject obj, jintArray pxData) {
    assert(ogCore);

    // get pointer to int data in java array
    jint *pxInts = env->GetIntArrayElements(pxData, 0);

    assert(pxInts);

    // cast to bytes and set as input data
    ogCore->setInputData((const unsigned char *)pxInts);

    // release pointer again
    env->ReleaseIntArrayElements(pxData, pxInts, 0);
}

/**
 * Set input as reference to a texture with ID <texID>.
 * @param texId
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_setInputTexture(JNIEnv *env, jobject obj, jint texId) {
    ogCore->setInputTexId(texId, GL_TEXTURE_EXTERNAL_OES);
    ogInputTexId = texId;
}


/**
 * Return the input pixel data as ARGB ByteBuffer. The size of this byte buffer
 * equals output frame width * output frame height * 4 (4 channel ARGB data).
 *
 * Note: The returned ByteBuffer is only a reference to the actual image data
 * on the native side! It is only valid until the next call to this function!
 *
 * @return reference to pixel data as ByteBuffer valid unit next call to this function
 */
JNIEXPORT jobject JNICALL Java_ogles_1gpgpu_OGJNIWrapper_getOutputPixels(JNIEnv *env, jobject obj) {
    assert(ogCore);

    // write to the output buffer
    ogCore->getOutputData(outputPxBufData);

    return outputPxBuf;
}

/**
 * Executes the GPGPU processing tasks.
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_process(JNIEnv *env, jobject obj) {
    assert(ogCore);

    // run the processing operations
    ogCore->process();
}

/**
 * Render the output to a render display. Before that, <init()> must have been called
 * with "createRenderDisp" = true
 */
JNIEXPORT void JNICALL Java_ogles_1gpgpu_OGJNIWrapper_renderOutput(JNIEnv *, jobject) {
    assert(ogInitialized && ogDisp);

    ogDisp->render();
}


/**
 * Get the output image width.
 * @return output image width
 */
JNIEXPORT jint JNICALL Java_ogles_1gpgpu_OGJNIWrapper_getOutputFrameW(JNIEnv *env, jobject obj) {
    return outputFrameSize[0];
}

/**
 * Get the output image height.
 * @return output image height
 */
JNIEXPORT jint JNICALL Java_ogles_1gpgpu_OGJNIWrapper_getOutputFrameH(JNIEnv *env, jobject obj) {
    return outputFrameSize[1];
}


/**
 * Return the time measurements in milliseconds for the individual steps: input, process, output
 * if ogles_gpgpu was compiled for benchmarking, otherwise return null.
 *
 * @return time measurements in milliseconds for the individual steps input, process, output
 * 		   or null if ogles_gpgpu was not compiled for benchmarking
 */
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
    return NULL;
#endif
}
