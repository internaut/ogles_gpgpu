/**
 * iOS App AppDelegate -- application lifecycle handling
 */

#import "AppDelegate.h"

#import "RootViewController.h"

@implementation AppDelegate

/**
 * Startup method
 */
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.window.backgroundColor = [UIColor whiteColor];
    
    // root view controller contains the main application logic
    RootViewController *rootViewCtrl = [[[RootViewController alloc] initWithNibName:nil bundle:nil] autorelease];
    
    [self.window setRootViewController:rootViewCtrl];
    
    [self.window makeKeyAndVisible];
    
    return YES;
}


@end
