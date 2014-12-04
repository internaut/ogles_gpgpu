package ogles_gpgpu.examples.ogstillimagedroid;

import ogles_gpgpu.OGJNIWrapper;
import ogles_gpgpu.examples.ogstillimagedroid.R;
import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;

/**
 * Project template for native image processing via JNI on Android.
 * 
 * MainActivity implements the simple user interaction: Touching the input image will run
 * the native image processing function (grayscale conversion).
 */
public class MainActivity extends Activity {
	private final String TAG = this.getClass().getSimpleName();
	
	private OGJNIWrapper ogWrapper;
	private ImageView imgView;
	private Bitmap origImgBm;
	private BitmapDrawable origImgBmDr;
	
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
		imgView = (ImageView)findViewById(R.id.img_view);
		
		// create a bitmap of the input image
		origImgBm = BitmapFactory.decodeResource(getResources(), R.drawable.leafs_1024x786);
		
		// set it as drawable for the image view
		origImgBmDr = new BitmapDrawable(getResources(), origImgBm);
		imgView.setImageDrawable(origImgBmDr);
		
		// set the "on click" event listener
		imgView.setOnClickListener(new ImageViewClickListener(origImgBm));
		
		// create ogles_gpgpu native interface
		ogWrapper = new OGJNIWrapper();	// native interface
		ogWrapper.init();
    }
    
    @Override
    protected void onDestroy() {
    	ogWrapper.cleanup();
    	
    	super.onDestroy();
    }

    
	private class ImageViewClickListener implements View.OnClickListener {
		private boolean filtered;
		private Bitmap bitmap;
		private int bitmapW;
		private int bitmapH;
		private int bitmapData[];	// pixel data if <bitmap> as ARGB int values
		
		public ImageViewClickListener(Bitmap bitmap) {			
			bitmapW = bitmap.getWidth();
			bitmapH = bitmap.getHeight();
			
			// get the pixel data as ARGB int values
			bitmapData = new int[bitmapW * bitmapH];
			bitmap.getPixels(bitmapData, 0, bitmapW, 0, 0, bitmapW, bitmapH);
			
			this.filtered = false;
			this.bitmap = bitmap;			
		}
		
		@Override
		public void onClick(View v) {
			ImageView imgView = (ImageView)v;
			Bitmap processedBm;

			if (!filtered) {
				// create a new empty bitmap for the result
				processedBm = Bitmap.createBitmap(
						bitmapW,
						bitmapH,
						Bitmap.Config.ARGB_8888);
				
				// run the native image processing function. data will be modified in-place
				Log.i(TAG, "will run native image processing function...");
//				imgProc.grayscale(bitmapData);
				
				// set the processed image data for the result bitmap
				processedBm.setPixels(bitmapData, 0, bitmapW, 0, 0, bitmapW, bitmapH);
				
				filtered = true;
			} else {
				processedBm = bitmap;
				filtered = false;
			}

			// update the image view
			BitmapDrawable filteredBitmapDrawable = new BitmapDrawable(getResources(), processedBm);			
			imgView.setImageDrawable(filteredBitmapDrawable);
		}
		
	}
}
