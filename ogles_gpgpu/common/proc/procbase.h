#ifndef OGLES_GPGPU_COMMON_PROC_BASE
#define OGLES_GPGPU_COMMON_PROC_BASE

#include "../common_includes.h"

#include "../gl/fbo.h"
#include "../gl/shader.h"

#define OGLES_GPGPU_QUAD_VERTICES 				4
#define OGLES_GPGPU_QUAD_COORDS_PER_VERTEX      3
#define OGLES_GPGPU_QUAD_TEXCOORDS_PER_VERTEX 	2
#define OGLES_GPGPU_QUAD_VERTEX_BUFSIZE          (OGLES_GPGPU_QUAD_VERTICES * OGLES_GPGPU_QUAD_COORDS_PER_VERTEX)
#define OGLES_GPGPU_QUAD_TEX_BUFSIZE 			(OGLES_GPGPU_QUAD_VERTICES * OGLES_GPGPU_QUAD_TEXCOORDS_PER_VERTEX)

namespace ogles_gpgpu {

class ProcBase {
public:
    ProcBase();
    virtual ~ProcBase();
    
    /**
     * Creates the shader and fbo.
     * Abstract method.
     */
    virtual void init(int inW, int inH) = 0;
    
    /**
     * Render a result, i.e. run the shader on the input texture.
     * Abstract method.
     */
    virtual void render() = 0;
    
    /**
     * Use texture id <id> as input texture.
     */
    void useTexture(GLuint id) { texId = id; }
    
    int getOutFrameW() const { return outFrameW; }
    int getOutFrameH() const { return outFrameH; }
    
    /**
     * Return the result data from the FBO.
     */
    virtual void getResultData(unsigned char *data) const;
    
    /**
     * Return the output texture id (= texture that is attached to the FBO).
     */
    GLuint getOutputTexId() const;
    
protected:
    virtual void baseInit(int inW, int inH, int outW = 0, int outH = 0, float scaleFactor = 1.0f);
    
    virtual void createFBO();
    virtual void createShader(const char *vShSrc, const char *fShSrc);
    
    
	static const GLfloat quadTexCoordsStd[];
	static const GLfloat quadTexCoordsFlipped[];
	static const GLfloat quadVertices[];
    
    FBO *fbo;       // strong ref.!
	Shader *shader;	// strong ref.!
    
	GLuint texId;

	int inFrameW;
	int inFrameH;
    
	int outFrameW;
	int outFrameH;
};

}

#endif