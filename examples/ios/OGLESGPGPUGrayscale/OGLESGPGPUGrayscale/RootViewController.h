#import <UIKit/UIKit.h>

#include "../../../../ogles_gpgpu/ogles_gpgpu.h"

@interface RootViewController : UIViewController {
    UIView *baseView;
    
    EAGLContext *eaglContext;
    
    ogles_gpgpu::Core gpgpuMngr;
    ogles_gpgpu::GrayscaleProc grayscaleProc;
}

@end
