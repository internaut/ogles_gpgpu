//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU simple thresholding processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_THRESH
#define OGLES_GPGPU_COMMON_PROC_THRESH

#include "../common_includes.h"

#include "base/filterprocbase.h"

namespace ogles_gpgpu {
    
/**
 * GPGPU thresholding processor that uses a simple threshold to binarize
 * a grayscale input image.
 */
class ThreshProc : public FilterProcBase {
public:
    /**
     * Constructor.
     */
    ThreshProc();
    
    /**
     * Return the processors name.
     */
    virtual const char *getProcName() { return "ThreshProc"; }
    
    /**
     * Set threshold as 8 bit value [0..255] <v> for simple thresholding.
     */
    void setThreshVal8Bit(int v) { threshVal = (float)v  / 255.0f; }
    
    /**
     * Set threshold as float value [0..1] <v> for simple thresholding.
     */
    void setThreshVal(float v) { threshVal = v; }
    
    /**
     * Get threshold as float value [0..1] <v> for simple thresholding.
     */
    float getThreshVal() const { return threshVal; }
    
    /**
     * Init the processor for input frames of size <inW>x<inH> which is at
     * position <order> in the processing pipeline.
     */
    virtual int init(int inW, int inH, unsigned int order, bool prepareForExternalInput = false);
    
    /**
     * Render the output.
     */
    virtual void render();
    
private:
    float threshVal;            // thresholding value [0.0 .. 1.0]
    
	GLint shParamUThresh;	// fixed threshold value
    
    static const char *fshaderSimpleThreshSrc;      // fragment shader source for simple thresholding
};
}

#endif