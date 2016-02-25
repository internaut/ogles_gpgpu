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

static std::vector<Rect2d> pack(const std::vector<Size2d> &src)
{
    std::vector<Rect2d> packed;
    
    int x = 0, y = 0;
    bool half = false;
    
    // Decrease going forward:
    int i = 0;
    for(; (i < src.size()) && !half; i++)
    {
        packed.emplace_back(x, y, src[i].width, src[i].height);
        x += src[i].width;
        if(src[i].height*2 < src[0].height)
        {
            half = true;
        }
    }
    
    // Decrease going backward -- now x becomes the right edge
    int t, l, r, b = src[0].height;
    for(; i < src.size(); i++)
    {
        r = x;
        l = r - src[i].width;
        t = b - src[i].height;
        packed.emplace_back(l, t, src[i].width, src[i].height);
        x -= src[i].width;
    }
    
    return packed;
}

static std::vector<Rect2d> reduce(const Size2d &src, int levels)
{
    std::vector<Rect2d> packed;
    
    int x = 0, y = 0, w = src.width, h = src.height;
    for(int i = 0; i < levels; i++)
    {
        packed.emplace_back(x, y, w, h);
        
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
    }
    
    return packed;
}


PyramidProc::PyramidProc(int levels) : m_levels(levels)
{

}

PyramidProc::PyramidProc(const std::vector<Size2d> &scales) : m_scales(scales)
{
    
}

void PyramidProc::setOutputSize(float scaleFactor)
{
    // noop
}

void PyramidProc::setScales(const std::vector<Size2d> &scales)
{
    m_crops = pack(scales);
}

void PyramidProc::setLevels(int levels)
{
    m_scales.clear();
    m_levels = levels;
}

const std::vector<Rect2d> & PyramidProc::getLevelCrops() const
{
    return m_crops;
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
    
    for(auto &c : m_crops)
    {
        glViewport(c.x, c.y, c.width, c.height);
        filterRenderDraw();
    }
    Tools::checkGLErr(getProcName(), "render draw");
    
    filterRenderCleanup();
    Tools::checkGLErr(getProcName(), "render cleanup");
}


int PyramidProc::init(int inW, int inH, unsigned int order, bool prepareForExternalInput)
{
    int width = 0, height = 0;
    if(m_scales.size())
    {
        m_crops = pack(m_scales);
        for(const auto &c : m_crops)
        {
            width = std::max(width, c.x + c.width);
            height = std::max(height, c.y + c.width);
        }
    }
    else
    {
        m_crops = reduce({inW, inH}, m_levels);
        width = inW * 3 / 2;
        height = inH;
        
    }

    FilterProcBase::setOutputSize(width, height);
    return FilterProcBase::init(inW, inH, order, prepareForExternalInput);
}


