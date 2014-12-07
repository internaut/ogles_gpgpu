package ogles_gpgpu;

import static javax.microedition.khronos.egl.EGL10.EGL_ALPHA_SIZE;
import static javax.microedition.khronos.egl.EGL10.EGL_BLUE_SIZE;
import static javax.microedition.khronos.egl.EGL10.EGL_DEFAULT_DISPLAY;
import static javax.microedition.khronos.egl.EGL10.EGL_DEPTH_SIZE;
import static javax.microedition.khronos.egl.EGL10.EGL_GREEN_SIZE;
import static javax.microedition.khronos.egl.EGL10.EGL_HEIGHT;
import static javax.microedition.khronos.egl.EGL10.EGL_NONE;
import static javax.microedition.khronos.egl.EGL10.EGL_NO_CONTEXT;
import static javax.microedition.khronos.egl.EGL10.EGL_RED_SIZE;
import static javax.microedition.khronos.egl.EGL10.EGL_STENCIL_SIZE;
import static javax.microedition.khronos.egl.EGL10.EGL_WIDTH;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL10;

import ogles_gpgpu.examples.ogstillimagedroid.BuildConfig;
import android.opengl.GLSurfaceView;
import android.util.Log;

public class OGManager {
	private final String TAG = this.getClass().getSimpleName();
	
	GLSurfaceView.Renderer renderer;
	
	private EGL10 egl;
	private EGLDisplay eglDisp;
	private EGLConfig[] eglConf;
	private EGLConfig curEGLConf;
	private EGLContext eglCtx;
	private EGLSurface eglSurface;
	private GL10 gl;
	private long tid = 0;	// thread id
	
	private int inputW;
	private int inputH;

	public OGManager() throws RuntimeException {
	}
	
	public void init(int inW, int inH) throws RuntimeException {
		inputW = inW;
		inputH = inH;
		
		egl = (EGL10) EGLContext.getEGL();
		if (egl == null || egl.eglGetError() != EGL10.EGL_SUCCESS) throw new RuntimeException("error getting EGL interface");
		
		eglDisp = egl.eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if (eglDisp == null || egl.eglGetError() != EGL10.EGL_SUCCESS) throw new RuntimeException("error getting EGL default display");
		
		int eglVers[] = new int[2];
		if (!egl.eglInitialize(eglDisp, eglVers) || egl.eglGetError() != EGL10.EGL_SUCCESS) {
			throw new RuntimeException("error on eglInitialize");
		} else {
			Log.i(TAG, "initialized EGL version " + eglVers[0] + "." + eglVers[1]);
		}
		
		curEGLConf = chooseConfig();
		
		eglCtx = egl.eglCreateContext(eglDisp, curEGLConf, EGL_NO_CONTEXT, null);
		if (eglCtx == null || eglCtx.equals(EGL10.EGL_NO_CONTEXT) || egl.eglGetError() != EGL10.EGL_SUCCESS) throw new RuntimeException("error creating EGL context");
		
		Log.i(TAG, "creating EGL surface of size " + inW + "x" + inH);
		
        int[] attribList = new int[] {
                EGL_WIDTH, inW,
                EGL_HEIGHT, inH,
                EGL_NONE
        };
		
		eglSurface = egl.eglCreatePbufferSurface(eglDisp, curEGLConf, attribList);
		if (eglSurface == null || egl.eglGetError() != EGL10.EGL_SUCCESS) throw new RuntimeException("error creating EGL surface");
		
		if (!egl.eglMakeCurrent(eglDisp, eglSurface, eglSurface, eglCtx) || egl.eglGetError() != EGL10.EGL_SUCCESS) {
			throw new RuntimeException("error on eglMakeCurrent");
		}
		
		
		
		gl = (GL10)eglCtx.getGL();
		if (gl == null || egl.eglGetError() != EGL10.EGL_SUCCESS) throw new RuntimeException("error getting GL interface");
		
		tid = android.os.Process.myTid();

		Log.i(TAG, "owner thread id is " + tid);
		Log.i(TAG, "owner thread name is " + Thread.currentThread().getName());
		
		// call ogles_gpgpu native init()
//		ogWrapper.init();
		
		// call ogles_gpgpu native prepare()
//		ogWrapper.prepare(inW, inH);
/*		ogWrapper.onSurfaceCreated(gl, curEGLConf);
		ogWrapper.onSurfaceChanged(gl, inW, inH);*/
	}
	
