#include "filterprocbase.h"

using namespace ogles_gpgpu;
using namespace std;

const char *FilterProcBase::vshaderDefault = TO_STR (
attribute vec4 aPos;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;
void main() {
    gl_Position = aPos;
    vTexCoord = aTexCoord;
}
);

#pragma mark public methods

void FilterProcBase::setOutputRenderOrientation(RenderOrientation o) {
    ProcBase::setOutputRenderOrientation(o);
    
    initTexCoordBuf();
}

#pragma mark protected methods

void FilterProcBase::filterInit(const char *fShaderSrc, RenderOrientation o) {
    // create shader object
    ProcBase::createShader(FilterProcBase::vshaderDefault, fShaderSrc);
    
    // get shader params
    shParamAPos = shader->getParam(ATTR, "aPos");
	shParamATexCoord = shader->getParam(ATTR, "aTexCoord");
    shParamUInputTex = shader->getParam(UNIF, "uInputTex");
    
	// set geometry
	memcpy(vertexBuf, ProcBase::quadVertices, OGLES_GPGPU_QUAD_VERTEX_BUFSIZE * sizeof(GLfloat));
    
	// set texture coordinates
    initTexCoordBuf(o);
}

void FilterProcBase::initTexCoordBuf(RenderOrientation overrideRenderOrientation) {
    RenderOrientation o = overrideRenderOrientation == RenderOrientationNone ?
                          renderOrientation : overrideRenderOrientation;
    
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
    }
    
	memcpy(texCoordBuf, coordsPtr, OGLES_GPGPU_QUAD_TEX_BUFSIZE * sizeof(GLfloat));
}

void FilterProcBase::filterRenderPrepare() {
	shader->use();
    
	// set the viewport
	glViewport(0, 0, outFrameW, outFrameH);
    
	glClear(GL_COLOR_BUFFER_BIT);
    
	// set input texture
    glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, texId);	// bind input texture
    
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