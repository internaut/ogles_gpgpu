package ogles_gpgpu;


import java.nio.ByteBuffer;


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
    public native ByteBuffer getOutputPixels();
}