#include "video.h"
#include "yuv2rgb.h"

using namespace ogles_gpgpu;

VideoSource::VideoSource()
{
    
}

VideoSource::VideoSource(const Size2d &size, GLenum inputPixFormat)
{
    configurePipeline(size, inputPixFormat);
}

void VideoSource::configurePipeline(const Size2d &size, GLenum inputPixFormat)
{
    if(inputPixFormat == 0) // 0 == NV{12,21}
    {
        if(!yuv2RgbProc)
        {
            yuv2RgbProc = std::make_shared<ogles_gpgpu::Yuv2RgbProc>();
            yuv2RgbProc->init(size.width, size.height, 0, true);
            frameSize = size;
        }

        if(size != frameSize)
        {
            yuv2RgbProc->reinit(size.width, size.height, true);
        } 

        yuv2RgbProc->createFBOTex(false); // TODO: mipmapping?
    }
    
    pipeline->prepare(size.width, size.height, inputPixFormat);
    frameSize = size;
}

void VideoSource::operator()(const Size2d &size, void* pixelBuffer, bool useRawPixels, GLuint inputTexture, GLenum inputPixFormat)
{
    assert(pipeline);
    
    if (firstFrame)
    {
        configurePipeline(size, inputPixFormat);
        firstFrame = false;
    }
    
    auto gpgpuInputHandler = pipeline->getMemTransferObj();
    gpgpuInputHandler->setUseRawPixels(useRawPixels);
    
    // on each new frame, this will release the input buffers and textures, and prepare new ones
    // texture format must be GL_BGRA because this is one of the native camera formats (see initCam)
    if(pixelBuffer)
    {
        if(inputPixFormat == 0)
        {
            yuv2RgbProc = std::make_shared<ogles_gpgpu::Yuv2RgbProc>();
            
            // YUV: Special case NV12=>BGR
            auto manager = yuv2RgbProc->getMemTransferObj();
            if (useRawPixels)
            {
                manager->setUseRawPixels(true);
            }
            manager->prepareInput(frameSize.width, frameSize.height, inputPixFormat, pixelBuffer);
            
            yuv2RgbProc->setTextures(manager->getLuminanceTexId(), manager->getChrominanceTexId());
            yuv2RgbProc->render();
            glFinish();
            
            gpgpuInputHandler->prepareInput(frameSize.width, frameSize.height, GL_NONE, nullptr);
            inputTexture = yuv2RgbProc->getOutputTexId();
        }
        else
        {
            gpgpuInputHandler->prepareInput(frameSize.width, frameSize.height, inputPixFormat, pixelBuffer);
            
            // gpgpuMngr->setInputData(reinterpret_cast< const unsigned char *>(pixelBuffer));
            
            setInputData(reinterpret_cast< const unsigned char *>(pixelBuffer));
            
            inputTexture = gpgpuInputHandler->getInputTexId();
        }
    }
    
    assert(inputTexture); // inputTexture must be defined at this point
    pipeline->process(inputTexture, 1, GL_TEXTURE_2D);
}

void VideoSource::setInputData(const unsigned char *data) {
    
#if 1
    bool useMipmaps = false;
#else
    assert(initialized && inputTexId > 0);
    
    // check set up and input data
    if (useMipmaps && !inputSizeIsPOT && !glExtNPOTMipmaps) {
        OG_LOGINF("Core", "WARNING: NPOT input image provided but NPOT mipmapping not supported!");
        OG_LOGINF("Core", "mipmapping disabled!");
        useMipmaps = false;
    }
#endif
    
    // set texture
    glActiveTexture(GL_TEXTURE1);
    
    // copy data as texture to GPU
    pipeline->setExternalInputData(data);
    
    // mipmapping
    if (pipeline->getWillDownscale() && useMipmaps) {
        OG_LOGINF("Core", "generating mipmap for input image");
        // enabled
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        // disabled
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    
    Tools::checkGLErr("Core", "set texture parameters for input data");
    
    glFinish();
}


