/**
 * Common types.
 */

#ifndef OGLES_GPGPU_COMMON_TYPES
#define OGLES_GPGPU_COMMON_TYPES

namespace ogles_gpgpu {

typedef enum {
    RenderOrientationNone = -1,
    RenderOrientationStd = 0,
    RenderOrientationFlipped,
    RenderOrientationDiagonal
} RenderOrientation;

}

#endif