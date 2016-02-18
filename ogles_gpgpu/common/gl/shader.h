//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * Shader helper class.
 */
#ifndef OGLES_GPGPU_COMMON_GL_SHADER
#define OGLES_GPGPU_COMMON_GL_SHADER

#include "../common_includes.h"

#if OGLES_GPGPU_OPENGLES
#  define OGLES_GPGPU_LOWP lowp
#  define OGLES_GPGPU_MEDIUMP mediump
#  define OGLES_GPGPU_HIGHP highp
#else
#  define OGLES_GPGPU_LOWP
#  define OGLES_GPGPU_MEDIUMP
#  define OGLES_GPGPU_HIGHP
#endif

namespace ogles_gpgpu {

typedef enum {
    ATTR,
    UNIF
} ShaderParamType;

/**
 * Shader helper class for creating and managing an OpenGL shader.
 */
class Shader {
public:
    
    typedef std::pair<int, const char *> Attribute;
    typedef std::vector<Attribute> Attributes;
    
    /**
     * Constructor.
     */
    Shader();

    /**
     * Deconstructor.
     */
    ~Shader();

    /**
     * Build an OpenGL shader object from vertex and fragment shader source code
     * <vshSrc> and <fshSrc>.
     */
    bool buildFromSrc(const char *vshSrc, const char *fshSrc, const std::vector<Attribute> &attributes={});

    /**
     * Use the shader program.
     */
    void use();

    /**
     * Get a shader parameter position for a parameter of type <type> and with
     * <name>.
     */
    GLint getParam(ShaderParamType type, const char *name) const;

private:
    /**
     * Create a shader program from sources <vshSrc> and <fshSrc>. Save shader ids in
     * <vshId> and <fshId>.
     */
    static GLuint create(const char *vshSrc, const char *fshSrc, GLuint *vshId, GLuint *fshId, const Attributes &attributes={});

    /**
     * Compile a shader of type <type> and source <src> and return its id.
     */
    static GLuint compile(GLenum type, const char *src);


    GLuint programId;   // full shader program id
    GLuint vshId;       // vertex shader id
    GLuint fshId;       // fragment shader id

};

}

#endif
