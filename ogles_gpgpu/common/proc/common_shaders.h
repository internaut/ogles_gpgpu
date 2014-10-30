#ifndef OGLES_GPGPU_COMMON_PROC_COMMON_SHADERS
#define OGLES_GPGPU_COMMON_PROC_COMMON_SHADERS

#define OGLES_GPGPU_DEFAULT_VSHADER "\
attribute vec4 aPos; \
attribute vec2 aTexCoord; \
varying vec2 vTexCoord; \
void main() { \
    gl_Position = aPos; \
    vTexCoord = aTexCoord; \
} \
"

#endif