//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "../../common_includes.h"
#include "box_opt_pass.h"


#include <cmath>

using namespace ogles_gpgpu;

static int getNumberOfOptimizedOffsets(int blurRadius)
{
    return std::min(blurRadius / 2 + (blurRadius % 2), 7);
}

static std::string vertexShaderForOptimizedBoxBlur(int blurRadius, float sigma)
{
    int numberOfOptimizedOffsets = getNumberOfOptimizedOffsets(blurRadius);
                   
    std::stringstream ss;
    ss << "attribute vec4 position;\n";
    ss << "attribute vec4 inputTextureCoordinate;\n";
    ss << "uniform float texelWidthOffset;\n";
    ss << "uniform float texelHeightOffset;\n\n";
    ss << "varying vec2 blurCoordinates[" << (unsigned long)(1 + (numberOfOptimizedOffsets * 2)) <<  "];\n\n";
    ss << "void main()\n";
    ss << "{\n";
    ss << "   gl_Position = position;\n";
    ss << "   vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);\n";
    ss << "   blurCoordinates[0] = inputTextureCoordinate.xy;\n";
    for (int currentOptimizedOffset = 0; currentOptimizedOffset < numberOfOptimizedOffsets; currentOptimizedOffset++)
    {
        int x1 = (unsigned long)((currentOptimizedOffset * 2) + 1);
        int x2 = (unsigned long)((currentOptimizedOffset * 2) + 2);
        
        GLfloat optimizedOffset = (GLfloat)(currentOptimizedOffset * 2) + 1.5;
        
        ss << "   blurCoordinates[" << x1 << "] = inputTextureCoordinate.xy + singleStepOffset * " << optimizedOffset << ";\n";
        ss << "   blurCoordinates[" << x2 << "] = inputTextureCoordinate.xy - singleStepOffset * " << optimizedOffset << ";\n";
    }
    ss << "}\n";
    
    return ss.str();
}

static std::string fragmentShaderForOptimizedBoxBlur(int blurRadius, float sigma, int pass = 1)
{
    assert(blurRadius > 1);
    
    // From these weights we calculate the offsets to read interpolated values from
    int numberOfOptimizedOffsets =  getNumberOfOptimizedOffsets(blurRadius);
    int trueNumberOfOptimizedOffsets = blurRadius / 2 + (blurRadius % 2);
    
    std::stringstream ss;
#if defined(OGLES_GPGPU_OPENGLES)
    ss << "precision highp float;\n";
    ss << "\n";
#endif
    ss << "uniform sampler2D inputImageTexture;\n";
    ss << "uniform float texelWidthOffset;\n";
    ss << "uniform float texelHeightOffset;\n\n";
    ss << "varying vec2 blurCoordinates[" << (1 + (numberOfOptimizedOffsets * 2)) << "];\n\n";
    ss << "void main()\n";
    ss << "{\n";
    ss << "   vec4 sum = vec4(0.0);\n";

    GLfloat boxWeight = 1.0 / GLfloat((blurRadius * 2) + 1), boxWeight2 = 2.0 * boxWeight;
    
    ss << " sum += texture2D(inputImageTexture, blurCoordinates[0]) * " << boxWeight << ";\n";

    for (int currentBlurCoordinateIndex = 0; currentBlurCoordinateIndex < numberOfOptimizedOffsets; currentBlurCoordinateIndex++)
    {
        int index1 = (unsigned long)((currentBlurCoordinateIndex * 2) + 1);
        int index2 = (unsigned long)((currentBlurCoordinateIndex * 2) + 2);
        ss << "   sum += texture2D(inputImageTexture, blurCoordinates[" << index1 << "]) * " << boxWeight2 <<";\n";
        ss << "   sum += texture2D(inputImageTexture, blurCoordinates[" << index2 << "]) * " << boxWeight2  <<";\n";
    }
    
    // If the number of required samples exceeds the amount we can pass in via varyings, we have to do dependent texture reads in the fragment shader
    if (trueNumberOfOptimizedOffsets > numberOfOptimizedOffsets)
    {
        ss << "   vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);\n";
        for (int currentOverlowTextureRead = numberOfOptimizedOffsets; currentOverlowTextureRead < trueNumberOfOptimizedOffsets; currentOverlowTextureRead++)
        {
            GLfloat optimizedOffset = (GLfloat)(currentOverlowTextureRead * 2) + 1.5;

            ss << "   sum += texture2D(inputImageTexture, blurCoordinates[0] + singleStepOffset * " << optimizedOffset << ") * " << boxWeight2 << ";\n";
            ss << "   sum += texture2D(inputImageTexture, blurCoordinates[0] - singleStepOffset * " << optimizedOffset << ") * " << boxWeight2 << ";\n";
        }
    }

    ss << "   gl_FragColor = sum;\n";
    ss << "}\n";
    
    return ss.str();
}


void BoxOptProcPass::setRadius(float newValue)
{
    float newBlurRadius = round(round(newValue / 2.0) * 2.0); 
    
    if (newBlurRadius != _blurRadiusInPixels)
    {
        _blurRadiusInPixels = newBlurRadius;
        
        //std::cout << "Blur radius " << _blurRadiusInPixels << " calculated sample radius " << calculatedSampleRadius << std::endl;
        //std::cout << "===" << std::endl;

        vshaderBoxSrc = vertexShaderForOptimizedBoxBlur(_blurRadiusInPixels, 0.0);
        fshaderBoxSrc = fragmentShaderForOptimizedBoxBlur(_blurRadiusInPixels, 0.0);
        
        //std::cout << vshaderBoxSrc << std::endl;
        //std::cout << fshaderBoxSrc << std::endl;
    }
}

// TODO: We need to override this if we are using the GPUImage shaders
void BoxOptProcPass::filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target)
{
    // create shader object
    ProcBase::createShader(vShaderSrc, fShaderSrc, target);
    
    // get shader params
    shParamAPos = shader->getParam(ATTR, "position");
    shParamATexCoord = shader->getParam(ATTR, "inputTextureCoordinate");
    Tools::checkGLErr(getProcName(), "filterShaderSetup");
}

void BoxOptProcPass::setUniforms()
{
    FilterProcBase::setUniforms();

    glUniform1f(shParamUTexelWidthOffset, (renderPass == 1) * pxDx);
    glUniform1f(shParamUTexelHeightOffset, (renderPass == 2) * pxDy);
}

void BoxOptProcPass::getUniforms()
{
    FilterProcBase::getUniforms();
    
    // calculate pixel delta values
    pxDx = 1.0f / (float)outFrameW; 
    pxDy = 1.0f / (float)outFrameH;

    shParamUInputTex = shader->getParam(UNIF, "inputImageTexture");
    shParamUTexelWidthOffset = shader->getParam(UNIF, "texelWidthOffset");
    shParamUTexelHeightOffset = shader->getParam(UNIF, "texelHeightOffset");
}

const char *BoxOptProcPass::getFragmentShaderSource()
{
    return fshaderBoxSrc.c_str();
}

const char *BoxOptProcPass::getVertexShaderSource()
{
    return vshaderBoxSrc.c_str();
}



