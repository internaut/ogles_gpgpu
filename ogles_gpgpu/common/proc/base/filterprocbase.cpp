//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "filterprocbase.h"

#include <memory.h> // for memcpy on linux

using namespace ogles_gpgpu;
using namespace std;

const char *FilterProcBase::vshaderGPUImage = OG_TO_STR(
 attribute vec4 position;
 attribute vec4 inputTextureCoordinate;
 
 varying vec2 textureCoordinate;
 void main()
 {
     gl_Position = position;
     textureCoordinate = inputTextureCoordinate.xy;
 }
);

const char *FilterProcBase::vshaderFilter3x3Src = OG_TO_STR(
                                                           
    attribute vec4 position;
    attribute vec4 inputTextureCoordinate;

    uniform float texelWidth;
    uniform float texelHeight;

    varying vec2 textureCoordinate;
    varying vec2 leftTextureCoordinate;
    varying vec2 rightTextureCoordinate;

    varying vec2 topTextureCoordinate;
    varying vec2 topLeftTextureCoordinate;
    varying vec2 topRightTextureCoordinate;

    varying vec2 bottomTextureCoordinate;
    varying vec2 bottomLeftTextureCoordinate;
    varying vec2 bottomRightTextureCoordinate;

    void main()
    {
        gl_Position = position;

        vec2 widthStep = vec2(texelWidth, 0.0);
        vec2 heightStep = vec2(0.0, texelHeight);
        vec2 widthHeightStep = vec2(texelWidth, texelHeight);
        vec2 widthNegativeHeightStep = vec2(texelWidth, -texelHeight);

        textureCoordinate = inputTextureCoordinate.xy;
        leftTextureCoordinate = inputTextureCoordinate.xy - widthStep;
        rightTextureCoordinate = inputTextureCoordinate.xy + widthStep;

        topTextureCoordinate = inputTextureCoordinate.xy - heightStep;
        topLeftTextureCoordinate = inputTextureCoordinate.xy - widthHeightStep;
        topRightTextureCoordinate = inputTextureCoordinate.xy + widthNegativeHeightStep;

        bottomTextureCoordinate = inputTextureCoordinate.xy + heightStep;
        bottomLeftTextureCoordinate = inputTextureCoordinate.xy - widthNegativeHeightStep;
        bottomRightTextureCoordinate = inputTextureCoordinate.xy + widthHeightStep;
});


const char *FilterProcBase::vshaderDefault = OG_TO_STR(
attribute vec4 aPos;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;
void main()
{
    gl_Position = aPos;
    vTexCoord = aTexCoord;
}
);

#pragma mark public methods

void FilterProcBase::setOutputRenderOrientation(RenderOrientation o) {
    ProcBase::setOutputRenderOrientation(o);

    initTexCoordBuf();
}

void FilterProcBase::useTexture(GLuint id, GLuint useTexUnit, GLenum target, int position) {
    texId = id;
    texUnit = useTexUnit;

    if (target != texTarget) {	// changed        
        if (fragShaderSrcForCompilation) {	// recreate shader with new texture target
            auto vShaderSrc  = vertexShaderSrcForCompilation ? vertexShaderSrcForCompilation : vshaderDefault;
            filterShaderSetup(vShaderSrc, fragShaderSrcForCompilation, target);
        }
        texTarget = target;
    }
}

#pragma mark protected methods

void FilterProcBase::filterInit(const char *vShaderSrc, const char *fShaderSrc, RenderOrientation o) {
    // create shader object
    filterShaderSetup(vShaderSrc, fShaderSrc, texTarget);

    // set geometry
    memcpy(vertexBuf, ProcBase::quadVertices, OGLES_GPGPU_QUAD_VERTEX_BUFSIZE * sizeof(GLfloat));

    // set texture coordinates
    initTexCoordBuf(o);
}

void FilterProcBase::filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target) {
    // create shader object
    ProcBase::createShader(vShaderSrc, fShaderSrc, target);

    // TODO: Support uniform reads through virtual API call:
    shParamAPos = shader->getParam(ATTR, "aPos");
    shParamATexCoord = shader->getParam(ATTR, "aTexCoord");
    shParamUInputTex = shader->getParam(UNIF, "uInputTex");
    
    // remember used shader source
    vertexShaderSrcForCompilation = vShaderSrc;
    fragShaderSrcForCompilation = fShaderSrc;
}

void FilterProcBase::getUniforms() {
    
}

