//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "../common_includes.h"
#include "pyramid.h"

using namespace std;
using namespace ogles_gpgpu;

PyramidProc::PyramidProc()
{
    //ProcBase::setOutputSize(1.5);
}

void PyramidProc::setOutputSize(float scaleFactor)
{
    // noop
}

void PyramidProc::setScales(const std::vector<Size> &scales)
{
    m_scales = scales;
}

void PyramidProc::PyramidProc::render()
{
    OG_LOGINF(getProcName(), "input tex %d, target %d, framebuffer of size %dx%d", texId, texTarget, outFrameW, outFrameH);
    filterRenderPrepare();
    
    setUniforms();
    
    Tools::checkGLErr(getProcName(), "render prepare");
    
    filterRenderSetCoords();
    Tools::checkGLErr(getProcName(), "render set coords");
    
    // Set a constant background color
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0,0,0,1);
    
    if(m_scales.size())
    {
        renderMultiscale();
    }
    else
    {
        renderPyramid();
    }
    
 
    filterRenderCleanup();
    Tools::checkGLErr(getProcName(), "render cleanup");
}

void PyramidProc::renderPyramid()
{
    int x = 0, y = 0, w = inFrameW, h = inFrameH;
    for(int i = 0; i < 8; i++)
    {
        glViewport(x, y, w, h);
        if(i % 2)
        {
            y += h;
        }
        else
        {
            x += w;
        }
        
        w = (w >> 1);
        h = (h >> 1);
        
        filterRenderDraw();
        Tools::checkGLErr(getProcName(), "render draw");
    }

}

void PyramidProc::renderMultiscale()
{
    int x = 0, y = 0;
    for(int i = 0; i < m_scales.size(); i++)
    {
        glViewport(x, y, m_scales[i].width, m_scales[i].height);
        x += m_scales[i].width;
        filterRenderDraw();
        Tools::checkGLErr(getProcName(), "render draw");
    }
}

int PyramidProc::init(int inW, int inH, unsigned int order, bool prepareForExternalInput)
{
    int width = 0, height = 0;
    if(m_scales.size())
    {
        for(int i = 0; i < m_scales.size(); i++)
        {
            width += m_scales[i].width;
            height = std::max(m_scales[i].height, height);
        }
    }
    else
    {
        width = inW * 3/2;
        height = inH;
    }

    ProcBase::setOutputSize(width, height);
    return FilterProcBase::init(inW, inH, order, prepareForExternalInput);
}


