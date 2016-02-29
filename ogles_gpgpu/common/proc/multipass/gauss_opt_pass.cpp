//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "../../common_includes.h"
#include "gauss_opt_pass.h"


#include <cmath>

using namespace ogles_gpgpu;


void getOptimizedGaussian(int blurRadius, float sigma, std::vector<GLfloat> &weights, std::vector<GLfloat> &offsets)
{
    std::vector<GLfloat> standardGaussianWeights(blurRadius + 1);
    GLfloat sumOfWeights = 0.0;
    
    GLfloat sigma2 = sigma*sigma;
    GLfloat norm = (1.0 / std::sqrt(2.0 * M_PI * sigma2));
    for (int currentGaussianWeightIndex = 0; currentGaussianWeightIndex < (blurRadius + 1); currentGaussianWeightIndex++)
    {
        standardGaussianWeights[currentGaussianWeightIndex] = norm * std::exp(-std::pow(currentGaussianWeightIndex, 2.0) / (2.0 * sigma2));
        if (currentGaussianWeightIndex == 0)
        {
            sumOfWeights += standardGaussianWeights[currentGaussianWeightIndex];
        }
        else
        {
            sumOfWeights += 2.0 * standardGaussianWeights[currentGaussianWeightIndex];
        }
    }
    
    // Next, normalize these weights to prevent the clipping of the Gaussian curve at the end of the discrete samples from reducing luminance
    for (int currentGaussianWeightIndex = 0; currentGaussianWeightIndex < (blurRadius + 1); currentGaussianWeightIndex++)
    {
        standardGaussianWeights[currentGaussianWeightIndex] = standardGaussianWeights[currentGaussianWeightIndex] / sumOfWeights;
    }
    
    // From these weights we calculate the offsets to read interpolated values from
    int numberOfOptimizedOffsets = std::min(blurRadius / 2 + (blurRadius % 2), 7);
    
    std::vector<GLfloat> optimizedGaussianOffsets(numberOfOptimizedOffsets);
    for (int currentOptimizedOffset = 0; currentOptimizedOffset < numberOfOptimizedOffsets; currentOptimizedOffset++)
    {
        GLfloat firstWeight = standardGaussianWeights[currentOptimizedOffset*2 + 1];
        GLfloat secondWeight = standardGaussianWeights[currentOptimizedOffset*2 + 2];
        GLfloat optimizedWeight = firstWeight + secondWeight;
        optimizedGaussianOffsets[currentOptimizedOffset] = (firstWeight * (currentOptimizedOffset*2 + 1) + secondWeight * (currentOptimizedOffset*2 + 2)) / optimizedWeight;
    }
    
    weights = standardGaussianWeights;
    offsets = optimizedGaussianOffsets;
}

std::string fragmentShaderForOptimizedBlur(int blurRadius, float sigma, bool doNorm = false, int pass = 1, float normConst=0.005f)
{
    std::vector<GLfloat> standardGaussianWeights;
    std::vector<GLfloat> optimizedGaussianOffsets;
    getOptimizedGaussian(blurRadius, sigma, standardGaussianWeights, optimizedGaussianOffsets);
    
    // From these weights we calculate the offsets to read interpolated values from
    int numberOfOptimizedOffsets =  std::min(blurRadius / 2 + (blurRadius % 2), 7);
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
    ss << "   vec4 center = texture2D(inputImageTexture, blurCoordinates[0]);\n";
    ss << "   sum += center * " << standardGaussianWeights[0] << ";\n";
    
    for (int currentBlurCoordinateIndex = 0; currentBlurCoordinateIndex < numberOfOptimizedOffsets; currentBlurCoordinateIndex++)
    {
        GLfloat firstWeight = standardGaussianWeights[currentBlurCoordinateIndex * 2 + 1];
        GLfloat secondWeight = standardGaussianWeights[currentBlurCoordinateIndex * 2 + 2];
        GLfloat optimizedWeight = firstWeight + secondWeight;
        int index1 = (unsigned long)((currentBlurCoordinateIndex * 2) + 1);
        int index2 = (unsigned long)((currentBlurCoordinateIndex * 2) + 2);
        ss << "   sum += texture2D(inputImageTexture, blurCoordinates[" << index1 << "]) * " << optimizedWeight <<";\n";
        ss << "   sum += texture2D(inputImageTexture, blurCoordinates[" << index2 << "]) * " << optimizedWeight <<";\n";
    }
    
    // If the number of required samples exceeds the amount we can pass in via varyings, we have to do dependent texture reads in the fragment shader
    if (trueNumberOfOptimizedOffsets > numberOfOptimizedOffsets)
    {
        ss << "   vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);\n";
        for (int currentOverlowTextureRead = numberOfOptimizedOffsets; currentOverlowTextureRead < trueNumberOfOptimizedOffsets; currentOverlowTextureRead++)
        {
            GLfloat firstWeight = standardGaussianWeights[currentOverlowTextureRead * 2 + 1];
            GLfloat secondWeight = standardGaussianWeights[currentOverlowTextureRead * 2 + 2];
            
            GLfloat optimizedWeight = firstWeight + secondWeight;
            GLfloat optimizedOffset = (firstWeight * (currentOverlowTextureRead * 2 + 1) + secondWeight * (currentOverlowTextureRead * 2 + 2)) / optimizedWeight;

            ss << "   sum += texture2D(inputImageTexture, blurCoordinates[0] + singleStepOffset * " << optimizedOffset << ") * " << optimizedWeight << ";\n";
            ss << "   sum += texture2D(inputImageTexture, blurCoordinates[0] - singleStepOffset * " << optimizedOffset << ") * " << optimizedWeight << ";\n";
        }
    }

    if(doNorm)
    {
        if(pass == 1)
        {
            ss << "   gl_FragColor = vec4(center.rgb, sum.r);\n";
        }
        else
        {
            ss << "   gl_FragColor = vec4( center.r/(sum.a + " << std::fixed << normConst << "), center.gb, 1.0);\n";
        }
    }
    else
    {
        ss << "   gl_FragColor = sum;\n";
    }
    
    ss << "}\n";
    
    return ss.str();
}

