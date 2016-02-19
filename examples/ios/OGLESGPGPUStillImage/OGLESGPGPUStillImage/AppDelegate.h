//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * iOS App AppDelegate -- application lifecycle handling
 */

#import <UIKit/UIKit.h>

#include "../../../../ogles_gpgpu/ogles_gpgpu.h"

/**
 * Application lifecycle handling
 */
@interface AppDelegate : UIResponder <UIApplicationDelegate>

    @property (strong, nonatomic) UIWindow *window; // the only application window

@end
