//
//  RootViewController.m
//  OGLESGPGPUGrayscale
//
//  Created by Markus Konrad on 30.10.14.
//  Copyright (c) 2014 INKA Research Group. All rights reserved.
//

#import "RootViewController.h"

@interface RootViewController ()

@end

@implementation RootViewController

#pragma mark init/dealloc

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        [self becomeFirstResponder];
    }
    return self;
}

- (void)dealloc {
    [baseView release];
    
    [super dealloc];
}

#pragma mark event handling

- (void)viewDidLoad
{
    [super viewDidLoad];

    const CGRect screenRect = [[UIScreen mainScreen] bounds];
    
    NSLog(@"loading view of size %dx%d", (int)screenRect.size.width, (int)screenRect.size.height);
    
    // create an empty base view
    CGRect baseFrame = CGRectMake(0, 0, screenRect.size.height, screenRect.size.width);
    baseView = [[UIView alloc] initWithFrame:baseFrame];
    
    // finally set the base view as view for this controller
    [self setView:baseView];
}

- (void)touchesBegan:(NSSet *)touches
           withEvent:(UIEvent *)event
{
    NSLog(@"touch on root view controller");
}

@end
