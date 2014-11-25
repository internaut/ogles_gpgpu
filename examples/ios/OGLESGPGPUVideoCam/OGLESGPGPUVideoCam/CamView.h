#import <UIKit/UIKit.h>

@class AVCaptureSession;

@interface CamView : UIView

@property (nonatomic, assign) AVCaptureSession *session;

@end
