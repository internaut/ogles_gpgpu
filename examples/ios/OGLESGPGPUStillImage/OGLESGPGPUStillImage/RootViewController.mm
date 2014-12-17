#import "RootViewController.h"

#define IDIOM    UI_USER_INTERFACE_IDIOM()
#define IPAD     UIUserInterfaceIdiomPad

// array with available test images
static NSArray *availableTestImages = [NSArray arrayWithObjects:
                                       @"moon_1024x512.png",
                                       @"moon_1024x1024.png",
                                       @"moon_2048x2048.png",
                                       @"building_2048x1536.jpg",
                                       nil];

/**
 * RootViewController private methods
 */
@interface RootViewController ()

/**
 * Init views
 */
- (void)initUI;

/**
 * Init ogles_gpgpu. Set up processing pipeline.
 */
- (void)initOGLESGPGPU;

/**
 * Action when input test image was selected (buttons on top)
 */
- (void)testImgBtnPressedAction:(id)sender;

/**
 * Present test image number <num>. Force display update <forceDisplay>.
 */
- (void)presentTestImg:(int)num forceDisplay:(BOOL)forceDisplay;

/**
 * Present the image processing output
 */
- (void)presentOutputImg;

/**
 * Run image processing on GPU via ogles_gpgpu.
 */
- (void)runImgProcOnGPU;

/**
 * Helper method: Convert UIImage <img> to RGBA data.
 */
- (unsigned char *)uiImageToRGBABytes:(UIImage *)img;

/**
 * Helper method: Convert RGBA <data> of size <w>x<h> to UIImage object.
 */
- (UIImage *)rgbaBytesToUIImage:(unsigned char *)data width:(int)w height:(int)h;

@end



/**
 * RootViewController implementation
 */
@implementation RootViewController

#pragma mark init/dealloc

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // set defaults
        selectedTestImg = -1;
        displayingOutput = NO;
    }
    return self;
}

- (void)dealloc {
    // delete data buffers
    if (testImgData) delete [] testImgData;
    if (outputBuf) delete [] outputBuf;
    
    // delete ogles_gpgpu singleton object
    ogles_gpgpu::Core::destroy();
    gpgpuMngr = NULL;
    
    // release image objects
    [testImg release];
    [outputImg release];
    
    // release views
    [imgView release];
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
    
    // init UI
    [self initUI];
    
    // init ogles_gpgpu
    [self initOGLESGPGPU];
    
    // load default image
    [self presentTestImg:0 forceDisplay:YES];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    if (!displayingOutput) {
        NSLog(@"WILL DISPLAY OUTPUT");
        
        // run the image processing stuff on the GPU and display the output
        [self runImgProcOnGPU];
        [self presentOutputImg];
        
        displayingOutput = YES;
    } else {
        NSLog(@"WILL DISPLAY TEST IMAGE");
        
        // display the test image as input image
        [self presentTestImg:selectedTestImg forceDisplay:YES];
        
        displayingOutput = NO;
    }
}

#pragma mark private methods

- (void)initUI {
    // get screen rect for landscape mode
    CGRect screenRect = [[UIScreen mainScreen] bounds];
    if (screenRect.size.width < screenRect.size.height) {
        float tmp = screenRect.size.width;
        screenRect.size.width = screenRect.size.height;
        screenRect.size.height = tmp;
    }
    NSLog(@"loading view of size %dx%d", (int)screenRect.size.width, (int)screenRect.size.height);
    
    // create an empty base view
    baseView = [[UIView alloc] initWithFrame:screenRect];
    
    // create the test image view
    imgView = [[UIImageView alloc] initWithFrame:screenRect];
    [baseView addSubview:imgView];
    
    // create buttons for loading the test images
    int i = 0;
    int btnW = 180;
    int btnMrgn = 10;
    int btnX = btnMrgn;
    int btnY = 20;
    for (NSString *testImgName in availableTestImages) {
        if (i > 0) {
            if (IDIOM != IPAD) {
                if (i % 2 == 0) {
                    btnY += 20 + btnMrgn;
                    btnX = btnMrgn;
                } else {
                    btnX += (btnW + btnMrgn);
                }
            } else {
                btnX += (btnW + btnMrgn);
            }
        }
        
        UIButton *btn = [UIButton buttonWithType:UIButtonTypeRoundedRect];
        [btn setTitle:[testImgName stringByDeletingPathExtension] forState:UIControlStateNormal];
        [btn setTag:i];
        [btn setBackgroundColor:[UIColor whiteColor]];
        [btn setFrame:CGRectMake(btnX, btnY, btnW, 26)];
        [btn addTarget:self action:@selector(testImgBtnPressedAction:) forControlEvents:UIControlEventTouchUpInside];
        
        [baseView addSubview:btn];
        i++;
    }
    
    // finally set the base view as view for this controller
    [self setView:baseView];
}

- (void)testImgBtnPressedAction:(id)sender {
    UIButton *btn = sender;
    [self presentTestImg:btn.tag forceDisplay:displayingOutput];
    displayingOutput = NO;
}

