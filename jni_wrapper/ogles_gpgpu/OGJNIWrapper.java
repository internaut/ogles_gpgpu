package ogles_gpgpu;

public class OGJNIWrapper {
    public native void init();
    
    public native void cleanup();
    
    public native void prepare(int inW, int inH);
    
    /**
     * @param pixels    pixel data with ARGB integers
     */
    public native void setInputPixels(int[] pixels);

    /**
     * @return pixel data with ARGB integers
     */
    public native int[] getOutputPixels();
    
    public native void process();
    
    public native int getOutputFrameW();
    public native int getOutputFrameH();
}