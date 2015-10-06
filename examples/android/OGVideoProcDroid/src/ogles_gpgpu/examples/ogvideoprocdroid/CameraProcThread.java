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

import ogles_gpgpu.OGJNIWrapper;

import com.android.grafika.gles.EglCore;
import com.android.grafika.gles.WindowSurface;

import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.Size;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.util.Log;
import android.view.SurfaceHolder;

/**
 * Camera frames processing thread.
 * We should use separate thread to avoid conflicts with main-UI OpenGL's context.
 * See for details:
 *  - https://github.com/internaut/ogles_gpgpu/issues/1
 *  - http://stackoverflow.com/questions/27626002/camera-preview-using-surface-texture
 * See CamActivity class for more information.
 *
 * All public methods except 'run' called from other threads.
 */
class CameraProcThread
    extends Thread
    implements SurfaceTexture.OnFrameAvailableListener {

    /**
      * Requested camera frames. Change this if it is not
      * supported by your device.
      */
    private final static int CAM_FPS = 30;

    private final static int CAM_FRAME_TEXTURE_TARGET = GLES11Ext.GL_TEXTURE_EXTERNAL_OES;

    private final Object lock = new Object();

    /**
     * ogles_gpgpu JNI interface.
     */
    private final OGJNIWrapper ogWrapper = new OGJNIWrapper();

    private CPUImgProcThread imgProcThread = null;

    /**
     * Camera frame texture OpenGL ID connected to the texture surface.
     */
    private int camTextureId = -1;

    /**
     * Camera preview frames size.
     */
    private Size camPreviewFrameSize = null;

    /**
      * Current state of the thread.
      */
    private boolean running = true;

    /**
     * Grafika's EGL interface helper.
     */
    private EglCore eglCore = new EglCore();

    /**
     * Camera frame availability notification. Variable is set in
     * onFrameAvailable call and mainly needed to check that wakeup
     * is not spurious.
     */
    private boolean frameNotification = false;

    /**
     * Grafika's full-screen window surface.
     */
    private WindowSurface windowSurface;

    /**
     * Camera frame texture surface.
     */
    private SurfaceTexture camTexture = null;

    private SurfaceHolder holder;

    /**
      * Flag: surfaceChanged callback is received from main UI.
      */
    private boolean isSurfaceChanged = false;

    /**
      * Values from surfaceChanged callback.
      */
    private int newWidth = -1;
    private int newHeight = -1;

    /**
      * Flag: Button clicked event received
      */
    private boolean modeChanged = false;

    /**
      * Flag for new mode.
      * - true: new mode is "input"
      * - false: new mode is "output"
      */
    private boolean newModeInput = false;

    /**
     * Constructor. Called either from surfaceCreated() or, when the
     * surface still exists, directly from onResume().
     */
    public CameraProcThread(CPUImgProcThread thread, SurfaceHolder surfaceHolder) {
        imgProcThread = thread;
        holder = surfaceHolder;
    }

    /**
     * Thread run loop. Will run until terminate() is called.
     */
    @Override
    public void run() {
        Log.i(CamActivity.TAG, "starting thread CameraProcThread");

        // Open the camera device.
        Camera cam = startCam();

        // init ogles_gpgpu: use platform optimizations, do NOT init
        // EGL (already done before) and create a render display.
        Log.i(CamActivity.TAG, "initializing ogles_gpgpu");
        ogWrapper.init(true, false, true);

        // create window surface and make it the current window
        Log.i(CamActivity.TAG, "creating window surface");
        windowSurface = new WindowSurface(eglCore, holder.getSurface(), false);
        windowSurface.makeCurrent();

        // create camera frame texture and this Activity as callback for
        // new camera frames
        Log.i(CamActivity.TAG, "creating camera frame texture");
        camTextureId = createGLTexture();
        camTexture = new SurfaceTexture(camTextureId);
        camTexture.setOnFrameAvailableListener(this);

        Log.i(CamActivity.TAG, "created camera frame texture with id " + camTextureId);

        // set "camTexture" as the Camera object's preview texture
        try {
            cam.setPreviewTexture(camTexture);
        } catch (IOException e) {
            e.printStackTrace();
            Log.e(CamActivity.TAG, "setPreviewTexture failed");
        }

        // start the camera preview output
        Log.i(CamActivity.TAG, "starting camera preview");
        cam.startPreview();

        try {
            mainLoop();
        }
        catch (Exception e) {
            e.printStackTrace();
            Log.e(CamActivity.TAG, "CameraProcThread mainLoop");
        }

        stopCam(cam);

        Log.i(CamActivity.TAG, "releasing ogles_gpgpu");
        ogWrapper.cleanup();

        // Reset EGL state.
        eglCore.makeNothingCurrent();
        eglCore.release();

        // release OpenGL resources and the WindowSurface
        releaseGL();

        Log.i(CamActivity.TAG, "stopped thread CameraProcThread");
    }

    private void mainLoop() throws Exception {
        while (running) {
            boolean frameAvailable = false;
            synchronized (lock) {
                if (isSurfaceChanged) {
                    // Rare event, process immediately
                    ogWrapper.setRenderDisp(newWidth, newHeight, OGJNIWrapper.ORIENTATION_FLIPPED);
                    ogWrapper.prepare(camPreviewFrameSize.width, camPreviewFrameSize.height, false);
                    isSurfaceChanged = false;
                }
                else if (modeChanged) {
                    // Rare event, process immediately
                    if (newModeInput) {
                        ogWrapper.setRenderDispShowMode(OGJNIWrapper.RENDER_DISP_MODE_INPUT);
                    } else {
                        ogWrapper.setRenderDispShowMode(OGJNIWrapper.RENDER_DISP_MODE_OUTPUT);
                    }
                    modeChanged = false;
                }
                else if (frameNotification) {
                    // Frequent event, do release lock as soon as possible so if one more
                    // call to onFrameAvailable occurs then there will be
                    // no need to wait.
                    frameAvailable = true;
                    frameNotification = false;
                }
                else {
                    // no event to process, waiting...
                    lock.wait();
                }
            }
            if (frameAvailable) {
                processFrame();
            }
        }
    }

    /**
     * Callback from SurfaceTexture.OnFrameAvailableListener. Is called each time a new
     * camera frame for the SurfaceTexture is available.
     */
    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        synchronized (lock) {
            frameNotification = true;
            lock.notify();
        }
    }

    /**
      * Process button clicked from main UI.
      */
    public void topBtnClicked(boolean modeInput) {
      synchronized (lock) {
        modeChanged = true;
        newModeInput = modeInput;
        lock.notify();
      }
    }

    /**
      * Process surfaceChanged callback from main UI.
      */
    public void surfaceChanged(int width, int height) {
        synchronized (lock) {
            newWidth = width;
            newHeight = height;
            isSurfaceChanged = true;
            lock.notify();
        }
    }

    /**
     * Stop the thread.
     */
    public void terminate() {
        synchronized (lock) {
            running = false;
            lock.notify();
        }
    }

    private void processFrame() {
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
        imgProcThread.update(ogWrapper);

        // swap GL display buffers
        windowSurface.swapBuffers();
    }

    /**
     * release the camera texture and the window surface.
     */
    private void releaseGL() {
        Log.i(CamActivity.TAG, "releasing camera frame texture");
        GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
        int[] t = { camTextureId };
        GLES20.glDeleteTextures(1, t, 0);

        camTexture.release();
        camTexture = null;

        Log.i(CamActivity.TAG, "releasing window surface");
        windowSurface.release();
        windowSurface = null;
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
     * Open the camera device.
     */
    private Camera startCam() {
        Log.i(CamActivity.TAG, "starting camera");

        // get camera information
        Camera.CameraInfo camInfo = new Camera.CameraInfo();

        Camera cam = null;

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
            Log.i(CamActivity.TAG, "No back-facing camera found; opening default");
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

        Log.i(CamActivity.TAG, "camera preview frame size is " + camPreviewFrameSize.width + "x" + camPreviewFrameSize.height);
        Log.i(CamActivity.TAG, "camera preview fps range is " + fps[0] / 1000.0f + " - " + fps[1] / 1000.0f);

        return cam;
    }

    /**
     * Stop camera preview and close camera device.
     */
    private void stopCam(Camera cam) {
        cam.stopPreview();
        cam.setPreviewCallback(null);
        cam.release();

        Log.i(CamActivity.TAG, "stopped camera");
    }
}
