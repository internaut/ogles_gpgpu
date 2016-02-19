//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

package ogles_gpgpu.examples.ogstillimagedroid;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map.Entry;

import ogles_gpgpu.OGJNIWrapper;
import ogles_gpgpu.examples.ogstillimagedroid.R;
import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;

/**
 * GPGPU based image processing of still images with ogles_gpgpu.
 *
 * MainActivity implements the ogles_gpgpu setup and the simple user interaction:
 * Touching the input image will run the GPGPU image processing function. This
 * will be executed by calling native functions to the ogles_gpgpu library via JNI.
 * See the wrapper class OGJNIWrapper and the C++ sources in the "jni" folder.
 *
 * Known issues:
 * - the application crashes when switching the screen off and on again. this is
 *   because ogWrapper.init() will try to initialize EGL again in "onResume", but
 *   this only works when the application was send to background and resumed again
 */
public class MainActivity extends Activity { /* implements SurfaceHolder.Callback */
    private final String TAG = this.getClass().getSimpleName();

    private HashMap<Integer, String> testImages = new HashMap<Integer, String>();

    private OGJNIWrapper ogWrapper;	// ogles_gpgpu native interface object

    private Button selectedBtn = null;
    private int defaultTestImgId;
    private int selectedTestImgId = -1;
    private LinearLayout btnGroup;
    private ImageView imgView;
    private Bitmap origImgBm;
    private BitmapDrawable origImgBmDr;

    private int inputW;					// input image width
    private int inputH;					// input image height
    private int outputW;				// output image width
    private int outputH;				// output image height

    private int[] inputPixels;			// pixel data of <origImgBm> as ARGB int values
    private ByteBuffer outputPixels;	// output pixel data as ARGB bytes values


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // set the test images
        initTestImagesMap();

        // set the content view to main view with image view and buttons
        initUI();

        // set the "on click" event listener for the image view
        imgView.setOnClickListener(new ImageViewClickListener());
    }

    @Override
    protected void onResume() {
        Log.i(TAG, "onResume");

        super.onResume();

        // create the native ogles_gpgpu wrapper object
        // the output pixel buffer will be directly delivered by ogWrapper.
        // it is managed on the native side.
        ogWrapper = new OGJNIWrapper();
        // use platform opt., init EGL on native side, do NOT create a render display
        // Note that "init EGL" will crash the application here when the screen was
        // turned off and on again. However, background/foreground switching works.
        ogWrapper.init(true, true, false);

        // load and display the default test image. this will also call ogWrapper.prepare()
        loadAndDisplayTestImage(defaultTestImgId);
    }

    @Override
    protected void onPause() {
        Log.i(TAG, "onPause");

        ogWrapper.cleanup();
        ogWrapper = null;

        super.onPause();
    }

    /**
     * Initialize test images map.
     */
    private void initTestImagesMap() {
        defaultTestImgId = R.drawable.leafs_1024x768;
        testImages.put(Integer.valueOf(defaultTestImgId), "leafs_1024x768");
        testImages.put(Integer.valueOf(R.drawable.building_2048x1536), "building_2048x1536");
    }

    /**
     * Initialize the UI.
     */
    private void initUI() {
        setContentView(R.layout.activity_main);
        imgView = (ImageView)findViewById(R.id.img_view);

        btnGroup = (LinearLayout)findViewById(R.id.btn_group);

        // set the buttons to switch between test images
        for (Entry<Integer, String> entry : testImages.entrySet()) {
            // create button
            Button btn = new Button(this);

            // customize button
            btn.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
            btn.setText(entry.getValue());
            btn.setTag(entry.getKey());

            if (entry.getKey().intValue() == defaultTestImgId) {
                btn.setPressed(true);
                selectedBtn = btn;
            }

            // set listener action
            btn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    selectedBtn.setPressed(false);

                    Button b = (Button)v;
                    b.setPressed(true);
                    selectedBtn = b;

                    // get the resource id of the test image, load and display it
                    int resId = ((Integer)(v.getTag())).intValue();
                    loadAndDisplayTestImage(resId);
                }
            });

            btnGroup.addView(btn);
        }
    }

    /**
     * Load and display the test image <resId>.
     * @param resId test image
     */
    private void loadAndDisplayTestImage(int resId) {
        if (selectedTestImgId == resId) return;	// no change

        // create a bitmap of the input image
        origImgBm = BitmapFactory.decodeResource(getResources(), resId);
        inputW = origImgBm.getWidth();
        inputH = origImgBm.getHeight();

        // set up the new input pixel buffer
        inputPixels = new int[inputW * inputH];

        // get the pixel data as ARGB int values
        origImgBm.getPixels(inputPixels, 0, inputW, 0, 0, inputW, inputH);

        // prepare ogles_gpgpu for the input image of size <inputW>x<inputH>
        prepareOG();

        // set it as drawable for the image view
        origImgBmDr = new BitmapDrawable(getResources(), origImgBm);
        imgView.setImageDrawable(origImgBmDr);

        // update the selected test image id
        selectedTestImgId = resId;
    }

    /**
     * Prepare ogles_gpgpu for the incoming image size
     */
    private void prepareOG() {
        // prepare for the input image size
        ogWrapper.prepare(inputW, inputH, true);
        outputW = ogWrapper.getOutputFrameW();
        outputH = ogWrapper.getOutputFrameH();

        // unfortunately, the first call to "process" will always create an empty result
        // when using Android platform optimizations.
        // the following is a workaround for this: make a fake first "process" call
        ogWrapper.setInputPixels(inputPixels);
        ogWrapper.process();
        outputPixels = ogWrapper.getOutputPixels();
        outputPixels = null;
    }

    /**
     * Image view "onClick" listener class. Sends the test image to ogles_gpgpu,
     * starts the GPGPU image processing tasks and reads back the result image.
     */
    private class ImageViewClickListener implements View.OnClickListener {
        private boolean filtered = false;	// current state

        /**
         * Callback for "click" action.
         */
        @Override
        public void onClick(View v) {
            ImageView imgView = (ImageView)v;
            BitmapDrawable dispDrawable;

            if (!filtered) {
                // set input pixels
                ogWrapper.setInputPixels(inputPixels);

                // create a new empty bitmap for the result
                Bitmap processedBm = Bitmap.createBitmap(
                                         outputW,
                                         outputH,
                                         Bitmap.Config.ARGB_8888);

                // run the native image processing function. data will be modified in-place
                Log.i(TAG, "will run native image processing function...");
                ogWrapper.process();

                // get the processed image data
                outputPixels = ogWrapper.getOutputPixels();

                // set it to the bitmap
                outputPixels.rewind();
                processedBm.copyPixelsFromBuffer(outputPixels);
                outputPixels.rewind();

                // create drawable from it
                dispDrawable = new BitmapDrawable(getResources(), processedBm);

                // print time measurments
                double[] timings = ogWrapper.getTimeMeasurements();
                if (timings != null) {
                    for (int i = 0; i < timings.length; ++i) {
                        Log.i(TAG, "measurement #" + i + ": " + timings[i]);
                    }
                }

                filtered = true;
            } else {
                dispDrawable = origImgBmDr;
                filtered = false;
            }

            // update the image view
            imgView.setImageDrawable(dispDrawable);
        }
    }
}
