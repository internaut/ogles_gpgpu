#include "core.h"

#import <CoreVideo/CVPixelBuffer.h> // CVPixelBufferRef
#import <OpenGLES/EAGL.h> // EAGLContext

namespace ogles_gpgpu {

void Core::printCVPixelBuffer(const char* tag, void* ptr)
{
    NSLog(@"[%s] %@", tag, (CVPixelBufferRef)ptr);
}

void* Core::getCurrentEAGLContext()
{
    return [EAGLContext currentContext];
}

} // namespace ogles_gpgpu