- (void)presentTestImg:(int)num forceDisplay:(BOOL)forceDisplay {
    if (!forceDisplay && selectedTestImg == num) return; // no change
    
    // get image file name
    NSString *testImgFile = [availableTestImages objectAtIndex:num];
    
    // release previous image object
    [testImg release];
    
    // load new image data
    testImg = [[UIImage imageNamed:testImgFile] retain];
    testImgW = (int)testImg.size.width;
    testImgH = (int)testImg.size.height;
    
    if (testImg) {
        NSLog(@"loaded test image %@ with size %dx%d", testImgFile, testImgW, testImgH);
    } else {
        NSLog(@"could not load test image %@", testImgFile);
    }
    
    // show the image object
    [imgView setImage:testImg];
    
    // release previous image data
    if (testImgData) delete [] testImgData;
    
    // get the RGBA bytes of the image
    testImgData = [self uiImageToRGBABytes:testImg];
    
    if (!testImgData) {
        NSLog(@"could not get RGBA data from test image %@", testImgFile);
    }
    
    // prepare ogles_gpgpu for this image size
    gpgpuMngr->prepare(testImgW, testImgH, GL_BGRA);
    
    // delete previous output data buffer
    if (outputBuf) delete [] outputBuf;
    
    // create output data buffer
    outputBuf = new unsigned char[gpgpuMngr->getOutputFrameW() * gpgpuMngr->getOutputFrameH() * 4];
    
    selectedTestImg = num;  // update
}

- (void)presentOutputImg {
    // release old image object
    [outputImg release];
    
    // create new image object from RGBA data
    outputImg = [[self rgbaBytesToUIImage:outputBuf
                                    width:gpgpuMngr->getOutputFrameW()
                                   height:gpgpuMngr->getOutputFrameH()] retain];
    if (!outputImg) {
        NSLog(@"error converting output RGBA data to UIImage");
    } else {
        NSLog(@"presenting output image of size %dx%d", (int)outputImg.size.width, (int)outputImg.size.height);
    }
    
    //    [imgView setFrame:CGRectMake(0, 0, outputImg.size.width, outputImg.size.height)];
    
    [imgView setImage:outputImg];
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
    grayscaleProc.setGrayscaleConvType(ogles_gpgpu::GRAYSCALE_INPUT_CONVERSION_BGR);    // needed, because we actually have BGRA input data when we use iOS optimized memory access
    
    // create the pipeline
    gpgpuMngr->addProcToPipeline(&grayscaleProc);
//    gpgpuMngr->addProcToPipeline(&simpleThreshProc);
    gpgpuMngr->addProcToPipeline(&adaptThreshProc);

    // initialize the pipeline
    gpgpuMngr->init(eaglContext);
}

- (void)runImgProcOnGPU {
    NSLog(@"copying image to GPU...");
    gpgpuMngr->setInputData(testImgData);
    NSLog(@"converting...");
    gpgpuMngr->process();
    NSLog(@"copying back to main memory...");
    gpgpuMngr->getOutputData(outputBuf);
    NSLog(@"done.");
    
#ifdef OGLES_GPGPU_BENCHMARK
    NSLog(@"Time measurements:");
    NSLog(@"---");
    vector<double> timeMeasurements = gpgpuMngr->getTimeMeasurements();
    for (vector<double>::iterator it = timeMeasurements.begin();
         it != timeMeasurements.end();
         ++it)
    {
        NSLog(@"> %0.9f ms", *it);
    }
    NSLog(@"---");
#endif
}

- (unsigned char *)uiImageToRGBABytes:(UIImage *)img {
    // get image information
    CGColorSpaceRef colorSpace = CGImageGetColorSpace(img.CGImage);
    
    const int w = [img size].width;
    const int h = [img size].height;
    
    // create the RGBA data buffer
    unsigned char *rgbaData = new unsigned char[w * h * 4];
    
    // create the CG context
    CGContextRef contextRef = CGBitmapContextCreate(rgbaData,
                                                    w, h,
                                                    8,
                                                    w * 4,
                                                    colorSpace,
                                                    kCGImageAlphaNoneSkipLast |
                                                    kCGBitmapByteOrderDefault);
    
    if (!contextRef) {
        delete rgbaData;
        
        return NULL;
    }
    
    // draw the image in the context
    CGContextDrawImage(contextRef, CGRectMake(0, 0, w, h), img.CGImage);
    
    CGContextRelease(contextRef);

    return rgbaData;
}

- (UIImage *)rgbaBytesToUIImage:(unsigned char *)rgbaData width:(int)w height:(int)h {
    // code from Patrick O'Keefe (http://www.patokeefe.com/archives/721)
    NSData *data = [NSData dataWithBytes:rgbaData length:w * h * 4];
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
    CGDataProviderRef provider = CGDataProviderCreateWithCFData((CFDataRef)data);
    
    // Creating CGImage from cv::Mat
    CGImageRef imageRef = CGImageCreate(w,                                  //width
                                        h,                                  //height
                                        8,                                  //bits per component
                                        8 * 4,                              //bits per pixel
                                        w * 4,                              //bytesPerRow
                                        colorSpace,                                 //colorspace
                                        kCGImageAlphaNone|kCGBitmapByteOrderDefault,// bitmap info
                                        provider,                                   //CGDataProviderRef
                                        NULL,                                       //decode
                                        false,                                      //should interpolate
                                        kCGRenderingIntentDefault                   //intent
                                        );
    
    
    // Getting UIImage from CGImage
    UIImage *finalImage = [UIImage imageWithCGImage:imageRef];
    CGImageRelease(imageRef);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colorSpace);
    
    return finalImage;
}

@end
