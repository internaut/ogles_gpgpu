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
});

const char *TransformProc::fshaderTransformSrc = OG_TO_STR(

#if defined(OGLES_GPGPU_OPENGLES)
precision mediump float;
#endif

varying vec2 vTexCoord;
uniform sampler2D uInputTex;
void main()
{
    gl_FragColor = vec4(texture2D(uInputTex, vTexCoord).rgba);
});

// Bicubic interpolation from here:
// http://www.java-gaming.org/index.php?topic=35123.0
// theagentd/Myomyomyo

const char *TransformProc::fshaderTransformBicubicSrc = OG_TO_STR(
highp vec4 cubic(highp float v)
{
   highp vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
   highp vec4 s = n * n * n;
   highp float x = s.x;
   highp float y = s.y - 4.0 * s.x;
   highp float z = s.z - 4.0 * s.y + 6.0 * s.x;
   highp float w = 6.0 - x - y - z;
   highp vec4 result = vec4(x, y, z, w) * (1.0/6.0);
   return result;
}

highp vec4 textureBicubic(sampler2D sampler, highp vec2 texCoords, highp vec2 texSize)
{
   highp vec2 invTexSize = 1.0 / texSize;
   
   texCoords = texCoords * texSize - 0.5;
   
   highp vec2 fxy = fract(texCoords);
   texCoords -= fxy;
   
   highp vec4 xcubic = cubic(fxy.x);
   highp vec4 ycubic = cubic(fxy.y);
   
   highp vec4 c = texCoords.xxyy + vec2(-0.5, +1.5).xyxy;
   
   highp vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
   highp vec4 offset = c + vec4(xcubic.yw, ycubic.yw) / s;
   
   offset *= invTexSize.xxyy;
   
   highp vec4 sample0 = texture2D(sampler, offset.xz);
   highp vec4 sample1 = texture2D(sampler, offset.yz);
   highp vec4 sample2 = texture2D(sampler, offset.xw);
   highp vec4 sample3 = texture2D(sampler, offset.yw);
   
   highp float sx = s.x / (s.x + s.y);
   highp float sy = s.z / (s.z + s.w);
   
   return mix(mix(sample3, sample2, sx), mix(sample1, sample0, sx), sy);
}

uniform highp vec2 texSize;
uniform sampler2D uInputTex;
varying highp vec2 vTexCoord;

void main()
{
   gl_FragColor = textureBicubic(uInputTex, vTexCoord, texSize);
   //gl_FragColor = texture2D(uInputTex, vTexCoord);
});

TransformProc::TransformProc()
{
    memset(transformMatrix.data, 0, sizeof(transformMatrix.data));
    for(int i = 0; i < 4; i++)
    {
        transformMatrix.data[i][i] = 1;
    }
}

void TransformProc::filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target)
{
    FilterProcBase::filterShaderSetup(vShaderSrc, fShaderSrc, target);
    Tools::checkGLErr(getProcName(), "filterShaderSetup");
    
    shParamUTransform = shader->getParam(UNIF, "transformMatrix");
    Tools::checkGLErr(getProcName(), "getParam()");
    
    if(interpolation == BICUBIC)
    {
        shParamUTransformSize = shader->getParam(UNIF, "texSize");
        Tools::checkGLErr(getProcName(), "getParam()");
    }
}

int TransformProc::init(int inW, int inH, unsigned int order, bool prepareForExternalInput)
{
    OG_LOGINF(getProcName(), "initialize");

    // create fbo for output
    createFBO();

    // ProcBase init - set defaults
    baseInit(inW, inH, order, prepareForExternalInput, procParamOutW, procParamOutH, procParamOutScale);

    // FilterProcBase init - create shaders, get shader params, set buffers for OpenGL
    auto fShaderSrc = (interpolation == BILINEAR) ? fshaderTransformSrc : fshaderTransformBicubicSrc;
    filterInit(vshaderTransformSrc, fShaderSrc);
    
    return 1;
}

void TransformProc::render()
{
    OG_LOGINF(getProcName(), "input tex %d, target %d, framebuffer of size %dx%d", texId, texTarget, outFrameW, outFrameH);

    filterRenderPrepare();
    Tools::checkGLErr(getProcName(), "render prepare");
    
    glUniformMatrix4fv(shParamUTransform, 1, 0, &transformMatrix.data[0][0]);        // set additional uniforms
    Tools::checkGLErr(getProcName(), "glUniformMatrix4fv");
    
    if(interpolation == BICUBIC)
    {
        glUniform2f(shParamUTransformSize, inFrameW, inFrameH);
        Tools::checkGLErr(getProcName(), "glUniformMatrix2fv");
    }

    filterRenderSetCoords();
    Tools::checkGLErr(getProcName(), "render set coords");

    filterRenderDraw();
    Tools::checkGLErr(getProcName(), "render draw");

    filterRenderCleanup();
    Tools::checkGLErr(getProcName(), "render cleanup");
}

void TransformProc::setTransformMatrix(const Mat44f &matrix)
{
    transformMatrix = matrix;
}

