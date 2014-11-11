#include "thresh.h"

using namespace std;
using namespace ogles_gpgpu;

// Simple thresholding fragment shader
// Requires a grayscale image as input!
const char *ThreshProc::fshaderSimpleThreshSrc = TO_STR(
precision mediump float;
varying vec2 vTexCoord;
uniform float uThresh;
uniform sampler2D uInputTex;
void main() {
    float gray = texture2D(uInputTex, vTexCoord).r;
    float bin = step(uThresh, gray);
    gl_FragColor = vec4(bin, bin, bin, 1.0);
}
);

// Adaptive thresholding - Pass 1 fragment shader
// Perform a vertical 5x1 average gray pixel value calculation
// Requires a grayscale image as input!
const char *ThreshProc::fshaderAdaptThreshPass1Src = TO_STR(
precision mediump float;
varying vec2 vTexCoord;
uniform vec2 uPxD;
uniform sampler2D uInputTex;
void main() {
    // get center pixel value
    float centerGray = texture2D(uInputTex, vTexCoord).r;
    // get the sum
    float sum = texture2D(uInputTex, vTexCoord + vec2(uPxD.x * -2.0, 0.0)).r +
        texture2D(uInputTex, vTexCoord + vec2(uPxD.x * -1.0, 0.0)).r +
        centerGray +
        texture2D(uInputTex, vTexCoord + vec2(uPxD.x *  1.0, 0.0)).r +
        texture2D(uInputTex, vTexCoord + vec2(uPxD.x *  2.0, 0.0)).r;
    // get the average
    float avg = sum / 5.0;
    // Result stores average pixel value (R) and original gray value (G)
    gl_FragColor = vec4(avg, centerGray, 0.0, 1.0);
}
);

// Adaptive thresholding - Pass 2
// Perform a horizontal 7x1 or 5x1 average gray pixel value calculation and
// the final binarization
const char *ThreshProc::fshaderAdaptThreshPass2Src = TO_STR(
precision mediump float;
varying vec2 vTexCoord;
uniform vec2 uPxD;
uniform sampler2D uInputTex;
void main() {
    vec4 centerPx = texture2D(uInputTex, vTexCoord); // stores: horizontal avg, orig. gray value, 0, 1
    const float bigC = 9.5 / 255.0;
    // get the sum
    float sum = texture2D(uInputTex, vTexCoord + vec2(uPxD.y * -2.0, 0.0)).r +
    texture2D(uInputTex, vTexCoord + vec2(uPxD.y * -1.0, 0.0)).r +
    centerPx.r +
    texture2D(uInputTex, vTexCoord + vec2(uPxD.y *  1.0, 0.0)).r +
    texture2D(uInputTex, vTexCoord + vec2(uPxD.y *  2.0, 0.0)).r;
    // get the average
    float avg = sum / 5.0;
    // create inverted binary value
    float bin = 1.0 - step(avg - bigC, centerPx.g); // centerPx.g is orig. gray value at current position
    // store thresholded values
    gl_FragColor = vec4(bin, bin, bin, 1.0);
}
);

ThreshProc::ThreshProc() {
    threshType = THRESH_SIMPLE;
    threshVal = 0.5f;
    pxDx = pxDy = 0.0f;
}

void ThreshProc::init(int inW, int inH, unsigned int order) {
    cout << "ogles_gpgpu::ThreshProc - init" << endl;
    
    // parent init - set defaults
    ProcBase::baseInit(inW, inH, order, procParamOutW, procParamOutH, procParamOutScale);
    
    if (threshType == THRESH_ADAPTIVE_PASS_1 || threshType == THRESH_ADAPTIVE_PASS_2) {
        // calculate pixel delta values
        pxDx = 1.0f / (float)outFrameW;
        pxDy = 1.0f / (float)outFrameH;
    }
    
    // create fbo
    ProcBase::createFBO();
    if (threshType == THRESH_ADAPTIVE_PASS_1) {
        fbo->createAttachedTex(outFrameH, outFrameW, willDownscale);   // swapped
    } else {
        fbo->createAttachedTex(outFrameW, outFrameH, willDownscale);
    }
    
    // create shader object
    const char *shSrc = NULL;
    if (threshType == THRESH_SIMPLE) {
        shSrc = fshaderSimpleThreshSrc;
    } else if (threshType == THRESH_ADAPTIVE_PASS_1) {
        shSrc = fshaderAdaptThreshPass1Src;
    } else if (threshType == THRESH_ADAPTIVE_PASS_2) {
        shSrc = fshaderAdaptThreshPass2Src;
    }
    
    ProcBase::createShader(ProcBase::vshaderDefault, shSrc);
    
    // get shader params
    shParamAPos = shader->getParam(ATTR, "aPos");
	shParamATexCoord = shader->getParam(ATTR, "aTexCoord");
    shParamUInputTex = shader->getParam(UNIF, "uInputTex");
    
    if (threshType == THRESH_SIMPLE) {
        shParamUThresh = shader->getParam(UNIF, "uThresh");
    } else {
        shParamUPxD = shader->getParam(UNIF, "uPxD");
    }
    
	// set geometry
	memcpy(vertexBuf, ProcBase::quadVertices,
           OGLES_GPGPU_QUAD_VERTEX_BUFSIZE * sizeof(GLfloat));
    
	// set texture coordinates
    if (threshType == THRESH_SIMPLE) {
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
    
	if (threshType == THRESH_SIMPLE) {
		glUniform1f(shParamUThresh, threshVal);	// thresholding value for simple thresholding
	} else {
		glUniform2f(shParamUPxD, pxDx, pxDy);	// texture pixel delta values
	}
    
	// render to FBO
	fbo->bind();
    
	// set the viewport
	if (threshType == THRESH_ADAPTIVE_PASS_1) {
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