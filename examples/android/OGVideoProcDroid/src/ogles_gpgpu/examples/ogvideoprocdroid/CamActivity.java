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
import android.os.Looper;
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
    
    private CPUImgProcThread imgProcThread;
//    private Object imgProcThreadLock = new Object();
    
//    private int procOutputW;		// processing output width
//    private int procOutputH;		// processing output width
//    private ByteBuffer procOutput;	// output pixel data as ARGB bytes values
    
    private HistView histView;
    
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
        
        imgProcThread = new CPUImgProcThread();
        
        ogWrapper = new OGJNIWrapper();
        
        histView = (HistView)findViewById(R.id.hist_view);

        // get surface holder and set callback
        SurfaceView sv = (SurfaceView)findViewById(R.id.surface_view);
        sv.getHolder().addCallback(this);
    }

    @Override
    protected void onResume() {
        Log.i(TAG, "onResume");
        super.onResume();
        
        imgProcThread.start();
        startCam();
    }

    @Override
    protected void onPause() {
        Log.i(TAG, "onPause");
        super.onPause();
        
        stopCam();
        
        imgProcThread.terminate();
        
        try {
			imgProcThread.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
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

//		procOutputW = ogWrapper.getOutputFrameW();
//		procOutputH = ogWrapper.getOutputFrameH();
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
		
		imgProcThread.update();
		
		// get the processed image data
//		procOutput = ogWrapper.getOutputPixels();
//		procOutput.rewind();
		
//		calcHist(procOutput);
//		printHist(outputHist);

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
	
	private class CPUImgProcThread extends Thread {
		private boolean running = false;
//		private boolean processing = false;
		private ByteBuffer imgData;
	    private float[] outputHist = new float[256];		// output histogram

		
		@Override
		public void run() {
			Log.i(TAG, "starting thread CPUImgProcThread");
			
			running = true;
			
			while (running) {
				if (imgData != null) {
					calcHist(imgData);
					
					histView.getHistCopy(outputHist);
					
					runOnUiThread(new Runnable() {
						@Override
						public void run() {
							histView.invalidate();
						}
					});
					
					// reset
					imgData = null;
				}
			}
			
			Log.i(TAG, "stopped thread CPUImgProcThread");
		}
		
		public void terminate() {
			running = false;
		}
		
		public void update() {
			if (imgData != null) return;	// image data already in use right now
			
			imgData = ogWrapper.getOutputPixels();
			imgData.rewind();
		}
	
		private void calcHist(ByteBuffer pxData) {
			// use an IntBuffer for image data access.
			// each pixel is a int value with RGBA data 
			IntBuffer intData = pxData.asIntBuffer();
			
			// reset histogram to zeros
			Arrays.fill(outputHist, 0.0f);
			
			// count values and store them in absolute histogram
			while (intData.hasRemaining()) {
				int grayVal = (intData.get() >> 8) & 0x000000FF;	// get the "B" channel
				
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
		
		private void printHist(float[] h) {
			Log.i(TAG, "output histogram values: " + Arrays.toString(outputHist));
		}
	}
}
