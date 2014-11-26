/**
 * GPGPU thresholding processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_THRESH
#define OGLES_GPGPU_COMMON_PROC_THRESH

#include "../common_includes.h"

#include "filterprocbase.h"

namespace ogles_gpgpu {

/**
 * Define thresholding types
 */
typedef enum {
    THRESH_SIMPLE = 0,      // simple (single-pass)
    THRESH_ADAPTIVE_PASS_1, // adaptive (rendering pass 1)
    THRESH_ADAPTIVE_PASS_2  // adaptive (rendering pass 2)
} ThreshProcType;
    
/**
 * GPGPU thresholding processor uses a simple or adaptive threshold to binarize
 * a grayscale input image.
 */
class ThreshProc : public FilterProcBase {
public:
    /**
     * Constructor.
     */
    ThreshProc();
    
    /**
     * Set threshold type to one of ThreshProcType in <t>.
     */
    void setThreshType(ThreshProcType t) { threshType = t; }
    
    /**
     * Get threshold type.
     */
    ThreshProcType getThreshType() const { return threshType; }
    
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
    virtual void init(int inW, int inH, unsigned int order, bool prepareForExternalInput = false);
    
    /**
     * Create a texture that is attached to the FBO and will contain the processing result.
     * Set <genMipmap> to true to generate a mipmap (usually only works with POT textures).
     * Overrides ProcBase's method.
     */
    virtual void createFBOTex(bool genMipmap);
    
    /**
     * Render the output.
     */
    virtual void render();
    
private:
    ThreshProcType threshType;  // thresholding type
    float threshVal;            // only used for simple thresholding [0.0 .. 1.0]
    
	GLint shParamUPxD;		// pixel delta values for texture lookup in the fragment shader. only used for adapt. thresholding
	GLint shParamUThresh;	// fixed threshold value. only used for simple thresholding
    
	float pxDx;	// pixel delta value for texture access. only used for adapt. thresholding
	float pxDy;	// pixel delta value for texture access. only used for adapt. thresholding
    
    static const char *fshaderSimpleThreshSrc;      // fragment shader source for simple thresholding
    static const char *fshaderAdaptThreshPass1Src;  // fragment shader source for adaptive thresholding pass 1
    static const char *fshaderAdaptThreshPass2Src;  // fragment shader source for adaptive thresholding pass 2
};
}

#endif