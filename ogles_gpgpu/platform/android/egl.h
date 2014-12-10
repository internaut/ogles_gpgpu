//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * Android EGL context manager
 */

#ifndef OGLES_GPGPU_ANDROID_EGL
#define OGLES_GPGPU_ANDROID_EGL

#include <EGL/egl.h>

namespace ogles_gpgpu {
    
/**
 * Android EGL context manager class with static functions for setting up and
 * tearing down the EGL context and its pixelbuffer surface.
 */
class EGL {
public:
    /**
     * Create a EGL context by choosing an appropriate config for the specified bit-sizes.
     * Returns true on success, otherwise false.
     */
    static bool setup(int rSize = 8, int gSize = 8, int bSize = 8, int aSize = 8, int depthSize = 16);
    
    /**
     * Create a pixelbuffer surface of size <w>x<h>. If the surface already exists, it will be
     * destroyed and recreated (no matter if its size changed or not).
     * Returns true on success, otherwise false.
     */
    static bool createPBufferSurface(int w, int h);
    
    /**
     * Activate current EGL setup. Both setup() and createPBufferSurface() should be
     * called first.
     */
    static bool activate();
    
    /**
     * Deactivate current EGL setup.
     */
    static bool deactivate();
    
    /**
     * Destroy the EGL context, display and surface instances. Also calls deactivate().
     */
    static void shutdown();
    
private:
    /**
     * Destroy the current EGL surface, if it was created.
     */
    static void destroySurface();
    
    
    static EGLConfig conf;
    static EGLSurface surface;
    static EGLContext ctx;
    static EGLDisplay disp;
};
}

#endif