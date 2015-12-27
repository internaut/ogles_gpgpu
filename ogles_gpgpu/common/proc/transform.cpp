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
}

int TransformProc::init(int inW, int inH, unsigned int order, bool prepareForExternalInput)
{
    OG_LOGINF(getProcName(), "initialize");

    // create fbo for output
    createFBO();

    // ProcBase init - set defaults
    baseInit(inW, inH, order, prepareForExternalInput, procParamOutW, procParamOutH, procParamOutScale);

    // FilterProcBase init - create shaders, get shader params, set buffers for OpenGL
    filterInit(vshaderTransformSrc, fshaderTransformSrc);
    
    return 1;
}

void TransformProc::render()
{
    OG_LOGINF(getProcName(), "input tex %d, target %d, framebuffer of size %dx%d", texId, texTarget, outFrameW, outFrameH);

    filterRenderPrepare();
    Tools::checkGLErr(getProcName(), "render prepare");
    
    glUniformMatrix4fv(shParamUTransform, 1, 0, &transformMatrix.data[0][0]);        // set additional uniforms
    Tools::checkGLErr(getProcName(), "glUniformMatrix4fv");

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

