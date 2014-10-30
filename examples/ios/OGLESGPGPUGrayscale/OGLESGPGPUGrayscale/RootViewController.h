#import <UIKit/UIKit.h>

#include "../../../../ogles_gpgpu/ogles_gpgpu.h"

@interface RootViewController : UIViewController {
    UIView *baseView;
    UIImageView *imgView;
    
    EAGLContext *eaglContext;
    
    ogles_gpgpu::Core gpgpuMngr;
    ogles_gpgpu::GrayscaleProc grayscaleProc;
    
    UIImage *testImg;
    unsigned char *testImgData;
    int testImgW;
    int testImgH;

    UIImage *outputImg;
    unsigned char *outputBuf;
}

@end
