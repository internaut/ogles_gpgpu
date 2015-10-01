//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

package ogles_gpgpu.examples.ogvideoprocdroid;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.util.Arrays;

import ogles_gpgpu.OGJNIWrapper;

import com.android.grafika.gles.EglCore;
import com.android.grafika.gles.WindowSurface;

import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.Size;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

/**
 * GPGPU based image processing of live camera frames using ogles_gpgpu.
 *
 * This example shows:
 * - how to feed live camera frames as OpenGL textures into ogles_gpgpu
 * - how to display the original input frames and the processed frames
 * - how to read the processed frame and further process it on the CPU side
 * - how to handle onPause() / onResume() in conjunction with the above
 *   features
 *
 * CamActivity implements the usage of ogles_gpgpu in conjunction with live camera
 * frames. These frames are acquired as OpenGL textures and fed directly into the
 * ogles_gpgpu library for image processing. Some simple filters are applied via
 * GPGPU to the camera frame (downscaling and grayscale conversion plus Gauss
 * blurring). Afterwards, the image is read back to main memory as a ByteBuffer and
 * is be further processed on the CPU side - a live grayscale histogram is
 * calculated. This is just to show how GPU-processed frames can be acquired from
 * ogles_gpgpu and can then be further processed on the CPU side, for example when
 * it is not possible to implement an algorithm as OpenGL shader.
 *
 * The GPGPU image processing functions are continuously applied to the live camera
 * frames. For this, the native functions of the ogles_gpgpu library are called via
 * JNI. See the wrapper class OGJNIWrapper and the C++ sources in the "jni" folder.
 *
 * This project uses the old, deprecated "Camera" API but therefore still runs on
 * older Android systems than 5.0 "Lollipop". Using the Camera2 API should work
 * in a similar way.
 *
 * Some methods here were copied from the Google Grafika project (https://github.com/google/grafika/)
 * See https://github.com/google/grafika/blob/master/src/com/android/grafika/TextureFromCameraActivity.java
 */
