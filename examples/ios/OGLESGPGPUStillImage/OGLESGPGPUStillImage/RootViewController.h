/**
 * iOS App RootViewController -- main application logic
 */

#import <UIKit/UIKit.h>

// include ogles_gpgpu main header
#include "../../../../ogles_gpgpu/ogles_gpgpu.h"

/**
 * Main application logic: Initialize application, set up the view and handle user input.
 * With this application, a series of selectable example images can be processed by the
 * means of GPGPU. The output will be displayed directly.
 */
@interface RootViewController : UIViewController {
    UIView *baseView;       // view fundament on which all other views are added
    UIImageView *imgView;   // image view for example images and GPGPU process output images
    
    EAGLContext *eaglContext;   // OpenGL ES 2.0 context
    
    ogles_gpgpu::Core *gpgpuMngr;   // ogles_gpgpu manager
    
    ogles_gpgpu::GrayscaleProc grayscaleProc;   // pipeline processor 1: convert input to grayscale image
    ogles_gpgpu::ThreshProc simpleThreshProc;       // pipeline processor 2 (alternative 1): simple thresholding
    ogles_gpgpu::AdaptThreshProc adaptThreshProc;   // pipeline processor 2 (alternative 2): adaptive thresholding (two passes)
    
    int selectedTestImg;    // currently selected test image
    BOOL displayingOutput;  // is true if the image processing output is displayed
    
    UIImage *testImg;           // current test image
    unsigned char *testImgData; // current test image RGBA data
    int testImgW;   // test image width
    int testImgH;   // test image height

    UIImage *outputImg;         // current output image
    unsigned char *outputBuf;   // current output image RGBA data
}

@end
