//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
//         David Hirvonen <dhirvonen@elucideye.com>
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * GPGPU transform processor.
 */
#ifndef OGLES_GPGPU_COMMON_PROC_PYRAMID
#define OGLES_GPGPU_COMMON_PROC_PYRAMID

#include "../common_includes.h"

#include "base/filterprocbase.h"

#include "transform.h"

namespace ogles_gpgpu {

/**
 * GPGPU transform processor to create a flat pyramid
 */
class PyramidProc : public TransformProc {
public:

    /**
     * Constructor for pyramid.
     */
    PyramidProc(int levels = 4);
    
    /**
     * Constructor for multiscale.
     */
    PyramidProc(const std::vector<Size2d> &scales);

    /**
     * Render a flat pyramid
     */
    virtual int render(int position=0);
    
    /**
     * Return the processors name.
     */
    virtual const char *getProcName() { return "PyramidProc"; }

    /**
     *
     */
    virtual int init(int inW, int inH, unsigned int order, bool prepareForExternalInput);

    /**
     * Preset output scales
     */
    void setScales(const std::vector<Size2d> &scales);
    
    /**
     * Set the # of pyramid levels.
     */
    void setLevels(int levels);
    
    /**
     * Get level crops.
     */
    const std::vector<Rect2d> & getLevelCrops() const;
    
private:
    
    virtual void setOutputSize(float scaleFactor);

    std::vector<Size2d> m_scales;
    
    int m_levels = 4;
    
    std::vector<Rect2d> m_crops;
    
};
}

#endif
