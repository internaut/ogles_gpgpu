package ogles_gpgpu.examples.ogvideoprocdroid;

import ogles_gpgpu.OGJNIWrapper;

import com.android.grafika.gles.EglCore;
import com.android.grafika.gles.WindowSurface;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class CamActivity extends Activity implements SurfaceHolder.Callback {
	private final static String TAG = "CamActivity";

	// The holder for our SurfaceView.  The Surface can outlive the Activity (e.g. when
    // the screen is turned off and back on with the power button).
    //
    // This becomes non-null after the surfaceCreated() callback is called, and gets set
    // to null when surfaceDestroyed() is called.
    private static SurfaceHolder surfaceHolder;

    private EglCore eglCore;
    
    private OGJNIWrapper ogWrapper;
    
    private WindowSurface windowSurface;
    

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_cam);
        
        ogWrapper = new OGJNIWrapper();

        SurfaceView sv = (SurfaceView) findViewById(R.id.surface_view);
        SurfaceHolder sh = sv.getHolder();
        sh.addCallback(this);
    }

    @Override
    protected void onResume() {
        Log.i(TAG, "onResume");
        super.onResume();
    }

    @Override
    protected void onPause() {
        Log.i(TAG, "onPause");
        super.onPause();
    }

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		Log.i(TAG, "surface created");
		
		surfaceHolder = holder;
		
		eglCore = new EglCore();
		ogWrapper.init(true, false);
		
		windowSurface = new WindowSurface(eglCore, holder.getSurface(), false);
		windowSurface.makeCurrent();
	}
	
	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		Log.i(TAG, "surface changed to size " + width + "x" + height);
		
		ogWrapper.prepare(width, height);
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		Log.i(TAG, "surface destroyed");
		
		surfaceHolder = null;
		
		eglCore.release();
	}

}
