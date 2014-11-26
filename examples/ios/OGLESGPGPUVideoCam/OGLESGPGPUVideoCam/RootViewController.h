/**
 *
 */
#import <UIKit/UIKit.h>

#import <AVFoundation/AVFoundation.h>
#import <AssetsLibrary/AssetsLibrary.h>
#import <GLKit/GLKit.h>

// include ogles_gpgpu main header
#include "../../../../ogles_gpgpu/ogles_gpgpu.h"

#import "CamView.h"

// Camera parameters

#define CAM_SESSION_PRESET  AVCaptureSessionPresetHigh
#define CAM_POSITION        AVCaptureDevicePositionBack

/**
 * Main view controller.
 * Handles UI initialization and interactions, camera frame input and frame processing.
 * With this application, the camera frames can be processed by the means of GPGPU. The
 * output will be displayed directly. Fast direct camera frame access is used so that no
 * data needs to be copied from/to the camera. The camera frame is used directly as texture
 * for the GPU processing tasks.
 */
@interface RootViewController : UIViewController<AVCaptureVideoDataOutputSampleBufferDelegate , GLKViewDelegate> {
    AVCaptureSession *camSession;               // controlls the camera session
    AVCaptureDeviceInput *camDeviceInput;       // input device: camera
    AVCaptureVideoDataOutput *vidDataOutput;    // controlls the video output

    BOOL showCamPreview;        // is YES if the camera preview is shown or NO if the processed frames are shown
    BOOL firstFrame;            // is YES when the current frame is the very first camera frame
    BOOL prepared;              // is YES when everything is ready to process camera frames
    
    CGSize frameSize;           // original camera frame size
    
    UIView *baseView;           // root view
    CamView *camView;           // shows the grabbed video frames ("camera preview")
    GLKView *glView;            // shows the processed video frames as textures in a GLKit view
    
    EAGLContext *eaglContext;   // OpenGL ES 2.0 context
    
    ogles_gpgpu::Core *gpgpuMngr;   // ogles_gpgpu manager
    
    ogles_gpgpu::MemTransfer *gpgpuInputHandler;    // input handler for direct access to the camera frames. weak ref!
    
    ogles_gpgpu::GrayscaleProc grayscaleProc;   // pipeline processor 1: convert input to grayscale image
//    ogles_gpgpu::ThreshProc simpleThreshProc;   // pipeline processor 2 (alternative 1): simple thresholding
    ogles_gpgpu::AdaptThreshProc adaptThreshProc; // pipeline processor 2 (alternative 2): adaptive thresholding (two passes)
    
    ogles_gpgpu::Disp *outputDispRenderer;  // display renderer to directly display the output in the GL view. weak ref!
    
    ogles_gpgpu::RenderOrientation dispRenderOrientation;   // current output orientation of the display renderer
}

@end