std::string vertexShaderForOptimizedBlur(int blurRadius, float sigma)
{
    std::vector<GLfloat> standardGaussianWeights;
    std::vector<GLfloat> optimizedGaussianOffsets;
    getOptimizedGaussian(blurRadius, sigma, standardGaussianWeights, optimizedGaussianOffsets);
    
    int numberOfOptimizedOffsets = optimizedGaussianOffsets.size();
                   
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
        const auto &optOffset = optimizedGaussianOffsets[currentOptimizedOffset];
        
        ss << "   blurCoordinates[" << x1 << "] = inputTextureCoordinate.xy + singleStepOffset * " << optOffset << ";\n";
        ss << "   blurCoordinates[" << x2 << "] = inputTextureCoordinate.xy + singleStepOffset * " << optOffset << ";\n";
    }
    ss << "}\n";
    
    return ss.str();
}


void GaussOptProcPass::setRadius(float newValue)
{
    if (round(newValue) != _blurRadiusInPixels)
    {
        _blurRadiusInPixels = round(newValue); // For now, only do integral sigmas
        
        int calculatedSampleRadius = 0;
        if (_blurRadiusInPixels >= 1) // Avoid a divide-by-zero error here
        {
            // Calculate the number of pixels to sample from by setting a bottom limit for the contribution of the outermost pixel
            float minimumWeightToFindEdgeOfSamplingArea = 1.0/256.0;
            float radius2 = std::pow(_blurRadiusInPixels, 2.0) ;
            calculatedSampleRadius = std::floor(std::sqrt(-2.0 * radius2 * std::log(minimumWeightToFindEdgeOfSamplingArea * std::sqrt(2.0 * M_PI * radius2))));
            calculatedSampleRadius += calculatedSampleRadius % 2; // There's nothing to gain from handling odd radius sizes, due to the optimizations I use
        }
        
        //std::cout << "Blur radius " << _blurRadiusInPixels << " calculated sample radius " << calculatedSampleRadius << std::endl;
        //std::cout << "===" << std::endl;

        vshaderGaussSrc = vertexShaderForOptimizedBlur(calculatedSampleRadius, _blurRadiusInPixels);
        fshaderGaussSrc = fragmentShaderForOptimizedBlur(calculatedSampleRadius, _blurRadiusInPixels, doNorm, renderPass, normConst);
        
        //std::cout << vshaderGaussSrc << std::endl;
        //std::cout << fshaderGaussSrc << std::endl;
    }
}

// TODO: We need to override this if we are using the GPUImage shaders
void GaussOptProcPass::filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target)
{
    // create shader object
    ProcBase::createShader(vShaderSrc, fShaderSrc, target);
    
    // get shader params
    shParamAPos = shader->getParam(ATTR, "position");
    shParamATexCoord = shader->getParam(ATTR, "inputTextureCoordinate");
    Tools::checkGLErr(getProcName(), "filterShaderSetup");
}

void GaussOptProcPass::setUniforms()
{
    FilterProcBase::setUniforms();

    glUniform1f(shParamUTexelWidthOffset, (renderPass == 1) * pxDx);
    glUniform1f(shParamUTexelHeightOffset, (renderPass == 2) * pxDy);
}

void GaussOptProcPass::getUniforms()
{
    FilterProcBase::getUniforms();
    
    // calculate pixel delta values
    pxDx = 1.0f / (float)outFrameW; // input or output?
    pxDy = 1.0f / (float)outFrameH;

    shParamUInputTex = shader->getParam(UNIF, "inputImageTexture");
    shParamUTexelWidthOffset = shader->getParam(UNIF, "texelWidthOffset");
    shParamUTexelHeightOffset = shader->getParam(UNIF, "texelHeightOffset");
}

const char *GaussOptProcPass::getFragmentShaderSource()
{
    return fshaderGaussSrc.c_str();
}

const char *GaussOptProcPass::getVertexShaderSource()
{
    return vshaderGaussSrc.c_str();
}



