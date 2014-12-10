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
 * Project template for native image processing via JNI on Android.
 * 
 * MainActivity implements the simple user interaction: Touching the input image will run
 * the native image processing function (grayscale conversion).
 */
public class MainActivity extends Activity /* implements SurfaceHolder.Callback */ {
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
	
	private int inputW;
	private int inputH;
	private int outputW;
	private int outputH;
	
	private int[] inputPixels;			// pixel data of <origImgBm> as ARGB int values
	private ByteBuffer outputPixels;	// output pixel data as ARGB bytes values
	
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        
        // set the test images
        initTestImagesMap();
        
		// set the content view to main view with image view and buttons
        initUI();
        	
		// the output pixel buffer will be directly delivered by ogWrapper.
		// it is managed on the native side.
		
		// create the native ogles_gpgpu wrapper object
		ogWrapper = new OGJNIWrapper();
		ogWrapper.init();
		
        // load and display the default test image. this will also call ogWrapper.prepare()
        loadAndDisplayTestImage(defaultTestImgId);
		
		// set the "on click" event listener for the image view
		imgView.setOnClickListener(new ImageViewClickListener());
    }


	@Override
    protected void onDestroy() {
    	ogWrapper.cleanup();
    	
    	super.onDestroy();
    }
	
    private void initTestImagesMap() {
    	defaultTestImgId = R.drawable.leafs_1024x768;
        testImages.put(Integer.valueOf(defaultTestImgId), "leafs_1024x768");
        testImages.put(Integer.valueOf(R.drawable.building_2048x1536), "building_2048x1536");
	}
    
    private void initUI() {
        setContentView(R.layout.activity_main);
		imgView = (ImageView)findViewById(R.id.img_view);
		
		btnGroup = (LinearLayout)findViewById(R.id.btn_group);
		
		for (Entry<Integer, String> entry : testImages.entrySet()) {
			Button btn = new Button(this);
			btn.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
			btn.setText(entry.getValue());
			btn.setTag(entry.getKey());
			
			if (entry.getKey().intValue() == defaultTestImgId) {
				btn.setPressed(true);
				selectedBtn = btn;
			}
			
	        btn.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					selectedBtn.setPressed(false);
					
					Button b = (Button)v;
					b.setPressed(true);
					selectedBtn = b;
					
					int resId = ((Integer)(v.getTag())).intValue();
					loadAndDisplayTestImage(resId);
				}
			});
	        
	        btnGroup.addView(btn);
		}
    }
    
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
    
    private void prepareOG() {
		// prepare for the input image size
    	ogWrapper.prepare(inputW, inputH);
    	outputW = ogWrapper.getOutputFrameW();
    	outputH = ogWrapper.getOutputFrameH();
    }
    
	private class ImageViewClickListener implements View.OnClickListener {
		private boolean filtered = false;
		
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
