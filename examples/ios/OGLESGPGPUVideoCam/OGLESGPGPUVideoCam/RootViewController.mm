#import "RootViewController.h"

#define INITIAL_PROC_TYPE 1

/**
 * Small helper function to convert a fourCC <code> to
 * a character string <fourCC> for printf and the like
 */
void fourCCStringFromCode(int code, char fourCC[5]) {
    for (int i = 0; i < 4; i++) {
        fourCC[3 - i] = code >> (i * 8);
    }
    fourCC[4] = '\0';
}

@interface RootViewController(Private)
/**
 * Init views
 */
- (void)initUI;

/**
 * initialize camera
 */
- (void)initCam;

/**
 * Init ogles_gpgpu. Set up processing pipeline.
 */
- (void)initOGLESGPGPU;

- (void)initGPUPipeline:(int)selectedType;

/**
 * Notify the video session about the interface orientation change
 */
- (void)interfaceOrientationChanged:(UIInterfaceOrientation)o;

/**
 * handler that is called when a output selection button is pressed
 */
- (void)procOutputSelectBtnAction:(UIButton *)sender;

/**
 * Prepare the GLKView and ogles_gpgpu for camera frames of <size>.
 */
- (void)prepareForFramesOfSize:(CGSize)size;

@end


@implementation RootViewController


#pragma mark init/dealloc

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        selectedProcType = -1;
        showCamPreview = (selectedProcType == 0);
        firstFrame = YES;
        prepared = NO;
        
        // set default orientation
        [self interfaceOrientationChanged:self.interfaceOrientation];
        
        // create an OpenGL context first of all
        eaglContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        if (!eaglContext || ![EAGLContext setCurrentContext:eaglContext]) {
            NSLog(@"failed setting current EAGL context");
        }
    }
    
    return self;
}

- (void)dealloc {
    // delete ogles_gpgpu singleton object
    ogles_gpgpu::Core::destroy();
    gpgpuMngr = NULL;
    
    // release camera stuff
    [vidDataOutput release];
    [camDeviceInput release];
    [camSession release];
    
    // release views
    [camView release];
    [baseView release];
    
    [super dealloc];
}

#pragma mark parent methods

- (void)didReceiveMemoryWarning {
    NSLog(@"memory warning!!!");
    
    [super didReceiveMemoryWarning];
}

- (void)loadView {
    // create the UI elements
    [self initUI];
}

- (void)viewWillAppear:(BOOL)animated {
    NSLog(@"view will appear - start camera session");
    [camSession startRunning];
}

- (void)viewDidDisappear:(BOOL)animated {
    NSLog(@"view did disappear - stop camera session");
    [camSession stopRunning];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    // set up camera
    [self initCam];
    
    // set up ogles_gpgpu
    [self initOGLESGPGPU];
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)o duration:(NSTimeInterval)duration {
    [self interfaceOrientationChanged:o];
}

#pragma mark AVCaptureVideoDataOutputSampleBufferDelegate methods

- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection
{
    // note that this method does needs to run on the main thread! this is specified in the initCam method
    
    // get a reference to the image buffer. A CVImageBufferRef is actually a CVPixelBufferRef, so we can
    // use it later in the MemTransferIOS class of ogles_gpgpu for direct texture access.
    CVImageBufferRef imgBuf = CMSampleBufferGetImageBuffer(sampleBuffer);
    if (!imgBuf) {
        NSLog(@"Error obtaining image buffer from camera sample buffer");
        
        return;
    }
    
    // when we get the first frame, prepare the system for the size of the incoming frames
    if (firstFrame) {
        frameSize = CVImageBufferGetDisplaySize(imgBuf);
        [self prepareForFramesOfSize:frameSize];
        firstFrame = NO;
    }
    
    // on each new frame, this will release the input buffers and textures, and prepare new ones
    // texture format must be GL_BGRA because this is one of the native camera formats (see initCam)
    gpgpuInputHandler->prepareInput(frameSize.width, frameSize.height, GL_BGRA, imgBuf);
    
    // set the input texture id - we do not copy any data, we use the camera frame directly as texture!
    gpgpuMngr->setInputTexId(gpgpuInputHandler->getInputTexId());
    
    // run processing pipeline
    gpgpuMngr->process();
    
    // update the GL view to display the output directly
    [glView display];
}

#pragma mark GLKView delegate methods

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    if (!prepared) return;
    
    // render output directly to screen
    outputDispRenderer->render();
}

#pragma mark private methods

