#ifndef OGLES_GPGPU_COMMON_SHADER
#define OGLES_GPGPU_COMMON_SHADER

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
    
	bool buildFromSrc(char *vshSrc, char *fshSrc);
	void use();
    
	GLint getParam(ShaderParamType type, const char *name);
    
private:
	static GLuint create(char *vshSrc, char *fshSrc, GLuint *vshId, GLuint *fshId);
	static GLuint compile(GLenum type, char *src);
    
	GLuint programId;
	GLuint vshId;
	GLuint fshId;

};
    
}

#endif