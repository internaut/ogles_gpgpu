#ifndef OGLES_GPGPU_COMMON_GL_SHADER
#define OGLES_GPGPU_COMMON_GL_SHADER

#include "../common_includes.h"

namespace ogles_gpgpu {

typedef enum {
    ATTR,
    UNIF
} ShaderParamType;
    
class Shader {
public:
	Shader();
	~Shader();
    
	bool buildFromSrc(const char *vshSrc, const char *fshSrc);
	void use();
    
	GLint getParam(ShaderParamType type, const char *name) const;
    
private:
	static GLuint create(const char *vshSrc, const char *fshSrc, GLuint *vshId, GLuint *fshId);
	static GLuint compile(GLenum type, const char *src);
    
	GLuint programId;
	GLuint vshId;
	GLuint fshId;

};
    
}

#endif