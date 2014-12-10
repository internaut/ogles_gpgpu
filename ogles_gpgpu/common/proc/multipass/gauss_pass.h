//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU gaussian smoothing processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_GAUSS_PASS
#define OGLES_GPGPU_COMMON_PROC_GAUSS_PASS

#include "../../common_includes.h"

#include "../base/filterprocbase.h"

namespace ogles_gpgpu {

/**
 * This filter applies gaussian smoothing to an input image.
 */
class GaussProcPass : public FilterProcBase {
public:
    /**
     * Construct as render pass <pass> (1 or 2).
     */
    GaussProcPass(int pass) : FilterProcBase(),
                              renderPass(pass),
                              pxDx(0.0f),
                              pxDy(0.0f)
    {
        assert(renderPass == 1 || renderPass == 2);
    }
    
    /**
     * Return the processors name.
     */
    virtual const char *getProcName() { return "GaussProcPass"; }
    
    /**
     * Init the processor for input frames of size <inW>x<inH> which is at
     * position <order> in the processing pipeline.
     */
    virtual int init(int inW, int inH, unsigned int order, bool prepareForExternalInput = false);
    
    /**
     * Render the output.
     */
    virtual void render();
    
    /**
     * Create a texture that is attached to the FBO and will contain the processing result.
     * Set <genMipmap> to true to generate a mipmap (usually only works with POT textures).
     * Overrides ProcBase's method.
     */
    virtual void createFBOTex(bool genMipmap);

private:
    int renderPass; // render pass number. must be 1 or 2
    
	GLint shParamUPxD;		// pixel delta values for texture lookup in the fragment shader. only used for adapt. thresholding
    
	float pxDx;	// pixel delta value for texture access
	float pxDy;	// pixel delta value for texture access
    
    static const char *fshaderGaussSrc;  // fragment shader source for gaussian smoothing for both passes
};

}
#endif
