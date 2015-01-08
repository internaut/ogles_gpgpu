//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * Common types and definitions.
 */

#ifndef OGLES_GPGPU_COMMON_TYPES
#define OGLES_GPGPU_COMMON_TYPES

namespace ogles_gpgpu {

typedef enum {
    RenderOrientationNone = -1,
    RenderOrientationStd = 0,
    RenderOrientationStdMirrored,
    RenderOrientationFlipped,
    RenderOrientationFlippedMirrored,
    RenderOrientationDiagonal
} RenderOrientation;

}

#endif