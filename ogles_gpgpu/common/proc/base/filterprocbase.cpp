//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "filterprocbase.h"

using namespace ogles_gpgpu;
using namespace std;

const char *FilterProcBase::vshaderDefault = OG_TO_STR (
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

void FilterProcBase::useTexture(GLuint id, GLuint useTexUnit, GLenum target) {
	texId = id;
	texUnit = useTexUnit;
	
	if (target != texTarget) {	// changed
		if (fragShaderSrcForCompilation) {	// recreate shader with new texture target
			filterShaderSetup(fragShaderSrcForCompilation, target);
		}
		
		texTarget = target;
	}
}

#pragma mark protected methods

void FilterProcBase::filterInit(const char *fShaderSrc, RenderOrientation o) {
	// create shader object
	filterShaderSetup(fShaderSrc, texTarget);
    
	// set geometry
	memcpy(vertexBuf, ProcBase::quadVertices, OGLES_GPGPU_QUAD_VERTEX_BUFSIZE * sizeof(GLfloat));
    
	// set texture coordinates
    initTexCoordBuf(o);
}

void FilterProcBase::filterShaderSetup(const char *fShaderSrc, GLenum target) {
    // create shader object
    ProcBase::createShader(FilterProcBase::vshaderDefault, fShaderSrc, target);
    
    // get shader params
    shParamAPos = shader->getParam(ATTR, "aPos");
	shParamATexCoord = shader->getParam(ATTR, "aTexCoord");
    shParamUInputTex = shader->getParam(UNIF, "uInputTex");
    
    // remember used shader source
    fragShaderSrcForCompilation = fShaderSrc;
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