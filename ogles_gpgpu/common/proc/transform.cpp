//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "../common_includes.h"
#include "transform.h"

using namespace std;
using namespace ogles_gpgpu;

const char *TransformProc::vshaderTransformSrc = OG_TO_STR(
attribute vec4 aPos;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;
uniform mat4 transformMatrix;
void main()
{
    gl_Position = transformMatrix * vec4(aPos.xyz, 1.0);
    vTexCoord = aTexCoord;
}
);

const char *TransformProc::fshaderTransformSrc = OG_TO_STR(

#if defined(OGLES_GPGPU_OPENGLES)
precision mediump float;
#endif

varying vec2 vTexCoord;
uniform sampler2D uInputTex;
void main()
{
    gl_FragColor = vec4(texture2D(uInputTex, vTexCoord).rgba);
}
);

// Bicubic interpolation from here:
// http://www.java-gaming.org/index.php?topic=35123.0
// theagentd/Myomyomyo

const char *TransformProc::fshaderTransformBicubicSrc = OG_TO_STR(

#if defined(OGLES_GPGPU_OPENGLES)
precision highp float;
#endif

vec4 cubic(OGLES_GPGPU_HIGHP float v)
{
    vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
    vec4 s = n * n * n;
    float x = s.x;
    float y = s.y - 4.0 * s.x;
    float z = s.z - 4.0 * s.y + 6.0 * s.x;
    float w = 6.0 - x - y - z;
    vec4 result = vec4(x, y, z, w) * (1.0/6.0);
    return result;
}

vec4 textureBicubic(sampler2D sampler, OGLES_GPGPU_HIGHP vec2 texCoords, OGLES_GPGPU_HIGHP vec2 texSize)
{
    vec2 invTexSize = 1.0 / texSize;
    
    texCoords = texCoords * texSize - 0.5;
    
    vec2 fxy = fract(texCoords);
    texCoords -= fxy;
    
    vec4 xcubic = cubic(fxy.x);
    vec4 ycubic = cubic(fxy.y);
    
    vec4 c = texCoords.xxyy + vec2(-0.5, +1.5).xyxy;
    
    vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset = c + vec4(xcubic.yw, ycubic.yw) / s;
    
    offset *= invTexSize.xxyy;
    
    vec4 sample0 = texture2D(sampler, offset.xz);
    vec4 sample1 = texture2D(sampler, offset.yz);
    vec4 sample2 = texture2D(sampler, offset.xw);
    vec4 sample3 = texture2D(sampler, offset.yw);
   
    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);
   
   return mix(mix(sample3, sample2, sx), mix(sample1, sample0, sx), sy);
}

uniform vec2 texSize;
uniform sampler2D uInputTex;
varying vec2 vTexCoord;

void main()
{
   gl_FragColor = textureBicubic(uInputTex, vTexCoord, texSize);
}
);

TransformProc::TransformProc() {
    memset(transformMatrix.data, 0, sizeof(transformMatrix.data));
    for(int i = 0; i < 4; i++) {
        transformMatrix.data[i][i] = 1;
    }
}


const char *TransformProc::getFragmentShaderSource() {
    return (interpolation == BILINEAR) ? fshaderTransformSrc : fshaderTransformBicubicSrc;
}

const char *TransformProc::getVertexShaderSource() {
    return vshaderTransformSrc;
}

void TransformProc::setUniforms() {
    FilterProcBase::setUniforms();
    glUniformMatrix4fv(shParamUTransform, 1, 0, &transformMatrix.data[0][0]);
    if(interpolation == BICUBIC) {
        glUniform2f(shParamUTransformSize, inFrameW, inFrameH);
    }
}

void TransformProc::getUniforms() {
    FilterProcBase::getUniforms();
    shParamUTransform = shader->getParam(UNIF, "transformMatrix");
    if(interpolation == BICUBIC) {
        shParamUTransformSize = shader->getParam(UNIF, "texSize");
    }
}

void TransformProc::setTransformMatrix(const Mat44f &matrix) {
    transformMatrix = matrix;
}

