package ogles_gpgpu;


import android.opengl.GLSurfaceView;
import android.util.Log;

import javax.microedition.khronos.egl.*;
import javax.microedition.khronos.opengles.GL10;


public class OGJNIWrapper implements GLSurfaceView.Renderer {
//	private final String TAG = this.getClass().getSimpleName();
	
	static {
		System.loadLibrary("og_jni_wrapper");
	}
	
	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		init();
	}
	
	@Override
	public void onSurfaceChanged(GL10 gl, int width, int height) {
		prepare(width, height);
	}
	
	@Override
	public void onDrawFrame(GL10 gl) {
		process();
	}

    
    public native int getOutputFrameW();
    public native int getOutputFrameH();
    
    public native void init();
    
    public native void cleanup();
    
    public native void prepare(int inW, int inH);
    
    /**
     * @param pixels    pixel data with ARGB integers
     */
    public native void setInputPixels(int[] pixels);
    
    public native void process();

    /**
     * @return pixel data with ARGB integers
     */
    public native int[] getOutputPixels();
}