/**
 * Render a result, i.e. run the shader on the input texture.
 * Abstract method.
 */
int FilterProcBase::render(int position) {
    OG_LOGINF(getProcName(), "input tex %d, target %d, framebuffer of size %dx%d", texId, texTarget, outFrameW, outFrameH);
    
    filterRenderPrepare();
    Tools::checkGLErr(getProcName(), "render prepare");

    setUniforms();
    Tools::checkGLErr(getProcName(), "setUniforms");
    
    filterRenderSetCoords();
    Tools::checkGLErr(getProcName(), "render set coords");
    
    filterRenderDraw();
    Tools::checkGLErr(getProcName(), "render draw");
    
    filterRenderCleanup();
    Tools::checkGLErr(getProcName(), "render cleanup");
    
    return 0;
}

/**
 * Initialize texture coordinate buffer according to member variable
 * <renderOrientation> or override member variable by <overrideRenderOrientation>.
 */
const GLfloat * FilterProcBase::getTexCoordBuf(RenderOrientation o) {
    const GLfloat *coordsPtr;
    
    switch (o) {
        default:
        case RenderOrientationStd:
            coordsPtr = ProcBase::quadTexCoordsStd;
            break;
        case RenderOrientationStdMirrored:
            coordsPtr = ProcBase::quadTexCoordsStdMirrored;
            break;
        case RenderOrientationFlipped:
            coordsPtr = ProcBase::quadTexCoordsFlipped;
            break;
        case RenderOrientationFlippedMirrored:
            coordsPtr = ProcBase::quadTexCoordsFlippedMirrored;
            break;
        case RenderOrientationDiagonal:
            coordsPtr = ProcBase::quadTexCoordsDiagonal;
            break;
        case RenderOrientationDiagonalFlipped:
            coordsPtr = ProcBase::quadTexCoordsDiagonalFlipped;
            break;
        case RenderOrientationDiagonalMirrored:
            coordsPtr = ProcBase::quadTexCoordsDiagonalMirrored;
            break;
    }
    
    return coordsPtr;
}

void FilterProcBase::initTexCoordBuf(RenderOrientation overrideRenderOrientation) {
    RenderOrientation o = (overrideRenderOrientation == RenderOrientationNone) ? renderOrientation : overrideRenderOrientation;
    memcpy(texCoordBuf, getTexCoordBuf(o), OGLES_GPGPU_QUAD_TEX_BUFSIZE * sizeof(GLfloat));
}

void FilterProcBase::filterRenderPrepare() {
    shader->use();
    Tools::checkGLErr(getProcName(), "shader->use()");

    // set the viewport
    glViewport(0, 0, outFrameW, outFrameH);

    glClear(GL_COLOR_BUFFER_BIT);

    assert(texTarget == GL_TEXTURE_2D); // texTarget = GL_TEXTURE_2D;
    
    // set input texture
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(texTarget, texId);	// bind input texture

    // set common uniforms
    glUniform1i(shParamUInputTex, texUnit);
}

void FilterProcBase::filterRenderSetCoords() {
    // render to FBO
    if (fbo) fbo->bind();

    // set geometry
    glEnableVertexAttribArray(shParamAPos);
    glVertexAttribPointer(shParamAPos,
                          OGLES_GPGPU_QUAD_COORDS_PER_VERTEX,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          vertexBuf);

    glVertexAttribPointer(shParamATexCoord,
                          OGLES_GPGPU_QUAD_TEXCOORDS_PER_VERTEX,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          texCoordBuf);
    glEnableVertexAttribArray(shParamATexCoord);
}

void FilterProcBase::filterRenderDraw() {
    // draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, OGLES_GPGPU_QUAD_VERTICES);
}

void FilterProcBase::filterRenderCleanup() {
    // cleanup
    glDisableVertexAttribArray(shParamAPos);
    glDisableVertexAttribArray(shParamATexCoord);

    if (fbo) fbo->unbind();
}

int FilterProcBase::init(int inW, int inH, unsigned int order, bool prepareForExternalInput) {
    OG_LOGINF(getProcName(), "initialize");
    
    // create fbo for output
    createFBO();
    
    // ProcBase init - set defaults
    baseInit(inW, inH, order, prepareForExternalInput, procParamOutW, procParamOutH, procParamOutScale);
    
    // FilterProcBase init - create shaders, get shader params, set buffers for OpenGL
    filterInit(getVertexShaderSource(), getFragmentShaderSource());

    // Get shader specific uniforms
    getUniforms();
    
    return 1;
}

