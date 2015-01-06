package ogles_gpgpu.examples.ogvideoprocdroid;

import java.io.IOException;

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

public class CamActivity extends Activity implements SurfaceHolder.Callback, SurfaceTexture.OnFrameAvailableListener {
	private final static String TAG = "CamActivity";
	
	private final static int CAM_FPS = 30;
	private final static int CAM_FRAME_TEXTURE_TARGET = GLES11Ext.GL_TEXTURE_EXTERNAL_OES;

	// The holder for our SurfaceView.  The Surface can outlive the Activity (e.g. when
    // the screen is turned off and back on with the power button).
    //
    // This becomes non-null after the surfaceCreated() callback is called, and gets set
    // to null when surfaceDestroyed() is called.
    private static SurfaceHolder surfaceHolder;

    private EglCore eglCore;
    
    private OGJNIWrapper ogWrapper;
    
    private WindowSurface windowSurface;
    
    private Camera cam;
    private Size camPreviewFrameSize;
    private Object camLock = new Object();
    private SurfaceTexture camTexture;
    private int camTextureId;
    
    
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
        
        startCam();
    }

    @Override
    protected void onPause() {
        Log.i(TAG, "onPause");
        super.onPause();
        
        stopCam();
    }

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		Log.i(TAG, "surface created");
		
		surfaceHolder = holder;
		
		eglCore = new EglCore();
		ogWrapper.init(true, false, true);
		
		windowSurface = new WindowSurface(eglCore, holder.getSurface(), false);
		windowSurface.makeCurrent();
		
		Log.i(TAG, "opening camera device");
		startCam();
		
		camTextureId = createGLTexture();
		camTexture = new SurfaceTexture(camTextureId);
		camTexture.setOnFrameAvailableListener(this);
		
		Log.i(TAG, "created camera frame texture with id " + camTextureId);
		
		Log.i(TAG, "starting camera preview");
		
		try {
			cam.setPreviewTexture(camTexture);
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		cam.startPreview();
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		Log.i(TAG, "surface changed to size " + width + "x" + height);
		
		ogWrapper.setRenderDisp(width, height, 2 /* = RenderOrientationFlipped */);
		ogWrapper.prepare(camPreviewFrameSize.width, camPreviewFrameSize.height, false);
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		Log.i(TAG, "surface destroyed");
		
		surfaceHolder = null;
		
		releaseGL();
		
		eglCore.release();
	}
	
	@Override
	public void onFrameAvailable(SurfaceTexture surfaceTexture) {
		GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
		camTexture.updateTexImage();
		
//		Log.i(TAG, "updated camera frame texture");
		
		ogWrapper.setInputTexture(camTextureId);
		ogWrapper.process();
		ogWrapper.renderOutput();
		
		windowSurface.swapBuffers();
	}

	private void releaseGL() {
		if (windowSurface != null) {
			windowSurface.release();
			windowSurface = null;
		}
		
		eglCore.makeNothingCurrent();
	}

	private void startCam() {
		if (cam != null) return;
		
		Log.i(TAG, "starting camera");
		
		synchronized (camLock) {
			Camera.CameraInfo camInfo = new Camera.CameraInfo();
			
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
	
	private void stopCam() {
		if (cam == null) return;
		
		synchronized (camLock) {
			cam.stopPreview();
			cam.release();
			cam = null;
		}
		
		Log.i(TAG, "stopped camera");
	}
	
	private int createGLTexture() {
		GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
		
		int[] textures = new int[1];
        GLES20.glGenTextures(1, textures, 0);

        int texId = textures[0];
        
        if (texId <= 0) {
        	throw new RuntimeException("invalid GL texture id generated");
        }
        
        GLES20.glBindTexture(CAM_FRAME_TEXTURE_TARGET, texId);

        GLES20.glTexParameterf(CAM_FRAME_TEXTURE_TARGET, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameterf(CAM_FRAME_TEXTURE_TARGET, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(CAM_FRAME_TEXTURE_TARGET, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(CAM_FRAME_TEXTURE_TARGET, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);

        return texId;
	}
}
