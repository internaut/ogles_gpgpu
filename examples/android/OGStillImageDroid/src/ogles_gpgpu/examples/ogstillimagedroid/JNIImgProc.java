package ogles_gpgpu.examples.ogstillimagedroid;

public class JNIImgProc {
	static {
		System.loadLibrary("jni_img_proc");
	}
	
	public native void grayscale(int[] pixels);
}
