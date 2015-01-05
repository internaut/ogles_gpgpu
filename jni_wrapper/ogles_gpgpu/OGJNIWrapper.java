//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

package ogles_gpgpu;


import java.nio.ByteBuffer;

/**
 * The ogles_gppgu JNI wrapper class. Interface to the native functions for the
 * og_jni_wrapper.so library.
 */
public class OGJNIWrapper {	
	static {
		// load the static library ogles_gpgpu JNI wrapper
		System.loadLibrary("og_jni_wrapper");
	}

    /**
     * Get the output image width.
     * @return output image width
     */
    public native int getOutputFrameW();
    
    /**
     * Get the output image height.
     * @return output image height
     */
    public native int getOutputFrameH();
    
    /**
     * Initialize ogles_gpgpu. Do this only once per application runtime.
     */
    public native void init(boolean usePlatformOptimizations, boolean initEGL, boolean createRenderDisp);
    
    /**
     * Prepare ogles_gpgpu for incoming images of size <inW> x <inH>. Do this
     * each time you change the input image size (and of course at the beginning
     * for the initial input image size). 
     * 
     * @param inW input frame width
     * @param inH input frame height
     */
    public native void prepare(int inW, int inH, boolean prepareInput);
    
    /**
     * Specify render display properties. Before that, <init()> must have been called with
     * "createRenderDisp" = true
     *
     * @param w render display width
     * @param h render display height
     * @param orientation render orientation
     */
    public native void setRenderDisp(int w, int h, int orientation);
    
    /**
     * Cleanup the ogles_gpgpu resources. Call this only once at the end of the
     * application runtime.
     */
    public native void cleanup();
    
    /**
     * Set the raw input pixel data as ARGB integer array. The size of this array
     * must equal <inW> * <inH> (set via <prepare()>).
     * 
     * @param pixels    pixel data with ARGB integers
     */
    public native void setInputPixels(int[] pixels);
    
    /**
     * Set input as reference to a texture with ID <texID>.
     * @param texId
     */
    public native void setInputTexture(int texId);
    
    /**
     * Executes the GPGPU processing tasks.
     */
    public native void process();
    
    /**
     * Render the output to a render display. Before that, <init()> must have been called
     * with "createRenderDisp" = true
     */
    public native void renderOutput();

    /**
     * Return the input pixel data as ARGB ByteBuffer. The size of this byte buffer
     * equals output frame width * output frame height * 4 (4 channel ARGB data).
     * @return pixel data as ByteBuffer
     */
    public native ByteBuffer getOutputPixels();
    
    /**
     * Return the time measurements in milliseconds for the individual steps: input, process, output
     * if ogles_gpgpu was compiled for benchmarking, otherwise return null.
     * 
     * @return time measurements in milliseconds for the individual steps input, process, output
     * 		   or null if ogles_gpgpu was not compiled for benchmarking 
     */
    public native double[] getTimeMeasurements();
}