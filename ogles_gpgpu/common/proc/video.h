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

#include <memory>

BEGIN_OGLES_GPGPU

#if __ANDROID__
#  define DFLT_PIX_FORMAT GL_RGBA
#else
#  define DFLT_PIX_FORMAT GL_BGRA
#endif

struct FrameInput
{
    FrameInput() {}
    FrameInput(const Size2d &size, void *pixelBuffer, bool useRawPixels, GLuint inputTexture, GLenum textureFormat)
    : size(size)
    , pixelBuffer(pixelBuffer)
    , useRawPixels(useRawPixels)
    , inputTexture(inputTexture)
    , textureFormat(textureFormat)
    {}
    
    Size2d size;
    void *pixelBuffer = nullptr;
    bool useRawPixels = false;
    GLuint inputTexture = 0;
    GLenum textureFormat = 0;
};

/**
 * Image source class
 */

class VideoSource {
public:
    
    typedef std::function<void(const std::string &tag)> Timer;

    VideoSource(void *glContext=nullptr);

    VideoSource(void *glContext, const Size2d &size, GLenum inputPixFormat);
    
    VideoSource(const Size2d &size, GLenum inputPixFormat);
    
    virtual ~VideoSource();
    
    void init(void *glContext);
    
    void operator()(const FrameInput &frame);
    
    void operator()(const Size2d &size, void* pixelBuffer, bool useRawPixels, GLuint inputTexture=0, GLenum inputPixFormat=DFLT_PIX_FORMAT);
    
    virtual void preConfig() {}
    
    virtual void postConfig() {}
    
    void set(ProcInterface *p);
    
    void setLogger(Timer &timer) { m_timer = timer; }
    
    GLuint getInputTexId();
    
protected:
    
    Timer m_timer;

    void *glContext = nullptr;

    void setInputData(const unsigned char *data);
    
    void configurePipeline(const Size2d &size, GLenum inputPixFormat);
    
    bool firstFrame = true;
    
    Size2d frameSize;
    
    ProcInterface* pipeline = nullptr;

    std::shared_ptr<ogles_gpgpu::Yuv2RgbProc> yuv2RgbProc;
};

END_OGLES_GPGPU

#endif // OGLES_GPGPU_COMMON_VIDEO