- (void)initUI {
    const CGRect screenRect = [[UIScreen mainScreen] bounds];
    
    NSLog(@"loading view of size %dx%d", (int)screenRect.size.width, (int)screenRect.size.height);
    
    // create an empty base view
    baseFrame = CGRectMake(0, 0, screenRect.size.height, screenRect.size.width);
    baseView = [[UIView alloc] initWithFrame:baseFrame];
    
    // create the image view for the camera frames
    camView = [[CamView alloc] initWithFrame:baseFrame];
    [camView setHidden:showCamPreview];
    [baseView addSubview:camView];
    
    // create the GLKView to show the processed frames as textures
    glView = [[GLKView alloc] initWithFrame:baseFrame context:eaglContext];
    [glView setDelegate:self];
    [glView setHidden:showCamPreview];
    [baseView addSubview:glView];
    
    // set a list of buttons for processing output display
    NSArray *btnTitles = [NSArray arrayWithObjects:
                          @"Normal",
                          @"Adapt. Thresh.",
                          @"Simple Thresh.",
                          @"Gauss",
                          nil];
    for (int btnIdx = 0; btnIdx < btnTitles.count; btnIdx++) {
        UIButton *procOutputSelectBtn = [UIButton buttonWithType:UIButtonTypeSystem];
        [procOutputSelectBtn setTag:btnIdx - 1];
        [procOutputSelectBtn setTitle:[btnTitles objectAtIndex:btnIdx]
                             forState:UIControlStateNormal];
        int btnW = 140;
        [procOutputSelectBtn setFrame:CGRectMake(10 + (btnW + 20) * btnIdx, 10, btnW, 35)];
        [procOutputSelectBtn setOpaque:YES];
        [procOutputSelectBtn addTarget:self
                                action:@selector(procOutputSelectBtnAction:)
                      forControlEvents:UIControlEventTouchUpInside];
        
        if (btnIdx == INITIAL_PROC_TYPE) {
            [procOutputSelectBtn setSelected:YES];
            prevSelBtn = procOutputSelectBtn;
        }
        
        [baseView addSubview:procOutputSelectBtn];
    }
    
    // finally set the base view as view for this controller
    [self setView:baseView];
}

- (void)initOGLESGPGPU {
    NSLog(@"initializing ogles_gpgpu");
    
    // get ogles_gpgpu::Core singleton instance
    gpgpuMngr = ogles_gpgpu::Core::getInstance();
    
    // enable iOS optimizations (fast texture access)
    ogles_gpgpu::Core::tryEnablePlatformOptimizations();
    
    // do not use mipmaps (will not work with NPOT images)
    gpgpuMngr->setUseMipmaps(false);
    
    // set up grayscale processor
    grayscaleProc.setOutputSize(0.5f);  // downscale to half size
//    grayscaleProc.setGrayscaleConvType(ogles_gpgpu::GRAYSCALE_INPUT_CONVERSION_BGR);    // needed, because we actually have BGRA input data when we use iOS optimized memory access
    
    // create the pipeline
    [self initGPUPipeline:INITIAL_PROC_TYPE];
    
    // initialize the pipeline
    gpgpuMngr->init(eaglContext);

}

- (void)initGPUPipeline:(int)type {
    if (selectedProcType == type) return;   // no change
    
    // reset the pipeline
    gpgpuMngr->reset();
    
    // create the pipeline
    if (type == 1) {
        gpgpuMngr->addProcToPipeline(&grayscaleProc);
        gpgpuMngr->addProcToPipeline(&adaptThreshProc);
    } else if (type == 2) {
        gpgpuMngr->addProcToPipeline(&grayscaleProc);
        gpgpuMngr->addProcToPipeline(&simpleThreshProc);
    } else if (type == 3) {
        gpgpuMngr->addProcToPipeline(&gaussProc);
    } else {
        NSLog(@"GPU pipeline definition #%d not supported", type);
    }
    
    // create the display renderer with which we can directly render the output
    // to the screen via OpenGL
    outputDispRenderer = gpgpuMngr->createRenderDisplay();
    outputDispRenderer->setOutputRenderOrientation(dispRenderOrientation);
    
    // reset this to call prepareForFramesOfSize again
    firstFrame = YES;
    if (prepared) {
        prepared = NO;
        [glView setFrame:baseFrame];
    }
}

