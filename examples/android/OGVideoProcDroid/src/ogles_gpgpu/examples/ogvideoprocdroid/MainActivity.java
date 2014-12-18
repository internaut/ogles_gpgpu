package ogles_gpgpu.examples.ogvideoprocdroid;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import ogles_gpgpu.examples.ogvideoprocdroid.R;
import android.app.Activity;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.os.Bundle;
import android.util.Log;
import android.util.Size;
import android.view.Surface;
import android.view.TextureView;


public class MainActivity extends Activity {
	private final static String TAG = "MainActivity";

	private int numAvailablePxFmt = 0;
	private int selectedPxFmt = 0; 
	
	private CameraManager camMngr;
	private String camId;
	private CameraDevice.StateCallback camStateCallback;
	private CameraDevice camDevice;
	private CameraCaptureSession camCapSession;
	
	private Semaphore camLock = new Semaphore(1);
	
	private AutoFitTextureView textureView;
	private TextureView.SurfaceTextureListener textureViewListener;
	
	private Size previewFrameSize;
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        setContentView(R.layout.activity_main);
        
        textureView = (AutoFitTextureView)findViewById(R.id.texture_view);

        textureViewListener = new TextureViewSurfaceListener();
    }

    @Override
    public void onResume() {
        super.onResume();

        // When the screen is turned off and turned back on, the SurfaceTexture is already
        // available, and "onSurfaceTextureAvailable" will not be called. In that case, we can open
        // a camera and start preview from here (otherwise, we wait until the surface is ready in
        // the SurfaceTextureListener).
        if (textureView.isAvailable()) {
        	initCam(textureView.getWidth(), textureView.getHeight());
        	startCam();
        } else {
        	textureView.setSurfaceTextureListener(textureViewListener);
        }
    }

    @Override
    public void onPause() {
        stopCam();
        
        super.onPause();
    }
    
    private void initCam(int outputW, int outputH) {
    	Log.i(TAG, "init cam with size " + outputW + "x" + outputH);
    	
    	camMngr = (CameraManager)this.getSystemService(Context.CAMERA_SERVICE);
    	
    	if (camMngr == null) {
    		Log.e(TAG, "CameraManager not available (i.e. camera2 API not available)");
    		
    		return;
    	}
    	
    	try {
    		for (String c : camMngr.getCameraIdList()) {
    			Log.i(TAG, "available camera with id: " + c);
    			CameraCharacteristics camParams = camMngr.getCameraCharacteristics(c);
    			
    			// only use back-facing camera
    			if (camParams.get(CameraCharacteristics.LENS_FACING) == CameraCharacteristics.LENS_FACING_BACK) {
    				Log.i(TAG, "> is back facing");
    				
    				// get camera configuration map
    				StreamConfigurationMap streamConf = camParams.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
    				
    				// get possible image formats
    				int[] possibleFmt = streamConf.getOutputFormats();
    				
    				if (numAvailablePxFmt == 0) {
    					numAvailablePxFmt = possibleFmt.length;
    				}
    				
    				for (int fmt : possibleFmt) {
    					Log.i(TAG, "> cam supports format: 0x" + Integer.toHexString(fmt));
    				}
    				
    				int chosenFmt = possibleFmt[selectedPxFmt];
    				Log.i(TAG, "> trying to use pixel format #" + selectedPxFmt);
    				
//    				// try to select requested image format format
//    				int chosenFmt;
//    				if (streamConf.isOutputSupportedFor(ImageFormat.YUV_420_888)) {
//    					chosenFmt = ImageFormat.YUV_420_888;
//    				} else {
//    					Log.e(TAG, "> YUV_420_888 format not supported");
//    					return;
//    				}
    				
    				// get possible sizes for this image format
    				Size[] possibleFmtSizes = streamConf.getOutputSizes(chosenFmt);
    				for (Size s : possibleFmtSizes) {
    					Log.i(TAG, "> cam format supports size: " + s.getWidth() + "x" + s.getHeight());
    				}
    				
    				// get possible sizes of the surface view
    				Size[] possibleSurfaceSizes = streamConf.getOutputSizes(SurfaceTexture.class);
    				for (Size s : possibleSurfaceSizes) {
    					Log.i(TAG, "> surface supports size: " + s.getWidth() + "x" + s.getHeight());
    				}
    				
    				
    				// choose the best preview frame size
    				Size largestSize = Collections.max(Arrays.asList(possibleFmtSizes), new CompareSizesByArea());
    				previewFrameSize = chooseOptimalSize(possibleSurfaceSizes, outputW, outputH, largestSize);
    				
    				Log.i(TAG, "> chosen preview frame size: " + previewFrameSize.getWidth() + "x" + previewFrameSize.getHeight());
    				
    				// set aspect ratio for the surface view (only landscape orientation is supported)
    				textureView.setAspectRatio(previewFrameSize.getWidth(), previewFrameSize.getHeight());
    				
    				// set camera id
    				camId = c;
    			}
    		}
    	} catch (CameraAccessException e) {
    		e.printStackTrace();
    	}
    }
    
    private void startCam() {
    	try {
            if (!camLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw new RuntimeException("Time out waiting to lock camera opening.");
            }
            
            camStateCallback = new CamStateCallback();
            camMngr.openCamera(camId, camStateCallback, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera opening.", e);
        }
    }
    
    private void stopCam() {
        try {
            camLock.acquire();
            if (null != camCapSession) {
            	camCapSession.close();
            	camCapSession = null;
            }
            
            if (null != camDevice) {
            	camDevice.close();
            	camDevice = null;
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera closing.", e);
        } finally {
        	camLock.release();
        }
    }
    
    private void createCamPreview() {
    	 try {
             SurfaceTexture texture = textureView.getSurfaceTexture();
             assert texture != null;
             
             // We configure the size of default buffer to be the size of camera preview we want.
             texture.setDefaultBufferSize(previewFrameSize.getWidth(), previewFrameSize.getHeight());

             // This is the output Surface we need to start preview.
             Surface previewSurface = new Surface(texture);

             // We set up a CaptureRequest.Builder with the output Surface.
             final CaptureRequest.Builder captureRequestBuilder = camDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
             captureRequestBuilder.addTarget(previewSurface);
             
             camDevice.createCaptureSession(Arrays.asList(previewSurface), new CameraCaptureSession.StateCallback() {
				@Override
				public void onConfigured(CameraCaptureSession session) {
					if (camDevice == null) return;
					
					// When the session is ready, we start displaying the preview.
					camCapSession = session;
                    try {
                        // Auto focus should be continuous for camera preview.
                    	captureRequestBuilder.set(CaptureRequest.CONTROL_AF_MODE,
                                CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
                        
                        // Flash is disabled
                    	captureRequestBuilder.set(CaptureRequest.CONTROL_AE_MODE,
                                CaptureRequest.CONTROL_AE_MODE_ON);

                        // Finally, we start displaying the camera preview.
                    	CaptureRequest capRequest = captureRequestBuilder.build();
                        camCapSession.setRepeatingRequest(capRequest, null, null);
                    } catch (CameraAccessException e) {
                        e.printStackTrace();
                    }
				}
				
				@Override
				public void onConfigureFailed(CameraCaptureSession session) {
					Log.e(TAG, "failed configuring camera capture session");
				}
			}, null);
             
         } catch (CameraAccessException e) {
             e.printStackTrace();
         }
    }
    
    /**
     * Given {@code choices} of {@code Size}s supported by a camera, chooses the smallest one whose
     * width and height are at least as large as the respective requested values, and whose aspect
     * ratio matches with the specified value.
     *
     * @param choices     The list of sizes that the camera supports for the intended output class
     * @param width       The minimum desired width
     * @param height      The minimum desired height
     * @param aspectRatio The aspect ratio
     * @return The optimal {@code Size}, or an arbitrary one if none were big enough
     */
    private static Size chooseOptimalSize(Size[] choices, int width, int height, Size aspectRatio) {
        // Collect the supported resolutions that are at least as big as the preview Surface
        List<Size> bigEnough = new ArrayList<Size>();
        int w = aspectRatio.getWidth();
        int h = aspectRatio.getHeight();
        for (Size option : choices) {
            if (option.getHeight() == option.getWidth() * h / w &&
                    option.getWidth() >= width && option.getHeight() >= height) {
                bigEnough.add(option);
            }
        }

        // Pick the smallest of those, assuming we found any
        if (bigEnough.size() > 0) {
            return Collections.min(bigEnough, new CompareSizesByArea());
        } else {
            Log.e(TAG, "Couldn't find any suitable preview size");
            return choices[0];
        }
    }
    
    /**
     * Compares two {@code Size}s based on their areas.
     */
    static class CompareSizesByArea implements Comparator<Size> {

        @Override
        public int compare(Size lhs, Size rhs) {
            // We cast here to ensure the multiplications won't overflow
            return Long.signum((long) lhs.getWidth() * lhs.getHeight() -
                    (long) rhs.getWidth() * rhs.getHeight());
        }

    }
    
    private class TextureViewSurfaceListener implements TextureView.SurfaceTextureListener {
		@Override
		public void onSurfaceTextureAvailable(SurfaceTexture surface,
				int width, int height) {
			initCam(width, height);
			startCam();
		}

		@Override
		public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {

		}

		@Override
		public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
			return true;
		}

		@Override
		public void onSurfaceTextureUpdated(SurfaceTexture surface) {
			
		}
    }
    
    private class CamStateCallback extends CameraDevice.StateCallback {
		@Override
		public void onOpened(CameraDevice camera) {
			// This method is called when the camera is opened.  We start camera preview here.
			Log.i(TAG, "camera device opened");
            camLock.release();
            camDevice = camera;
            createCamPreview();
		}

		@Override
		public void onDisconnected(CameraDevice camera) {
			Log.i(TAG, "camera device disconnected");
			camLock.release();
			camDevice.close();
			camDevice = null;
		}

		@Override
		public void onError(CameraDevice camera, int error) {
			Log.i(TAG, "camera device error occured, will close camera device");
			
			camLock.release();
			camDevice.close();
			camDevice = null;
			
			// try again!
			if (selectedPxFmt < numAvailablePxFmt - 1) {
				selectedPxFmt++;
				initCam(previewFrameSize.getWidth(), previewFrameSize.getHeight());
				startCam();
			}
		}
    	
    }
}
