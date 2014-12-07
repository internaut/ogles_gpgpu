package ogles_gpgpu.examples.ogstillimagedroid;

import java.nio.ByteBuffer;

import ogles_gpgpu.OGJNIWrapper;
import ogles_gpgpu.examples.ogstillimagedroid.R;
import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageView;

/**
 * Project template for native image processing via JNI on Android.
 * 
 * MainActivity implements the simple user interaction: Touching the input image will run
 * the native image processing function (grayscale conversion).
 */
public class MainActivity extends Activity /* implements SurfaceHolder.Callback */ {
	private final String TAG = this.getClass().getSimpleName();
	
	private OGJNIWrapper ogWrapper;	// ogles_gpgpu native interface object

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
        
		// set the content view to main view with image view
        setContentView(R.layout.activity_main);
		imgView = (ImageView)findViewById(R.id.img_view);
		
		// create a bitmap of the input image
		origImgBm = BitmapFactory.decodeResource(getResources(), R.drawable.leafs_1024x786);
		inputW = origImgBm.getWidth();
		inputH = origImgBm.getHeight();
		
		// set it as drawable for the image view
		origImgBmDr = new BitmapDrawable(getResources(), origImgBm);
		imgView.setImageDrawable(origImgBmDr);
		
		// set up the input pixel buffer
		inputPixels = new int[inputW * inputH];
		
		// the output pixel buffer will be directly delivered by ogWrapper.
		// it is managed on the native side.
		
		// create the native ogles_gpgpu wrapper object
		ogWrapper = new OGJNIWrapper();
		ogWrapper.init();
		
		// prepare for the input image size
    	ogWrapper.prepare(inputW, inputH);
    	outputW = ogWrapper.getOutputFrameW();
    	outputH = ogWrapper.getOutputFrameH();
		
		// set the "on click" event listener
		imgView.setOnClickListener(new ImageViewClickListener(origImgBm));
    }
    
    @Override
    protected void onDestroy() {
    	ogWrapper.cleanup();
    	
    	super.onDestroy();
    }
    
	private class ImageViewClickListener implements View.OnClickListener {
		private boolean filtered;
		private Bitmap origBitmap;
		
		public ImageViewClickListener(Bitmap bitmap) {			
			// get the pixel data as ARGB int values
			bitmap.getPixels(inputPixels, 0, inputW, 0, 0, inputW, inputH);
			
			filtered = false;
			origBitmap = bitmap;			
		}
		
		@Override
		public void onClick(View v) {
			ImageView imgView = (ImageView)v;
			Bitmap processedBm;

			if (!filtered) {
				// set input pixels
				ogWrapper.setInputPixels(inputPixels);
				
				// create a new empty bitmap for the result
				processedBm = Bitmap.createBitmap(
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
				
				filtered = true;
			} else {
				processedBm = origBitmap;
				filtered = false;
			}

			// update the image view
			BitmapDrawable filteredBitmapDrawable = new BitmapDrawable(getResources(), processedBm);			
			imgView.setImageDrawable(filteredBitmapDrawable);
		}
	}
}
