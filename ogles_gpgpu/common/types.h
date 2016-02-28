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

#include "common_includes.h"

namespace ogles_gpgpu {

typedef enum {
    RenderOrientationNone = -1,
    RenderOrientationStd = 0,
    RenderOrientationStdMirrored,
    RenderOrientationFlipped,
    RenderOrientationFlippedMirrored,
    RenderOrientationDiagonal,
    RenderOrientationDiagonalFlipped,
    RenderOrientationDiagonalMirrored
} RenderOrientation;
    
    
// Map camera orientation (in degrees) to RenderOrientation enum
inline RenderOrientation degreesToOrientation(int degrees)
{
    switch(degrees)
    {
        case 360:
        case 0: return ogles_gpgpu::RenderOrientationStd;
        case 90: return ogles_gpgpu::RenderOrientationDiagonalFlipped;
        case 180: return ogles_gpgpu::RenderOrientationFlipped;
        case 270: return ogles_gpgpu::RenderOrientationDiagonalMirrored;
        default: assert(false);
    }
    return ogles_gpgpu::RenderOrientationStd;
}
    
struct Size2d
{
    Size2d();
    Size2d(int width, int height);
    int width = 0, height = 0;
};

inline bool operator==(const Size2d& lhs, const Size2d& rhs)
{
    return lhs.width == rhs.width && lhs.height == rhs.height;
}

inline bool operator!=(const Size2d& lhs, const Size2d& rhs)
{
    return !(lhs == rhs);
}
    
struct Rect2d
{
    Rect2d() {}
    Rect2d(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}
    int x = 0, y = 0, width = 0, height = 0;
};
    
struct Vec3f
{
    GLfloat data[3];
};
    
struct Mat44f
{
    GLfloat data[4][4];
};

}

#endif
