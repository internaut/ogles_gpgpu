//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * Helper class to simply display an output.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_DISP
#define OGLES_GPGPU_COMMON_PROC_DISP

#include "../common_includes.h"

#include "base/filterprocbase.h"

namespace ogles_gpgpu {
    
/**
 * Helper class to simply display an output.
 * Render an input texture to a fullscreen quad.
 */
class Disp : public FilterProcBase {
public:    
    /**
     * Return the processors name.
     */
    virtual const char *getProcName() { return "Disp"; }
    
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
     * Not implemented - no output texture needed because Disp renders on screen.
     */
    virtual void createFBOTex(bool genMipmap) { assert(false); }

    /**
     * Not implemented - no output is returned because Disp renders on screen.
     */
    virtual void getResultData(unsigned char *data) const { assert(false); }
    
    /**
     * Not implemented - no MemTransferObj for output is set because Disp renders on screen.
     */
    virtual MemTransfer *getMemTransferObj() const { assert(false); return NULL; }

private:
    static const char *fshaderDispSrc;         // fragment shader source
};
    
}

#endif