//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

package ogles_gpgpu.examples.ogvideoprocdroid;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.util.Arrays;

import ogles_gpgpu.OGJNIWrapper;

import android.app.Activity;
import android.util.Log;

/**
 * Image processing thread for asynchronous processing on the CPU side.
 * When update() is called on this thread, it will take the current output
 * of ogles_gpgpu (the current GPU-processed frame) and run a histogram
 * calculation on it. The result is send to the HistView instance.
 * This is just to show how GPU-processed frames can be acquired from
 * ogles_gpgpu and can then be further processed on the CPU side, for
 * example when it is not possible to implement an algorithm as OpenGL
 * shader. See CamActivity class for more information.
 *
 * All public methods except 'run' called from other threads.
 */
class CPUImgProcThread extends Thread {
    /**
      * Output data from ogles_gpgpu library. Pixel data as ARGB bytes values.
      */
    private ByteBuffer imgData = null;

    /**
     * 'imgData' access synchronization.
     */
    private final Object lock = new Object();

    /**
     * Data availability flag. Variable is set in 'update' call and mainly
     * needed to check that wakeup is not spurious.
     */
    private boolean dataReady = false;

    /**
      * Current state of the thread.
      */
    private boolean running = true;

    /**
      * Output histogram.
      */
    private float[] outputHist = new float[256];

    /**
     * Histogram view.
     */
    private HistView histView = null;

    /**
     * Activity that have started this thread.
     */
    private Activity mainActivity;

    public CPUImgProcThread(HistView view, Activity activity) {
        histView = view;
        mainActivity = activity;
    }

    /**
     * Thread run loop. Will run until terminate() is called.
     */
    @Override
    public void run() {
        Log.i(CamActivity.TAG, "starting thread CPUImgProcThread");

        try {
            mainLoop();
        }
        catch (Exception e) {
            e.printStackTrace();
            Log.e(CamActivity.TAG, "CPUImgProcThread mainLoop");
        }

        imgData = null;

        Log.i(CamActivity.TAG, "stopped thread CPUImgProcThread");
    }

    private void mainLoop() throws Exception {
        while (running) {
            synchronized (lock) {
                lock.wait();
                if (!dataReady) {
                    continue;
                }
                // calculate the histogram from the image data
                calcHist(imgData);

                // send it to histView, where a copy will be made
                histView.getHistCopy(outputHist);

                // invalidate the histView, so it will be redrawn.
                // this *must* be called on the UI thread
                mainActivity.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        histView.invalidate();
                    }
                });
                imgData = null;
                dataReady = false;
            }
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

    /**
     * Notify about new image data from ogles_gpgpu. The "imgData" ByteBuffer is
     * only a reference to the actual image data on the native side! It is
     * only valid until the next call to "getOutputPixels()"!
     */
    public void update(OGJNIWrapper ogWrapper) {
        if (!running) {
            return;
        }
        if (dataReady) {
            return; // TODO; Should be `if (!tryLock) { return; }`
        }
        synchronized (lock) {
            // get reference to current result image data
            imgData = ogWrapper.getOutputPixels();
            imgData.rewind();

            dataReady = true;
            lock.notify();
        }
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
}
