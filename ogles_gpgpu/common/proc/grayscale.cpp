#include "grayscale.h"

using namespace std;
using namespace ogles_gpgpu;

const GLfloat GrayscaleProc::grayscaleConvVecRGB[3] = {
    0.299, 0.587, 0.114
};

const GLfloat GrayscaleProc::grayscaleConvVecBGR[3] = {
    0.114, 0.587, 0.299
};

const char *GrayscaleProc::fshaderGrayscaleSrc = TO_STR(
precision mediump float;
varying vec2 vTexCoord;
uniform sampler2D uInputTex;
uniform vec3 uInputConvVec;
void main() {
    float gray = dot(texture2D(uInputTex, vTexCoord).rgb, uInputConvVec);
    gl_FragColor = vec4(gray, gray, gray, 1.0);
}
);

GrayscaleProc::GrayscaleProc() {
    // set defaults
    inputConvType = GRAYSCALE_INPUT_CONVERSION_NONE;
    setGrayscaleConvType(GRAYSCALE_INPUT_CONVERSION_RGB);
}

void GrayscaleProc::init(int inW, int inH, unsigned int order, bool prepareForExternalInput) {
    cout << "ogles_gpgpu::GrayscaleProc - init" << endl;
    
    // create fbo
    ProcBase::createFBO();
    
    // parent init - set defaults
    ProcBase::baseInit(inW, inH, order, prepareForExternalInput, procParamOutW, procParamOutH, procParamOutScale);
    
    // create shader object
    ProcBase::createShader(ProcBase::vshaderDefault, fshaderGrayscaleSrc);
    
    // get shader params
    shParamAPos = shader->getParam(ATTR, "aPos");
	shParamATexCoord = shader->getParam(ATTR, "aTexCoord");
    shParamUInputTex = shader->getParam(UNIF, "uInputTex");
    shParamUInputConvVec = shader->getParam(UNIF, "uInputConvVec");
    
	// set geometry
	memcpy(vertexBuf, ProcBase::quadVertices,
           OGLES_GPGPU_QUAD_VERTEX_BUFSIZE * sizeof(GLfloat));
    
	// set texture coordinates
    initTexCoordBuf(texCoordBuf);
}

void GrayscaleProc::render() {
    cout << "ogles_gpgpu::GrayscaleProc - input tex " << texId << " / render to framebuffer of size " << outFrameW << "x" << outFrameH << endl;
    
	shader->use();
    
	// set the viewport (will effectively scale down)
	glViewport(0, 0, outFrameW, outFrameH);
    
	glClear(GL_COLOR_BUFFER_BIT);
    
	// set texture
    glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, texId);	// bind input texture
    
	// set uniforms
    glUniform1i(shParamUInputTex, texUnit);
    glUniform3fv(shParamUInputConvVec, 1, grayscaleConvVec);
    Tools::checkGLErr("ogles_gpgpu::GrayscaleProc - set uniforms");
    
	// render to FBO
	fbo->bind();
    
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
    
    Tools::checkGLErr("ogles_gpgpu::GrayscaleProc - render");
    
	// cleanup
	glDisableVertexAttribArray(shParamAPos);
	glDisableVertexAttribArray(shParamATexCoord);
    
	fbo->unbind();

}

void GrayscaleProc::setGrayscaleConvVec(const GLfloat v[3]) {
    inputConvType = GRAYSCALE_INPUT_CONVERSION_CUSTOM;
    memcpy(grayscaleConvVec, v, sizeof(GLfloat) * 3);
}

void GrayscaleProc::setGrayscaleConvType(GrayscaleInputConversionType type) {
    if (inputConvType == type) return;  // no change
    
    const GLfloat *v = NULL;
    
    if (type == GRAYSCALE_INPUT_CONVERSION_RGB) {
        v = &grayscaleConvVecRGB[0];
    } else if (type == GRAYSCALE_INPUT_CONVERSION_BGR) {
        v = &grayscaleConvVecBGR[0];
    } else {
        cerr << "ogles_gpgpu::GrayscaleProc - unknown grayscale input conversion type " << type << endl;
        v = &grayscaleConvVecRGB[0];    // set default
    }
    
    memcpy(grayscaleConvVec, v, sizeof(GLfloat) * 3);
    
    inputConvType = type;
}