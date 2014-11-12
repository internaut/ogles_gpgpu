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

- (void)runGrayscaleConvertOnGPU;

- (unsigned char *)uiImageToRGBABytes:(UIImage *)img;

- (UIImage *)rgbaBytesToUIImage:(unsigned char *)data width:(int)w height:(int)h;

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
    if (testImgData) delete [] testImgData;
    if (outputBuf) delete [] outputBuf;
    
    [testImg release];
    [outputImg release];
    
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
    
    // load test image
    NSString *testImgFile = @"moon_1024x512.png";
//    NSString *testImgFile = @"building_2048x1536.jpg";
//    NSString *testImgFile = @"moon_2048x2048.png";
    testImg = [[UIImage imageNamed:testImgFile] retain];
    testImgW = (int)testImg.size.width;
    testImgH = (int)testImg.size.height;
    
    if (testImg) {
        NSLog(@"loaded test image %@ with size %dx%d", testImgFile, testImgW, testImgH);
    } else {
        NSLog(@"could not load test image %@", testImgFile);
    }
    
    // get the RGBA bytes of the image
    testImgData = [self uiImageToRGBABytes:testImg];
    
    if (!testImgData) {
        NSLog(@"could not get RGBA data from test image %@", testImgFile);
    }

    // init UI
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
    [imgView setImage:testImg];
    [baseView addSubview:imgView];
    
    // finally set the base view as view for this controller
    [self setView:baseView];
    
    [self initOGLESGPGPU];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    NSLog(@"touch on root view controller");
    
    [self runGrayscaleConvertOnGPU];
}

#pragma mark private methods

- (void)initOGLESGPGPU {
    NSLog(@"initializing ogles_gpgpu");
    
    grayscaleProc.setOutputSize(0.5f);
//    grayscaleProc.setOutputSize(1.0f);
    adaptThreshProc[0].setThreshType(ogles_gpgpu::THRESH_ADAPTIVE_PASS_1);
    adaptThreshProc[1].setThreshType(ogles_gpgpu::THRESH_ADAPTIVE_PASS_2);
    gpgpuMngr.addProcToPipeline(&grayscaleProc);
//    gpgpuMngr.addProcToPipeline(&simpleThreshProc);
    gpgpuMngr.addProcToPipeline(&adaptThreshProc[0]);
    gpgpuMngr.addProcToPipeline(&adaptThreshProc[1]);

    gpgpuMngr.init(testImgW, testImgH, true);
    
    outputBuf = new unsigned char[gpgpuMngr.getOutputFrameW() * gpgpuMngr.getOutputFrameH() * 4];
}

- (void)runGrayscaleConvertOnGPU {
    NSLog(@"copying image to GPU...");
    gpgpuMngr.setInputData(testImgData);
    NSLog(@"converting...");
    gpgpuMngr.process();
    NSLog(@"copying back to main memory...");
    gpgpuMngr.getOutputData(outputBuf);
    NSLog(@"done.");
    
    [outputImg release];
    outputImg = [[self rgbaBytesToUIImage:outputBuf
                                    width:gpgpuMngr.getOutputFrameW()
                                   height:gpgpuMngr.getOutputFrameH()] retain];
    if (!outputImg) {
        NSLog(@"error converting output RGBA data to UIImage");
    } else {
        NSLog(@"presenting output image of size %dx%d", (int)outputImg.size.width, (int)outputImg.size.height);
    }
    
//    [imgView setFrame:CGRectMake(0, 0, outputImg.size.width, outputImg.size.height)];
    
    [imgView setImage:outputImg];
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
