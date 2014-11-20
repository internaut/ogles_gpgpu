//
//  AppDelegate.m
//  OGLESGPGPUGrayscale
//
//  Created by Markus Konrad on 16.10.14.
//  Copyright (c) 2014 INKA Research Group. All rights reserved.
//

#import "AppDelegate.h"

#import "RootViewController.h"

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    // Override point for customization after application launch.
    self.window.backgroundColor = [UIColor whiteColor];
    
    RootViewController *rootViewCtrl = [[[RootViewController alloc] initWithNibName:nil bundle:nil] autorelease];
    
    [self.window setRootViewController:rootViewCtrl];
    
    [self.window makeKeyAndVisible];
    
    return YES;
}


@end
