//
//  RootViewController.m
//  OGLESGPGPUGrayscale
//
//  Created by Markus Konrad on 30.10.14.
//  Copyright (c) 2014 INKA Research Group. All rights reserved.
//

#import "RootViewController.h"

static NSArray *availableTestImages = [NSArray arrayWithObjects:
                                       @"moon_1024x512.png",
                                       @"moon_1024x1024.png",
                                       @"moon_2048x2048.png",
                                       @"building_2048x1536.jpg",
                                       nil];

@interface RootViewController ()

- (void)initUI;

- (void)initOGLESGPGPU;

- (void)testImgBtnPressedAction:(id)sender;

- (void)presentTestImg:(int)num forceDisplay:(BOOL)forceDisplay;

- (void)presentOutputImg;

- (void)runImgProcOnGPU;

- (unsigned char *)uiImageToRGBABytes:(UIImage *)img;

- (UIImage *)rgbaBytesToUIImage:(unsigned char *)data width:(int)w height:(int)h;

@end

@implementation RootViewController

#pragma mark init/dealloc

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
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
    
    // create buttons for the test images
    int i = 0;
    int btnW = 180;
    int btnMrgn = 10;
    for (NSString *testImgName in availableTestImages) {
        UIButton *btn = [UIButton buttonWithType:UIButtonTypeRoundedRect];
        [btn setTitle:[testImgName stringByDeletingPathExtension] forState:UIControlStateNormal];
        [btn setTag:i];
        [btn setBackgroundColor:[UIColor whiteColor]];
        [btn setFrame:CGRectMake(btnMrgn + (btnW + btnMrgn) * i, 20, btnW, 26)];
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
    gpgpuMngr->prepare(testImgW, testImgH);
    
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
    
    gpgpuMngr = ogles_gpgpu::Core::getInstance();
    
    gpgpuMngr->setUseMipmaps(false);
    
    grayscaleProc.setOutputSize(0.5f);
//    grayscaleProc.setOutputSize(1.0f);
    adaptThreshProc[0].setThreshType(ogles_gpgpu::THRESH_ADAPTIVE_PASS_1);
    adaptThreshProc[1].setThreshType(ogles_gpgpu::THRESH_ADAPTIVE_PASS_2);
    
    gpgpuMngr->addProcToPipeline(&grayscaleProc);
//    gpgpuMngr->addProcToPipeline(&simpleThreshProc);
    gpgpuMngr->addProcToPipeline(&adaptThreshProc[0]);
    gpgpuMngr->addProcToPipeline(&adaptThreshProc[1]);

    gpgpuMngr->init(true);
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
    vector<float> timeMeasurements = gpgpuMngr->getTimeMeasurements();
    for (vector<float>::iterator it = timeMeasurements.begin();
         it != timeMeasurements.end();
         ++it)
    {
        NSLog(@"> %f ms", *it);
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
