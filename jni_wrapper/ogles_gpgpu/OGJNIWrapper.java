package ogles_gpgpu;


import android.opengl.GLSurfaceView;
import android.util.Log;

import javax.microedition.khronos.egl.*;
import javax.microedition.khronos.opengles.GL10;


public class OGJNIWrapper {	
	static {
		System.loadLibrary("og_jni_wrapper");
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