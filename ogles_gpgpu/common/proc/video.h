//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015, http://www.mkonrad.net
//         David Hirvonen
//
// See LICENSE file in project repository root for the license.
//

/**
 * Helper class to simply display an output.
 */
#ifndef OGLES_GPGPU_COMMON_VIDEO
#define OGLES_GPGPU_COMMON_VIDEO

#include "../common_includes.h"
#include "base/procinterface.h"
#include "base/procbase.h"
#include "yuv2rgb.h"

namespace ogles_gpgpu {

#if __ANDROID__
#  define DFLT_PIX_FORMAT GL_RGBA
#else
#  define DFLT_PIX_FORMAT GL_BGRA
#endif

/**
 * Image source class
 */

class VideoSource {
public:

    VideoSource();
    
    VideoSource(const Size2d &size, GLenum inputPixFormat);
    
    void operator()(const Size2d &size, void* pixelBuffer, bool useRawPixels, GLuint inputTexture=0, GLenum inputPixFormat=DFLT_PIX_FORMAT);

    void set(ProcBase *p) {
        pipeline = p;
    }
    
protected:

    void setInputData(const unsigned char *data);
    
    void configurePipeline(const Size2d &size, GLenum inputPixFormat);
    
    bool firstFrame = true;
    
    Size2d frameSize;
    
    ProcBase* pipeline = nullptr;

    std::shared_ptr<ogles_gpgpu::Yuv2RgbProc> yuv2RgbProc;

};
}

#endif // OGLES_GPGPU_COMMON_VIDEO
