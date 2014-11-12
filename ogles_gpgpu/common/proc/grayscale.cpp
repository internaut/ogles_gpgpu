#include "grayscale.h"

using namespace std;
using namespace ogles_gpgpu;

const char *GrayscaleProc::fshaderGrayscaleSrc = TO_STR(
precision mediump float;
varying vec2 vTexCoord;
uniform sampler2D uInputTex;
const vec3 rgb2gray = vec3(0.299, 0.587, 0.114);
void main() {
    float gray = dot(texture2D(uInputTex, vTexCoord).rgb, rgb2gray);
    gl_FragColor = vec4(gray, gray, gray, 1.0);
}
);

void GrayscaleProc::init(int inW, int inH, unsigned int order) {
    cout << "ogles_gpgpu::GrayscaleProc - init" << endl;
    
    // parent init - set defaults
    ProcBase::baseInit(inW, inH, order, procParamOutW, procParamOutH, procParamOutScale);
    
    // create fbo
    ProcBase::createFBO();
    
    // create shader object
    ProcBase::createShader(ProcBase::vshaderDefault, fshaderGrayscaleSrc);
    
    // get shader params
    shParamAPos = shader->getParam(ATTR, "aPos");
	shParamATexCoord = shader->getParam(ATTR, "aTexCoord");
    shParamUInputTex = shader->getParam(UNIF, "uInputTex");
    
	// set geometry
	memcpy(vertexBuf, ProcBase::quadVertices,
           OGLES_GPGPU_QUAD_VERTEX_BUFSIZE * sizeof(GLfloat));
    
	// set texture coordinates
	memcpy(texCoordBuf, ProcBase::quadTexCoordsStd,
		   OGLES_GPGPU_QUAD_TEX_BUFSIZE * sizeof(GLfloat));
}

void GrayscaleProc::render() {
    cout << "ogles_gpgpu::GrayscaleProc - render to framebuffer of size " << outFrameW << "x" << outFrameH << endl;
    
	shader->use();
    
	// set the viewport (will effectively scale down)
	glViewport(0, 0, outFrameW, outFrameH);
    
	glClear(GL_COLOR_BUFFER_BIT);
    
	// set texture
    glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, texId);	// bind input texture
    
	// set uniforms
    glUniform1i(shParamUInputTex, texUnit);
    
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