	public void setRenderer(GLSurfaceView.Renderer renderer) {
		renderer.onSurfaceCreated(gl, curEGLConf);
		renderer.onSurfaceChanged(gl, inputW, inputH);
	}
	
	public void render() {
		renderer.onDrawFrame(gl);
	}
	
//	public void destroy() {
//		if (BuildConfig.DEBUG && Thread.currentThread().getId() == tid) throw new AssertionError("caller thread id: " + Thread.currentThread().getId());
//		
//		ogWrapper.cleanup();
//	}
	
//	public void setInput(int[] pixels) {
//		if (BuildConfig.DEBUG && Thread.currentThread().getId() == tid) throw new AssertionError("caller thread id: " + Thread.currentThread().getId());
//		
//		ogWrapper.setInputPixels(pixels);
//	}
//	
//	public void runProcessing() {
//		if (BuildConfig.DEBUG && Thread.currentThread().getId() == tid) throw new AssertionError("caller thread id: " + Thread.currentThread().getId());
//		
//		ogWrapper.process();
////		ogWrapper.onDrawFrame(gl);
//	}
//	
//	public int[] getOutput() {
//		if (BuildConfig.DEBUG && Thread.currentThread().getId() == tid) throw new AssertionError("caller thread id: " + Thread.currentThread().getId());
//		
//		return ogWrapper.getOutputPixels();
//	}
//	
//	public int getOutputFrameW() {
//		return ogWrapper.getOutputFrameW();
//	}
//	
//	public int getOutputFrameH() {
//		return ogWrapper.getOutputFrameH();
//	}
	
    private EGLConfig chooseConfig() {
        int[] attribList = new int[] {
                EGL_RED_SIZE, 		8,
                EGL_GREEN_SIZE, 	8,
                EGL_BLUE_SIZE, 		8,
                EGL_ALPHA_SIZE, 	8,
                EGL_DEPTH_SIZE, 	16,
                EGL_STENCIL_SIZE, 	0,
                EGL_NONE
        };
 
        int[] numConfig = new int[1];
        egl.eglChooseConfig(eglDisp, attribList, null, 0, numConfig);
        eglConf = new EGLConfig[numConfig[0]];
        egl.eglChooseConfig(eglDisp, attribList, eglConf, numConfig[0], numConfig);
 
        listConfig();
 
        return eglConf[0];  // Best match is probably the first configuration
    }
    
    private void listConfig() {
        Log.i(TAG, "Config List {");
 
        for (EGLConfig config : eglConf) {
            int d, s, r, g, b, a;
                   
            // Expand on this logic to dump other attributes        
            d = getConfigAttrib(config, EGL_DEPTH_SIZE);
            s = getConfigAttrib(config, EGL_STENCIL_SIZE);
            r = getConfigAttrib(config, EGL_RED_SIZE);
            g = getConfigAttrib(config, EGL_GREEN_SIZE);
            b = getConfigAttrib(config, EGL_BLUE_SIZE);
            a = getConfigAttrib(config, EGL_ALPHA_SIZE);                
            Log.i(TAG, "    <d,s,r,g,b,a> = <" + d + "," + s + "," +
                r + "," + g + "," + b + "," + a + ">");
        }
 
        Log.i(TAG, "}");
    }
    
    private int getConfigAttrib(EGLConfig config, int attribute) {
        int[] value = new int[1];
        return egl.eglGetConfigAttrib(eglDisp, config, attribute, value) ? value[0] : 0;
    }
}
