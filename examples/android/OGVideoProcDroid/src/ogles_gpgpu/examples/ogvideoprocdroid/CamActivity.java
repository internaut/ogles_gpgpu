//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

package ogles_gpgpu.examples.ogvideoprocdroid;

import android.app.Activity;
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
 * ogles_gpgpu library for image processing (see CameraProcThread class). Some simple filters are applied via
 * GPGPU to the camera frame (downscaling and grayscale conversion plus Gauss
 * blurring). Afterwards, the image is read back to main memory as a ByteBuffer and
 * is be further processed on the CPU side - a live grayscale histogram is
 * calculated (see CPUImgProcThread class). This is just to show how GPU-processed frames can be acquired from
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
    implements SurfaceHolder.Callback {
    public final static String TAG = "CamActivity";

    /**
     * SurfaceHolder instance which is set during surfaceCreated() and is destroyed and
     * set to null in surfaceDestroyed(). This is the case when the application is send
     * to background. It is *not* the case, when the power-on/off button is pressed and
     * the screen is switched off. The surfaceHolder object will remain then.
     */
    private SurfaceHolder surfaceHolder;

    /**
     * Thread for asynchronous image processing on the CPU side.
     */
    private CPUImgProcThread imgProcThread;

    /**
     * Thread for processing camera frames.
     */
    private CameraProcThread cameraProcThread;

    /**
     * Activity creation - set content view.
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_cam);
    }

    /**
     * Activity resume:
     *  - create and start image processing threads if surface already exists
     *  - if surface not exists threads will be started when surfaceCreated
     * callback will be received
     */
    @Override
    protected void onResume() {
        Log.i(TAG, "onResume");
        super.onResume();

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
            startThreads();
        } else {
            Log.i(TAG, "surface holder will be set automatically");
            // surfaceCreated() will be called automatically later when the
            // SurfaceView appears
        }
    }

    /**
     * Activity pause - stops all threads.
     */
    @Override
    protected void onPause() {
        Log.i(TAG, "onPause");
        try {
            stopThreads();
        }
        catch (InterruptedException e) {
            e.printStackTrace();
            Log.e(TAG, "Exception catched");
        }
        super.onPause();
    }

    /**
     * Button callback for the buttons on top ("show input" / "show output").
     * @param btn button that was clicked
     */
    public void topBtnClicked(View btn) {
        boolean modeInput = (btn.getId() == R.id.btn_show_input);
        cameraProcThread.topBtnClicked(modeInput);
    }

    /**
     * Surface creation callback. Will set the surface holder and start threads.
     */
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.i(TAG, "surface created");
        surfaceHolder = holder;
        startThreads();
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
        cameraProcThread.surfaceChanged(width, height);
    }

    /**
     * Surface destruction callback. Is called when the SurfaceView disappears because
     * the application goes to the background. Note that onPause will be called
     * before surfaceDestroyed so threads must be in terminated states here.
     */
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.i(TAG, "surface destroyed");

        // reset the surface holder because it will not be valid anymore
        surfaceHolder = null;

        if (cameraProcThread != null) {
            throw new RuntimeException("Thread is not stopped");
        }
    }

    /**
     * Start working threads
     */
    private void startThreads() {
        HistView histView = (HistView)findViewById(R.id.hist_view);

        // create and start image processing thread for further image processing
        // on the CPU side after the camera frame was initially processed on the GPU
        imgProcThread = new CPUImgProcThread(histView, this);
        imgProcThread.start();
        imgProcThread.setName("CPUImageProcThread");

        cameraProcThread = new CameraProcThread(imgProcThread, surfaceHolder);
        cameraProcThread.start();
        cameraProcThread.setName("CameraProcThread");
    }

    /**
     * Stop working threads
     */
    private void stopThreads() throws InterruptedException {
        // First we should stop cameraProcThread since it
        // may send update to imgProcThread
        cameraProcThread.terminate();
        cameraProcThread.join();
        cameraProcThread = null;

        imgProcThread.terminate();
        imgProcThread.join();
        imgProcThread = null;
    }
}