public class CamActivity
    extends Activity
    implements SurfaceHolder.Callback,
    SurfaceTexture.OnFrameAvailableListener {
    private final static String TAG = "CamActivity";

    private final static int CAM_FPS = 30;    /** requested camera frames. change this if it is not supported by your device */
    private final static int CAM_FRAME_TEXTURE_TARGET = GLES11Ext.GL_TEXTURE_EXTERNAL_OES;

    /**
     * SurfaceHolder instance which is set during surfaceCreated() and is destroyed and
     * set to null in surfaceDestroyed(). This is the case when the application is send
     * to background. It is *not* the case, when the power-on/off button is pressed and
     * the screen is switched off. The surfaceHolder object will remain then.
     */
    private static SurfaceHolder surfaceHolder;

    /**
     * Grafika's EGL interface helper.
     */
    private EglCore eglCore;

    /**
     * ogles_gpgpu JNI interface.
     */
    private OGJNIWrapper ogWrapper;

    /**
     * Thread for asynchronous image processing on the CPU side.
     */
    private CPUImgProcThread imgProcThread;

    /**
     * Histogram view.
     */
    private HistView histView;

    /**
     * Grafika's full-screen window surface.
     */
    private WindowSurface windowSurface;

    /**
     * Camera device handler.
     */
    private Camera cam;

    /**
     * Camera preview frames size.
     */
    private Size camPreviewFrameSize;

    /**
     * Camera lock for safe starting/stopping the camera.
     */
    private Object camLock = new Object();

    /**
     * Camera frame texture surface.
     */
    private SurfaceTexture camTexture;

    /**
     * Camera frame texture OpenGL ID connected to the texture surface.
     */
    private int camTextureId;


    /**
     * Activity creation - set content view.
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_cam);

        histView = (HistView)findViewById(R.id.hist_view);
    }

    /**
     * Activity resume - create ogles_gpgpu wrapper object, create and start
     * image processing thread, start the camera und re-initialize the surface
     * if it already exists (screen was switched off before).
     */
    @Override
    protected void onResume() {
        Log.i(TAG, "onResume");
        super.onResume();

        // create ogles_gpgpu wrapper object
        if (ogWrapper == null) {
            ogWrapper = new OGJNIWrapper();
        }

        // create and start image processing thread for further image processing
        // on the CPU side after the camera frame was initially processed on the GPU
        imgProcThread = new CPUImgProcThread();
        imgProcThread.start();
        imgProcThread.setName("CPUImageProcThread");

        // start the camera
        startCam();

        // add this Activity as callback to the SurfaceView which will later
        // show the camera frames. this causes that surfaceCreated() will later
        // be called after the surface appeared. in this function, further
        // initializations are executed.
        SurfaceView sv = (SurfaceView)findViewById(R.id.surface_view);
        sv.getHolder().addCallback(this);

        // check if the surface holder already exists. this is the case when
        // the screen was turned off and on again
        if (surfaceHolder != null) {
            Log.i(TAG, "surface holder already existent");
            mainInit(surfaceHolder, false);  // re-initialize everything in this case
        } else {
            Log.i(TAG, "surface holder will be set automatically");
            // surfaceCreated() will be called automatically later when the
            // SurfaceView appears
        }
    }

    /**
     * Activity pause - stops the camera, the image processing thread and
     * releases OpenGL stuff when the application is set to background or
     * the screen is turned off.
     */
    @Override
    protected void onPause() {
        Log.i(TAG, "onPause");

        // stop the camera preview and the camera capture
        stopCam();

        // stop the image processing thread *after* the camera was released
        synchronized (camLock) {
            stopImgProcThread();
        }

        // release OpenGL resources and the WindowSurface
        releaseGL();

        // reset the EGL state
        resetEGL();

        super.onPause();
    }

    /**
     * Button callback for the buttons on top ("show input" / "show output").
     * @param btn button that was clicked
     */
    public void topBtnClicked(View btn) {
        int btnId = btn.getId();

        if (btnId == R.id.btn_show_input) {
            ogWrapper.setRenderDispShowMode(OGJNIWrapper.RENDER_DISP_MODE_INPUT);
        } else {
            ogWrapper.setRenderDispShowMode(OGJNIWrapper.RENDER_DISP_MODE_OUTPUT);
        }
    }

    /**
     * Surface creation callback. Will set the surface holder and call the
     * mainInit() method.
     */
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.i(TAG, "surface created");

        surfaceHolder = holder;

        mainInit(surfaceHolder, true);
    }

    /**
     * Surface change callback. Is called when the surface dimensions change and right
     * at the beginning. After that, the surface dimensions are known and the ogles_gpgpu
     * render display is configured. Furthermore, ogles_gpgpu is prepared to receive
     * camera frames of a certain size.
     */
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.i(TAG, "surface changed to size " + width + "x" + height);

        ogWrapper.setRenderDisp(width, height, OGJNIWrapper.ORIENTATION_FLIPPED);
        ogWrapper.prepare(camPreviewFrameSize.width, camPreviewFrameSize.height, false);
    }

    /**
     * Surface destruction callback. Is called when the SurfaceView disappears because
     * the application goes to the background.
     */
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.i(TAG, "surface destroyed");

        // reset the surface holder because it will not be valid anymore
        surfaceHolder = null;

        // clean up the ogles_gpgpu system
        if (ogWrapper != null) {
            Log.i(TAG, "releasing ogles_gpgpu");
            ogWrapper.cleanup();
            ogWrapper = null;
        }

        // reset and release EGL
        resetEGL();
        releaseEGL();
    }

    /**
     * Callback from SurfaceTexture.OnFrameAvailableListener. Is called each time a new
     * camera frame for the SurfaceTexture is available.
     */
    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        // check if these objects are null, because they can become unable during
        // application shutdown
        if (camTexture == null || windowSurface == null) return;

  //eglCore.makeCurrent(windowSurface); //  public void makeCurrent(EGLSurface eglSurface)
        windowSurface.makeCurrent();

        // update camera frame texture
        GLES20.glActiveTexture(GLES20.GL_TEXTURE1);

        camTexture.updateTexImage();

        // set ogles_gpgpu input texture: the camera frame texture id
        ogWrapper.setInputTexture(camTextureId);

        // run the GPGPU processing functions
        ogWrapper.process();

        // also render the output to the display
        ogWrapper.renderOutput();

        // update histogram in separate thread
        if (imgProcThread != null && imgProcThread.isRunning()) {
            imgProcThread.update();
        }

        // swap GL display buffers
        windowSurface.swapBuffers();
    }

    /**
     * helper function to stop the image processing thread.
     */
    private void stopImgProcThread() {
        imgProcThread.terminate();

        try {
            imgProcThread.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        imgProcThread = null;
    }

    /**
     * release the camera texture and the window surface.
     */
    private void releaseGL() {
        if (camTexture != null) {
            Log.i(TAG, "releasing camera frame texture");
            GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
            int[] t = { camTextureId };
            GLES20.glDeleteTextures(1, t, 0);

            camTexture.release();
            camTexture = null;
        }

        if (windowSurface != null) {
            Log.i(TAG, "releasing window surface");
            windowSurface.release();
            windowSurface = null;
        }
    }

    /**
     * Reset EGL state.
     */
    private void resetEGL() {
        if (eglCore != null) {
            Log.i(TAG, "resetting EGL");
            eglCore.makeNothingCurrent();
        }
    }

    /**
     * Release EGL.
     */
    private void releaseEGL() {
        if (eglCore != null) {
            Log.i(TAG, "releasing EGL");
            eglCore.release();
        }
    }

    /**
     * Open the camera device.
     */
    private void startCam() {
        if (cam != null) return;  // already existing -> abort

        Log.i(TAG, "starting camera");

        synchronized (camLock) {
            // get camera information
            Camera.CameraInfo camInfo = new Camera.CameraInfo();

            // get the back facing camera
            int numCameras = Camera.getNumberOfCameras();
            for (int i = 0; i < numCameras; i++) {
                Camera.getCameraInfo(i, camInfo);
                if (camInfo.facing == Camera.CameraInfo.CAMERA_FACING_BACK) {
                    cam = Camera.open(i);
                    break;
                }
            }
            if (cam == null) {
                Log.i(TAG, "No back-facing camera found; opening default");
                cam = Camera.open();    // opens first back-facing camera
            }
            if (cam == null) {
                throw new RuntimeException("Unable to open camera");
            }

            // get the camera parameters and set the preview size and FPS rate
            Camera.Parameters camParams = cam.getParameters();
            camPreviewFrameSize = camParams.getPreferredPreviewSizeForVideo();
            camParams.setPreviewSize(camPreviewFrameSize.width, camPreviewFrameSize.height);

            camParams.setPreviewFpsRange(CAM_FPS * 1000, CAM_FPS * 1000);
            camParams.setRecordingHint(true);
            cam.setParameters(camParams);

            int[] fps = new int[2];
            camParams.getPreviewFpsRange(fps);

            Log.i(TAG, "camera preview frame size is " + camPreviewFrameSize.width + "x" + camPreviewFrameSize.height);
            Log.i(TAG, "camera preview fps range is " + fps[0] / 1000.0f + " - " + fps[1] / 1000.0f);
        }
    }

    /**
     * Stop camera preview and close camera device.
     */
    private void stopCam() {
        if (cam == null) return;

        synchronized (camLock) {
            cam.stopPreview();
            cam.setPreviewCallback(null);
            cam.release();
            cam = null;
        }

        Log.i(TAG, "stopped camera");
    }

    /**
     * Main initializer function. Called either from surfaceCreated() or, when the
     * surface still exists, directly from onResume().
     *
     * Will initialize EGL and ogles_gpgpu. Will create a window surface and a
     * camera frame texture. Will start the camera output at the end.
     *
     * @param holder
     * @param isNew
     */
    private void mainInit(SurfaceHolder holder, boolean isNew) {
        Log.i(TAG, "main initializer started (is new surface: " + isNew + ")");

        if (isNew) {
            // init EGL
            Log.i(TAG, "initializing EGL");
            eglCore = new EglCore();


            // init ogles_gpgpu: use platform optimizations, do NOT init
            // EGL (already done before) and create a render display.
            Log.i(TAG, "initializing ogles_gpgpu");
            ogWrapper.init(true, false, true);
        }

        // create window surface and make it the current window
        Log.i(TAG, "creating window surface");
        windowSurface = new WindowSurface(eglCore, holder.getSurface(), false);
        windowSurface.makeCurrent();

        // create camera frame texture and this Activity as callback for
        // new camera frames
        Log.i(TAG, "creating camera frame texture");
        camTextureId = createGLTexture();
        camTexture = new SurfaceTexture(camTextureId);
        camTexture.setOnFrameAvailableListener(this);

        Log.i(TAG, "created camera frame texture with id " + camTextureId);

        // set "camTexture" as the Camera object's preview texture
        try {
            cam.setPreviewTexture(camTexture);
        } catch (IOException e) {
            e.printStackTrace();
        }

        // start the camera preview output
        Log.i(TAG, "starting camera preview");
        cam.startPreview();
    }

    /**
     * helper function to create and setup a new OpenGL texture.
     * @return new OpenGL texture ID
     */
    private int createGLTexture() {
        // set texture unit
        GLES20.glActiveTexture(GLES20.GL_TEXTURE1);

        // create texture ID
        int[] textures = new int[1];
        GLES20.glGenTextures(1, textures, 0);

        int texId = textures[0];

        if (texId <= 0) {
            throw new RuntimeException("invalid GL texture id generated");
        }

        // bind texture to special target "GL_TEXTURE_EXTERNAL_OES"
        // this is absolutely necessary for camera frames!
        GLES20.glBindTexture(CAM_FRAME_TEXTURE_TARGET, texId);

        // do NOT use mipmapping (usually only available for POT textures)
        GLES20.glTexParameterf(CAM_FRAME_TEXTURE_TARGET, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameterf(CAM_FRAME_TEXTURE_TARGET, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);

        // set clamping
        GLES20.glTexParameteri(CAM_FRAME_TEXTURE_TARGET, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(CAM_FRAME_TEXTURE_TARGET, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);

        // return id
        return texId;
    }


    /**
     * Image processing thread for asynchronous processing on the CPU side.
     * When update() is called on this thread, it will take the current output
     * of ogles_gpgpu (the current GPU-processed frame) and run a histogram
     * calculation on it. The result is send to the HistView instance.
     * This is just to show how GPU-processed frames can be acquired from
     * ogles_gpgpu and can then be further processed on the CPU side, for
     * example when it is not possible to implement an algorithm as OpenGL
     * shader.
     */
    private class CPUImgProcThread extends Thread {
        private boolean running = false;
        private ByteBuffer imgData;            // passed output pixel data as ARGB bytes values
        private float[] outputHist = new float[256];  // output histogram

        /**
         * Thread run loop. Will run until terminate() is called.
         */
        @Override
        public void run() {
            Log.i(TAG, "starting thread CPUImgProcThread");

            running = true;

            while (running) {
                // check if new image data was received
                if (imgData != null) {
                    // calculate the histogram from the image data
                    calcHist(imgData);

                    // send it to histView, where a copy will be made
                    histView.getHistCopy(outputHist);

                    // invalidate the histView, so it will be redrawn.
                    // this *must* be called on the UI thread
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            histView.invalidate();
                        }
                    });

                    // reset and wait for new image data
                    imgData = null;
                }
            }

            // reset
            imgData = null;

            Log.i(TAG, "stopped thread CPUImgProcThread");
        }

        /**
         * Stop the thread.
         */
        public void terminate() {
            running = false;
        }

        /**
         * Acquire new image data from ogles_gpgpu. The "imgData" ByteBuffer is
         * only a reference to the actual image data on the native side! It is
         * only valid until the next call to "getOutputPixels()"!
         */
        public void update() {
            if (imgData != null || !running) return;  // image data already in use right now

            // get reference to current result image data
            imgData = ogWrapper.getOutputPixels();
            imgData.rewind();
        }

        /**
         * Thread running status
         * @return thread running status
         */
        public boolean isRunning() {
            return running;
        }

        /**
         * Calculate the 256 bin histogram of a grayscale image.
         * @param pxData grayscale image data
         */
        private void calcHist(ByteBuffer pxData) {
            // use an IntBuffer for image data access.
            // each pixel is a int value with RGBA data
            IntBuffer intData = pxData.asIntBuffer();

            // reset histogram to zeros
            Arrays.fill(outputHist, 0.0f);

            // count values and store them in absolute histogram
            while (intData.hasRemaining()) {
                int grayVal = (intData.get() >> 8) & 0x000000FF;  // get the "B" channel

                outputHist[grayVal] += 1.0f;
            }

            intData.rewind();

            // normalize histogram
            float maxVal = 0.0f;
            for (float v : outputHist) {
                if (v > maxVal) {
                    maxVal = v;
                }
            }

            for (int i = 0; i < outputHist.length; i++) {
                outputHist[i] /= maxVal;
            }
        }

        /**
         * Print the histogram to console.
         * @param h histogram array
         */
        private void printHist(float[] h) {
            Log.i(TAG, "output histogram values: " + Arrays.toString(outputHist));
        }
    }
}
