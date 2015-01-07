//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "shader.h"

using namespace std;
using namespace ogles_gpgpu;

Shader::Shader() {
	programId = 0;
}

Shader::~Shader() {
	if (programId > 0) {
        OG_LOGINF("Shader", "deleting shader program");
		glDeleteProgram(programId);
	}
}

bool Shader::buildFromSrc(const char *vshSrc, const char *fshSrc) {
	programId = create(vshSrc, fshSrc, &vshId, &fshId);
    
	return (programId > 0);
}

void Shader::use() {
	glUseProgram(programId);
}

GLint Shader::getParam(ShaderParamType type, const char *name) const {
    // get position according to type and name
	GLint id = (type == ATTR) ?
        glGetAttribLocation(programId, name) :
        glGetUniformLocation(programId, name);
    
	if (id < 0) {
        OG_LOGERR("Shader", "could not get parameter id for param %s", name);
	}
    
	return id;
}

GLuint Shader::create(const char *vshSrc, const char *fshSrc, GLuint *vshId, GLuint *fshId) {
    // compile shaders for full shader program
	*vshId = compile(GL_VERTEX_SHADER, vshSrc);
	*fshId = compile(GL_FRAGMENT_SHADER, fshSrc);
    
    // create shader program
	GLuint programId = glCreateProgram();
    
	if (programId == 0) {
        OG_LOGERR("Shader", "could not create shader program");
		return 0;
	}
    
	glAttachShader(programId, *vshId);   // add the vertex shader to program
	glAttachShader(programId, *fshId);   // add the fragment shader to program
	glLinkProgram(programId);   // link both shaders to a full program
    
	// check link status
	GLint linkStatus;
	glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE) {
        OG_LOGERR("Shader", "could not link shader program. error log:");
		GLchar infoLogBuf[1024];
		GLsizei infoLogLen;
		glGetProgramInfoLog(programId, 1024, &infoLogLen, infoLogBuf);
        cerr << infoLogBuf << endl << endl;
        
		glDeleteProgram(programId);
        
		return 0;
	}
    
	return programId;
}

GLuint Shader::compile(GLenum type, const char *src) {
    // create a shader
	GLuint shId = glCreateShader(type);
    
	if (shId == 0) {
        OG_LOGERR("Shader", "could not create shader");
		return 0;
	}
    
    // set shader source
    glShaderSource(shId, 1, (const GLchar**)&src, NULL);
    
    // compile the shader
    glCompileShader(shId);
    
    // check compile status
    GLint compileStatus;
    glGetShaderiv(shId, GL_COMPILE_STATUS, &compileStatus);
    
	if (compileStatus != GL_TRUE) {
        OG_LOGERR("Shader", "could not compile shader program. error log:");
		GLchar infoLogBuf[1024];
		GLsizei infoLogLen;
		glGetShaderInfoLog(shId, 1024, &infoLogLen, infoLogBuf);
        cerr << infoLogBuf << endl << endl;
        OG_LOGERR("Shader", "could not compile shader program. shader source:");
        cerr << src << endl << endl;
        
		glDeleteShader(shId);
        
		return 0;
	}
    
	return shId;
}
