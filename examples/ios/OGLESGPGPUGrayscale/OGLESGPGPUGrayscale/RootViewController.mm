//
//  RootViewController.m
//  OGLESGPGPUGrayscale
//
//  Created by Markus Konrad on 30.10.14.
//  Copyright (c) 2014 INKA Research Group. All rights reserved.
//

#import "RootViewController.h"

@interface RootViewController ()
- (void)initOGLESGPGPU;
@end

@implementation RootViewController

#pragma mark init/dealloc

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        [self becomeFirstResponder];
    }
    return self;
}

- (void)dealloc {
    [baseView release];
    [eaglContext release];
    
    [super dealloc];
}

#pragma mark event handling

- (void)viewDidLoad {
    [super viewDidLoad];
    
    // create an OpenGL context
    eaglContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    if (![EAGLContext setCurrentContext:eaglContext]) {
        NSLog(@"failed setting current EAGL context");
    }

    const CGRect screenRect = [[UIScreen mainScreen] bounds];
    
    NSLog(@"loading view of size %dx%d", (int)screenRect.size.width, (int)screenRect.size.height);
    
    // create an empty base view
    CGRect baseFrame = CGRectMake(0, 0, screenRect.size.height, screenRect.size.width);
    baseView = [[UIView alloc] initWithFrame:baseFrame];
    
    // finally set the base view as view for this controller
    [self setView:baseView];
    
    [self initOGLESGPGPU];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    NSLog(@"touch on root view controller");
}

#pragma mark private methods

- (void)initOGLESGPGPU {
    NSLog(@"initializing ogles_gpgpu");
    
    grayscaleProc.setOutputSize(0.5f);
    gpgpuMngr.addProcToPipeline(&grayscaleProc);
    
    gpgpuMngr.init(512, 512, true);
}

@end
