#include "disp.h"

using namespace std;
using namespace ogles_gpgpu;

const char *Disp::fshaderDispSrc = TO_STR(
precision mediump float;
varying vec2 vTexCoord;
uniform sampler2D uInputTex;
void main() {
    gl_FragColor = vec4(texture2D(uInputTex, vTexCoord).rgba);
//    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
);

void Disp::init(int inW, int inH, unsigned int order, bool prepareForExternalInput) {
    cout << "ogles_gpgpu::Disp - init" << endl;
    
    // parent init - set defaults
    ProcBase::baseInit(inW, inH, order, prepareForExternalInput, procParamOutW, procParamOutH, procParamOutScale);
    
    // create shader object
    ProcBase::createShader(ProcBase::vshaderDefault, fshaderDispSrc);
    
    // get shader params
    shParamAPos = shader->getParam(ATTR, "aPos");
	shParamATexCoord = shader->getParam(ATTR, "aTexCoord");
    shParamUInputTex = shader->getParam(UNIF, "uInputTex");
    
	// set geometry
	memcpy(vertexBuf, ProcBase::quadVertices,
           OGLES_GPGPU_QUAD_VERTEX_BUFSIZE * sizeof(GLfloat));
    
	// set texture coordinates
    initTexCoordBuf(texCoordBuf);
}

void Disp::render() {
    cout << "ogles_gpgpu::Disp - input tex " << texId << " / render to framebuffer of size " << outFrameW << "x" << outFrameH << endl;
    
	shader->use();
    
	// set the viewport (will effectively scale down)
	glViewport(0, 0, outFrameW, outFrameH);
    
	glClear(GL_COLOR_BUFFER_BIT);
    
	// set texture
    glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, texId);	// bind input texture
    
	// set uniforms
    glUniform1i(shParamUInputTex, texUnit);
    Tools::checkGLErr("ogles_gpgpu::Disp - set uniforms");
    
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
    
    Tools::checkGLErr("ogles_gpgpu::Disp - render");
    
	// cleanup
	glDisableVertexAttribArray(shParamAPos);
	glDisableVertexAttribArray(shParamATexCoord);
}

void Disp::setOutputRenderOrientation(RenderOrientation o) {
    renderOrientation = o;
    
	// set texture coordinates again
    initTexCoordBuf(texCoordBuf);
}
