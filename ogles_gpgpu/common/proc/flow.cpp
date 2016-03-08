#include "../common_includes.h"
#include "flow.h"

BEGIN_OGLES_GPGPU

FlowProc::FlowProc(float tau, float strength) : tau(tau), strength(strength) {}

void FlowProc::filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target)
{
    // create shader object and get attributes:
    ProcBase::createShader(vShaderSrc, fShaderSrc, target);
    shParamAPos = shader->getParam(ATTR, "position");
    shParamATexCoord = shader->getParam(ATTR, "inputTextureCoordinate");
}

void FlowProc::getUniforms()
{
    FilterProcBase::getUniforms();
    texelWidthUniform = shader->getParam(UNIF, "texelWidth");
    texelHeightUniform = shader->getParam(UNIF, "texelHeight");
    shParamUInputTex = shader->getParam(UNIF, "inputImageTexture");
    shParamUStrength = shader->getParam(UNIF, "strength");
    shParamUTau = shader->getParam(UNIF, "tau");
}

void FlowProc::setUniforms()
{
    FilterProcBase::setUniforms();
    glUniform1f(texelWidthUniform, (1.0f/ float(outFrameW)));
    glUniform1f(texelHeightUniform, (1.0f/ float(outFrameH)));
    glUniform1f(shParamUStrength, strength);
    glUniform1f(shParamUTau, tau);
}


// Solve 2x2 matrix inverse via Cramer's rule:

// [ IxX Ixy; Iyx Iyy ] [u v]' = [ Ix*It Iy*It ]

// "Let it be..."
// [ A C; C B ] * [ u v ]' = [ X Y ]

// |A C|
// |Y B|
// D = det([A C; C B])

// |X C|
// |Y B|
// Dx = det([X C; Y B])

// |A X|
// |C Y|
// Dy = det([A X; C Y])

// x = Dx/D
// y = Dy/D

// See, for example:
// http://www.mathworks.com/help/vision/ref/opticalflowlk-class.html?refresh=true


// Cornerness:
// T1 = (A+B)/2.0
// T2 = sqrt(4*C^2 + (A-B)^2)/2.0
// lambda_1 = T1 + T2
// lambda+2 = T1 - T2

// case 1: lambda_1 >= tau && lambda_2 >= tau  | non singular
// case 2: lambda_1 >= tau && lambda_2 < tau   | singular (can be normalized to calculate u and v)
// case 3: lambda_1 < tau && lambda_2 < tau    | flow is 0

// TODO: slow, avoid dependent texture lookups!!!!

const char *FlowProc::fshaderFlowSrc = OG_TO_STR
(
#if defined(OGLES_GPGPU_OPENGLES)
 precision highp float;
#endif
 
 varying vec2 textureCoordinate;
 uniform sampler2D inputImageTexture;
 
 uniform float texelWidth;
 uniform float texelHeight;
 uniform float strength;
 uniform float tau; // noise threshold (0.004)
 
 const int wSize = 5;
 
 void main()
 {
     float X = 0.0;
     float Y = 0.0;
     float A = 0.0;
     float B = 0.0;
     float C = 0.0;
     
     // Build the 2x2 matrix and the Intensity vector:
     for(int y=-wSize; y<=wSize; y++)
     {
         for(int x=-wSize; x<=wSize; x++)
         {
             vec2 delta = vec2( float(x)*texelWidth, float(y)*texelHeight );
             vec2 pos = textureCoordinate + delta;
             vec4 pix = texture2D(inputImageTexture, pos) * 2.0 - 1.0;
             float w = cos(dot(delta,delta)*40.0);
             A 	= A + w * pix.x * pix.x;
             B 	= B + w * pix.y * pix.y;
             C 	= C + w * pix.x * pix.y;
             X 	= X + w * pix.x * pix.z;
             Y 	= Y + w * pix.y * pix.z;
         }
     }
     
     // T1 = (A+B)/2.0
     // T2 = sqrt(4*C^2 + (A-B)^2)/2.0
     // lambda_1 = T1 + T2
     // lambda_2 = T1 - T2
     float TMP = (A-B);
     float T1 = (A+B)/2.0;
     float T2 = sqrt(4.0 * C*C + TMP*TMP)/2.0;
     float L1 = T1 + T2;
     float L2 = T1 - T2;
     
     float D = 1.0/(A*B-C*C);
     
     if(L1 <= tau || L2 <= tau)
     {
         D = 0.0;
     }
     
     vec4 center = texture2D(inputImageTexture, textureCoordinate);
     vec2 uv = vec2(X*B - C*Y, A*Y - X*C) * D;
     vec4 flow = vec4(strength * ((-uv + 1.0) / 2.0), (center.xy + 1.0) / 2.0);
     gl_FragColor = flow;
 });

const char *FlowProc::fshaderFlowSrcOpt = OG_TO_STR
(
#if defined(OGLES_GPGPU_OPENGLES)
 precision highp float;
#endif
 
 varying vec2 textureCoordinate;
 uniform sampler2D inputImageTexture;
 
 uniform float texelWidth;
 uniform float texelHeight;
 uniform float strength;
 uniform float tau; // noise threshold (0.004)
 
 void main()
 {
     vec4 pix = texture2D(inputImageTexture, textureCoordinate) * 2.0 - 1.0;
     float A = pix.x * pix.x;
     float B = pix.y * pix.y;
     float C = pix.x * pix.y;
     float X = pix.x * pix.z;
     float Y = pix.y * pix.z;
     
     float D = 1.0/(A*B-C*C);
     
     if(D > 100.0)
     {
         D = 0.0;
     }
     
     vec2 uv = vec2( X*B - C*Y, A*Y - X*C ) * D;
     vec4 flow = vec4( strength * ((-uv + 1.0) / 2.0), (pix.xy + 1.0) / 2.0 );
     gl_FragColor = flow;
 });

END_OGLES_GPGPU

#include "grayscale.h"
#include "gauss_opt.h"
#include "fifo.h"
#include "ixyt.h"
#include "transform.h"

// #################

BEGIN_OGLES_GPGPU

struct FlowPipeline::Impl
{
    Impl(float tau, float strength, bool doGray)
    : doGray(doGray)
    , gaussProc(1.0f)
    , flowProc(tau, strength)
    {
        if(!doGray)
        {
            grayProc.setGrayscaleConvType(GRAYSCALE_INPUT_CONVERSION_NONE);
        }

        { // flow processing
            grayProc.add(&diffProc, 0);
            grayProc.add(&fifoProc);
            fifoProc.add(&diffProc, 1);
            diffProc.add(&gaussProc);
            gaussProc.add(&flowProc);
        }
     }

    bool doGray = true;
    
    GrayscaleProc grayProc;
    FifoProc fifoProc;
    IxytProc diffProc;
    GaussOptProc gaussProc;
    FlowProc flowProc;
};

FlowPipeline::FlowPipeline(float tau, float strength, bool doGray)
{
    m_pImpl = std::unique_ptr<Impl>(new Impl(tau, strength, doGray));
};

FlowPipeline::~FlowPipeline()
{
    // Don't delete weak refs:
}

ProcInterface * FlowPipeline::first()
{
    return &m_pImpl->grayProc;
}

ProcInterface * FlowPipeline::last()
{
    return &m_pImpl->flowProc; // fifoProc;
}

float FlowPipeline::getStrength() const
{
    return m_pImpl->flowProc.getStrength();
}


END_OGLES_GPGPU
