#include "shader.h"

using namespace std;
using namespace ogles_gpgpu;

Shader::Shader() {
	programId = 0;
}

Shader::~Shader() {
	if (programId > 0) {
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
	GLint id = (type == ATTR) ?
    glGetAttribLocation(programId, name) :
    glGetUniformLocation(programId, name);
    
	if (id < 0) {
        cerr << "ogles_gpgpu::Shader - Could not get parameter id for param " << name << endl;
	}
    
	return id;
}

GLuint Shader::create(const char *vshSrc, const char *fshSrc, GLuint *vshId, GLuint *fshId) {
	*vshId = compile(GL_VERTEX_SHADER, vshSrc);
	*fshId = compile(GL_FRAGMENT_SHADER, fshSrc);
    
	GLuint programId = glCreateProgram();
    
	if (programId == 0) {
		cerr << "ogles_gpgpu::Shader - Could not create shader program" << endl;
		return 0;
	}
    
	glAttachShader(programId, *vshId);   // add the vertex shader to program
	glAttachShader(programId, *fshId);   // add the fragment shader to program
	glLinkProgram(programId);
    
	// check link status
	GLint linkStatus;
	glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE) {
		cerr << "ogles_gpgpu::Shader - Could not link shader program:" << endl;
		GLchar infoLogBuf[1024];
		GLsizei infoLogLen;
		glGetProgramInfoLog(programId, 1024, &infoLogLen, infoLogBuf);
        cerr << infoLogBuf << endl;
        
		glDeleteProgram(programId);
        
		return 0;
	}
    
	return programId;
}

GLuint Shader::compile(GLenum type, const char *src) {
	GLuint shId = glCreateShader(type);
    
	if (shId == 0) {
        cerr << "ogles_gpgpu::Shader -  Could not create shader." << endl;
		return 0;
	}
    
    glShaderSource(shId, 1, (const GLchar**)&src, NULL);
    glCompileShader(shId);
    
    // check compile status
    GLint compileStatus;
    glGetShaderiv(shId, GL_COMPILE_STATUS, &compileStatus);
    
	if (compileStatus != GL_TRUE) {
		cerr << "ogles_gpgpu::Shader - Could not compile shader:" << endl;
		GLchar infoLogBuf[1024];
		GLsizei infoLogLen;
		glGetShaderInfoLog(shId, 1024, &infoLogLen, infoLogBuf);
        cerr << infoLogBuf << endl << endl;
        cerr << "shader source:" << endl << src << endl;
        
		glDeleteShader(shId);
        
		return 0;
	}
    
	return shId;
}