- (void)initCam {
    NSLog(@"initializing cam");
    
    NSError *error = nil;
    
    // set up the camera capture session
    camSession = [[AVCaptureSession alloc] init];
    [camSession setSessionPreset:CAM_SESSION_PRESET];
    [camView setSession:camSession];
    
    // get the camera device
	NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    assert(devices.count > 0);
    
	AVCaptureDevice *camDevice = [devices firstObject];
	for (AVCaptureDevice *device in devices) {
		if ([device position] == CAM_POSITION) {
			camDevice = device;
			break;
		}
	}
    
    camDeviceInput = [[AVCaptureDeviceInput deviceInputWithDevice:camDevice error:&error] retain];
    
    if (error) {
        NSLog(@"error getting camera device: %@", error);
        return;
    }
    
    assert(camDeviceInput);
    
    // add the camera device to the session
    if ([camSession canAddInput:camDeviceInput]) {
        [camSession addInput:camDeviceInput];
        [self interfaceOrientationChanged:self.interfaceOrientation];
    }
    
    // create camera output
    vidDataOutput = [[AVCaptureVideoDataOutput alloc] init];
    [camSession addOutput:vidDataOutput];
    
    // set dispatch to be on the main thread so OpenGL can do things with the data
    // THIS IS CRUCIAL!
    [vidDataOutput setSampleBufferDelegate:self queue:dispatch_get_main_queue()];
    
    // get best output video format
    NSArray *outputPixelFormats = vidDataOutput.availableVideoCVPixelFormatTypes;
    int neededPxFmtCode = kCVPixelFormatType_32BGRA;
    int foundPxFmtCode = -1;
    for (NSNumber *format in outputPixelFormats) {
        int code = [format intValue];
        if (code == neededPxFmtCode) {
            foundPxFmtCode = code;
        }
        
        char fourCC[5];
        fourCCStringFromCode(code, fourCC);
        NSLog(@"available video output format: %s (code %d)", fourCC, code);
    }
    
    if (foundPxFmtCode < 0) {
        NSLog(@"could not find the needed pixel format for the camera frames!");
        return;
    }

    // specify output video format
    NSDictionary *outputSettings = [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:foundPxFmtCode]
                                                               forKey:(id)kCVPixelBufferPixelFormatTypeKey];
    [vidDataOutput setVideoSettings:outputSettings];
}

- (void)procOutputSelectBtnAction:(UIButton *)sender {
    NSLog(@"proc output selection button pressed: %@ (proc type %ld)",
          [sender titleForState:UIControlStateNormal], (long)sender.tag);
    
    showCamPreview = (sender.tag < 0);
    [camView setHidden:!showCamPreview];    // only show original camera frames in "normal" display mode
    [glView setHidden:showCamPreview];      // only show processed frames for other than "normal" display mode
    
    if (!showCamPreview) {
        [self initGPUPipeline:sender.tag + 1];
    }
    
    [prevSelBtn setSelected:NO];
    [sender setSelected:YES];
    prevSelBtn = sender;
}

- (void)interfaceOrientationChanged:(UIInterfaceOrientation)o {
    [[(AVCaptureVideoPreviewLayer *)camView.layer connection] setVideoOrientation:(AVCaptureVideoOrientation)o];
    
    // update the display renderer, too
    if (o == UIInterfaceOrientationLandscapeLeft) {
        dispRenderOrientation = ogles_gpgpu::RenderOrientationStdMirrored;
    } else {
        dispRenderOrientation = ogles_gpgpu::RenderOrientationFlipped;
    }
    
    if (prepared && outputDispRenderer) {
        outputDispRenderer->setOutputRenderOrientation(dispRenderOrientation);
    }
}

- (void)prepareForFramesOfSize:(CGSize)size {
    float frameAspectRatio = size.width / size.height;
    NSLog(@"camera frames are of size %dx%d (aspect %f)", (int)size.width, (int)size.height, frameAspectRatio);
    
    float uiScaling = [[UIScreen mainScreen] scale];
    
    NSLog(@"device's UI scaling factor is %f", uiScaling);
    
    // update gl frame view size
    float newViewH = glView.frame.size.width / frameAspectRatio;   // calc new height
    float viewYOff = (glView.frame.size.height - newViewH) / 2;
    
    CGRect correctedViewRect = CGRectMake(0, viewYOff, glView.frame.size.width, newViewH);

    [glView setFrame:correctedViewRect];

    // update output the display renderer's output size
    outputDispRenderer->setOutputSize(glView.frame.size.width * uiScaling, newViewH * uiScaling);
    
    // prepare ogles_gpgpu for the incoming frame size
    // GL_NONE means that the input memory transfer object is NOT prepared
    // this will be done in captureOutput: on each new frame
    gpgpuMngr->prepare(size.width, size.height, GL_NONE);
    gpgpuInputHandler = gpgpuMngr->getInputMemTransfer();
    
    // everything prepared
    prepared = YES;
}

@end
