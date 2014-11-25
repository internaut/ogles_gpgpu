#import "AppDelegate.h"

#import "RootViewController.h"

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

    RootViewController *rootViewCtrl = [[[RootViewController alloc] initWithNibName:nil bundle:nil] autorelease];
    [self.window setRootViewController:rootViewCtrl];
    
    self.window.backgroundColor = [UIColor whiteColor];
    [self.window makeKeyAndVisible];
    return YES;
}

@end
