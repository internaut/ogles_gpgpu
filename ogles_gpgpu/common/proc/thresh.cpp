#include "thresh.h"

#include "common_shaders.h"

using namespace std;
using namespace ogles_gpgpu;

ThreshProc::ThreshProc() {
    threshType = SIMPLE;
    threshVal = 0.5f;
    pxDx = pxDy = 0.0f;
}

void ThreshProc::init(int inW, int inH, unsigned int order) {
    cout << "ogles_gpgpu::ThreshProc - init" << endl;
    
    // parent init - set defaults
    ProcBase::baseInit(inW, inH, order, procParamOutW, procParamOutH, procParamOutScale);
    
    if (threshType == ADAPTIVE_PASS_1 || threshType == ADAPTIVE_PASS_2) {
        // calculate pixel delta values
        pxDx = 1.0f / (float)outFrameW;
        pxDy = 1.0f / (float)outFrameH;
    }
    
    // create fbo
    ProcBase::createFBO();
    if (threshType == ADAPTIVE_PASS_1) {
        fbo->createAttachedTex(outFrameH, outFrameW);   // swapped
    } else {
        fbo->createAttachedTex(outFrameW, outFrameH);
    }
    
    // create shader object
    const char *shSrc = NULL;
    if (threshType == SIMPLE) {
        shSrc = OGLES_GPGPU_COMMON_PROC_SIMPLE_THRESH_FSHADER;
    } else if (threshType == ADAPTIVE_PASS_1) {
        shSrc = OGLES_GPGPU_COMMON_PROC_ADAPT_THRESH1_FSHADER;
    } else if (threshType == ADAPTIVE_PASS_2) {
        shSrc = OGLES_GPGPU_COMMON_PROC_ADAPT_THRESH2_FSHADER;
    }
    
    ProcBase::createShader(OGLES_GPGPU_DEFAULT_VSHADER, shSrc);
    
    // get shader params
    shParamAPos = shader->getParam(ATTR, "aPos");
	shParamATexCoord = shader->getParam(ATTR, "aTexCoord");
    shParamUInputTex = shader->getParam(UNIF, "uInputTex");
    
    if (threshType == SIMPLE) {
        shParamUThresh = shader->getParam(UNIF, "uThresh");
    } else {
        shParamUPxD = shader->getParam(UNIF, "uPxD");
    }
    
	// set geometry
	memcpy(vertexBuf, ProcBase::quadVertices,
           OGLES_GPGPU_QUAD_VERTEX_BUFSIZE * sizeof(GLfloat));
    
	// set texture coordinates
    if (threshType == SIMPLE) {
        memcpy(texCoordBuf, ProcBase::quadTexCoordsStd,
               OGLES_GPGPU_QUAD_TEX_BUFSIZE * sizeof(GLfloat));
    } else {
        memcpy(texCoordBuf, ProcBase::quadTexCoordsDiagonal,
               OGLES_GPGPU_QUAD_TEX_BUFSIZE * sizeof(GLfloat));
    }
}

void ThreshProc::render() {
    cout << "ogles_gpgpu::ThreshProc - render (thresh type " << threshType << ")" << endl;
    
	shader->use();
    
	// set texture
    glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, texId);	// bind input texture
    
	// set uniforms
    glUniform1i(shParamUInputTex, texUnit);
    
	if (threshType == SIMPLE) {
		glUniform1f(shParamUThresh, threshVal);	// thresholding value for simple thresholding
	} else {
		glUniform2f(shParamUPxD, pxDx, pxDy);	// texture pixel delta values
	}
    
	// render to FBO
	fbo->bind();
    
	// set the viewport
	if (threshType == ADAPTIVE_PASS_1) {
		glViewport(0, 0, outFrameH, outFrameW);	// swapped
	} else {
		glViewport(0, 0, outFrameW, outFrameH);
	}
    
	glClear(GL_COLOR_BUFFER_BIT);
    
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
    
	// draw
	glDrawArrays(GL_TRIANGLE_STRIP, 0, OGLES_GPGPU_QUAD_VERTICES);
    
    Tools::checkGLErr("ogles_gpgpu::ThreshProc - render");
    
	// cleanup
	glDisableVertexAttribArray(shParamAPos);
	glDisableVertexAttribArray(shParamATexCoord);
    
	fbo->unbind();
}