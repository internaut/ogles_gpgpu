//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * Abstract class that defines a multipass processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_MULTIPASSPROC
#define OGLES_GPGPU_COMMON_PROC_MULTIPASSPROC

#include "../../common_includes.h"
#include "multiprocinterface.h"

#include <list>

using namespace std;

namespace ogles_gpgpu {

class MultiPassProc : public MultiProcInterface {
public:

    virtual ProcInterface* getInputFilter() const;
    virtual ProcInterface* getOutputFilter() const;
    virtual ProcInterface * operator[](int i) const;
    virtual size_t size() const;
    
    /**
     * Deconstructor.
     * Removes all pass instances.
     */
    virtual ~MultiPassProc();

    /**
     * Init the processor for input frames of size <inW>x<inH> which is at
     * position <order> in the processing pipeline.
     * Abstract method.
     */
    virtual int init(int inW, int inH, unsigned int order, bool prepareForExternalInput = false);

    /**
     * Reinitialize the proc for a different input frame size of <inW>x<inH>.
     */
    virtual int reinit(int inW, int inH, bool prepareForExternalInput = false);

    /**
     * Cleanup processor's resources.
     */
    virtual void cleanup();

    /**
     * Create a texture that is attached to the FBO and will contain the processing result.
     * Set <genMipmap> to true to generate a mipmap (usually only works with POT textures).
     */
    virtual void createFBOTex(bool genMipmap);

    /**
     * Render a result, i.e. run the shader on the input texture.
     * Abstract method.
     */
    virtual int render(int position=0);

    /**
     * Use texture id <id> as input texture at texture <useTexUnit> with texture target <target>.
     */
    virtual void useTexture(GLuint id, GLuint useTexUnit = 1, GLenum target = GL_TEXTURE_2D, int position=0);

    /**
     * Returns true if output size < input size.
     */
    virtual bool getWillDownscale() const;

    /**
     * Get number of passes for this multipass processor.
     */
    size_t getNumPasses() const { return size(); }

    /**
     * Return te list of processor instances of each pass of this multipass processor.
     */
    std::vector<ProcInterface *> getProcPasses() const { return procPasses; }

protected:

    std::vector<ProcInterface *> procPasses;   // holds all instances to the single processing passes. strong ref!
};

}

#endif
