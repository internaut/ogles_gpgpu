#include "../common_includes.h"
#include "flow.h"

// For the full pipeline
#include "grayscale.h"
#include "gauss_opt.h"
#include "fifo.h"
#include "ixyt.h"
#include "diff.h"
#include "tensor.h"
#include "median.h"
#include "nms.h"
#include "box_opt.h"

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

    const float offset = 1.0f;
    glUniform1f(texelWidthUniform, (offset/float(outFrameW)));
    glUniform1f(texelHeightUniform, (offset/float(outFrameH)));
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

// -------------
// | |*| + |*| |
// -------------
// | |*| + |*| |
// -------------
// | |*| + |*| |
// -------------

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
 
 const int wSize = 6;
 
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
             float w = 1.0; // cos(dot(delta,delta)*40.0);
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
     vec2 uv = strength * (vec2(X*B - C*Y, A*Y - X*C) * D);
     vec4 flow = vec4(((-uv + 1.0) / 2.0), (center.xy + 1.0) / 2.0);
     gl_FragColor = flow;
 });

// ============== convenience =========================

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
FlowPipeline::~FlowPipeline() { }
ProcInterface * FlowPipeline::first() { return &m_pImpl->grayProc; }
ProcInterface * FlowPipeline::last() { return &m_pImpl->flowProc; }
float FlowPipeline::getStrength() const { return m_pImpl->flowProc.getStrength(); }

// ====================================================
// ======= Test two input smoothed tensor output ======
// ====================================================

FlowImplProc::FlowImplProc(bool isX, float strength) : isX(isX), strength(strength) {}

void FlowImplProc::filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target)
{
    ProcBase::createShader(vShaderSrc, fShaderSrc, target);
    shParamAPos = shader->getParam(ATTR, "position");
    shParamATexCoord = shader->getParam(ATTR, "inputTextureCoordinate");
}

void FlowImplProc::getUniforms()
{
    FilterProcBase::getUniforms();
    shParamUInputTex = shader->getParam(UNIF, "inputImageTexture");
    shParamUStrength = shader->getParam(UNIF, "strength");
}

void FlowImplProc::setUniforms()
{
    FilterProcBase::setUniforms();
    glUniform1f(shParamUStrength, strength);
}

const char * FlowImplProc::fshaderFlowXSrc = OG_TO_STR
(
#if defined(OGLES_GPGPU_OPENGLES)
 precision highp float;
#endif
 varying vec2 textureCoordinate;
 uniform sampler2D inputImageTexture;
 uniform float strength;
 void main()
 {
     vec4 val = texture2D(inputImageTexture, textureCoordinate);
     vec4 pix = (val * 2.0) - 1.0;
     vec3 t = vec3(pix.x*pix.x, pix.y*pix.y, (pix.x*pix.y+1.0)/2.0);
     vec4 x = vec4(t, (pix.x*pix.z+1.0)/2.0);
     gl_FragColor = x;
 });

const char *FlowImplProc::fshaderFlowYSrc = OG_TO_STR
(
#if defined(OGLES_GPGPU_OPENGLES)
 precision highp float;
#endif
 
 varying vec2 textureCoordinate;
 uniform sampler2D inputImageTexture;
 uniform float strength;
 
 void main()
 {
     vec4 val = texture2D(inputImageTexture, textureCoordinate);
     vec4 pix = (val * 2.0) - 1.0;
     vec3 t = vec3(pix.x*pix.x, pix.y*pix.y, (pix.x*pix.y+1.0)/2.0);
     vec4 y = vec4(t, (pix.y*pix.z+1.0)/2.0);
     gl_FragColor = y;
 });


//##########################################################################
//                   +=> [Ix^2; Ix*Iy; Iy^2; Ix*It] => SMOOTH ===+
//  [Ix; Iy; It; .]  |                                           | => FLOW
//                   +=> [Ix^2; Ix*Iy; Iy^2; Iy*It] => SMOOTH ===+
//##########################################################################

Flow2Proc::Flow2Proc(float tau, float strength) : tau(tau), strength(strength) {}

void Flow2Proc::filterShaderSetup(const char *vShaderSrc, const char *fShaderSrc, GLenum target)
{
    ProcBase::createShader(vShaderSrc, fShaderSrc, target);
    shParamAPos = shader->getParam(ATTR, "position");
    shParamATexCoord = shader->getParam(ATTR, "inputTextureCoordinate");
}

void Flow2Proc::getUniforms()
{
    TwoInputProc::getUniforms();
    shParamUStrength = shader->getParam(UNIF, "strength");
    shParamUTau = shader->getParam(UNIF, "tau");
}

void Flow2Proc::setUniforms()
{
    TwoInputProc::setUniforms();
    glUniform1f(shParamUStrength, strength);
    glUniform1f(shParamUTau, tau);
}

const char *Flow2Proc::fshaderFlowSrc = OG_TO_STR
(
#if defined(OGLES_GPGPU_OPENGLES)
 precision highp float;
#endif
 
 varying vec2 textureCoordinate;
 uniform sampler2D inputImageTexture;
 uniform sampler2D inputImageTexture2;

 uniform float strength;
 uniform float tau; // noise threshold (0.004)
 
 void main()
 {
     vec4 pix1 = texture2D(inputImageTexture, textureCoordinate);
     vec4 pix2 = texture2D(inputImageTexture2, textureCoordinate);
     
     float A = pix1.x;             // Ix^2
     float B = pix1.y;             // Iy^2
     float C = pix1.z * 2.0 - 1.0; // Ix*Iy
     float X = pix1.w * 2.0 - 1.0; // Ix * It
     float Y = pix2.w * 2.0 - 1.0; // Iy * It
     
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
     
     // Sort such that L1 < L2
     if(L1 > L2)
     {
         float L = L1;
         L1 = L2;
         L2 = L;
     }
     
     if(L1 <= tau)
     {
         D = 0.0;
     }
     
     vec2 uv = strength * (vec2(X*B - C*Y, A*Y - X*C) * D);
     vec4 flow = vec4(((-uv + 1.0) / 2.0), L1, L2); // TODO: L1,L2 need scaling
     gl_FragColor = flow;
 });


// =========================================

#define USE_MEDIAN 0

struct Flow2Pipeline::Impl
{
    Impl(float tau, float strength, bool doGray)
    : diffProc(40.0)
    , flowXProc(true, 1.f)
    , flowXSmoothProc(2.0)
    , flowYProc(false, 1.f)
    , flowYSmoothProc(2.0)
    , flowProc(tau, strength)
    {
        if(!doGray)
        {
            grayProc.setGrayscaleConvType(GRAYSCALE_INPUT_CONVERSION_NONE);
        }
        
        { // flow processing
            grayProc.add(&fifoProc);
            fifoProc.add(&diffProc, 1);
            
            grayProc.add(&diffProc, 0);

            diffProc.add(&flowXProc);
            flowXProc.add(&flowXSmoothProc);
            flowXSmoothProc.add(&flowProc, 0);
            
            diffProc.add(&flowYProc);
            flowYProc.add(&flowYSmoothProc);
            flowYSmoothProc.add(&flowProc, 1);
        
            nmsProc.swizzle(2); // b channel
            nmsProc.setThreshold(0.1f);
            
            flowProc.add(&nmsProc);
            
#if USE_MEDIAN
            flowProc.add(&medianProc);
#endif
        }
    }
    
    GrayscaleProc grayProc;
    FIFOPRoc fifoProc;
    IxytProc diffProc;
    
    FlowImplProc flowXProc;
    GaussOptProc flowXSmoothProc;
    
    FlowImplProc flowYProc;
    GaussOptProc flowYSmoothProc;

    Flow2Proc flowProc;
    
    NmsProc nmsProc; // corners!
    
#if USE_MEDIAN
    MedianProc medianProc;
#endif
};

Flow2Pipeline::Flow2Pipeline(float tau, float strength, bool doGray)
{
    m_pImpl = std::unique_ptr<Impl>(new Impl(tau, strength, doGray));
    
    procPasses.push_back(&m_pImpl->grayProc);
    procPasses.push_back(&m_pImpl->fifoProc);
    procPasses.push_back(&m_pImpl->diffProc);
    procPasses.push_back(&m_pImpl->flowXProc);
    procPasses.push_back(&m_pImpl->flowXSmoothProc);
    procPasses.push_back(&m_pImpl->flowYProc);
    procPasses.push_back(&m_pImpl->flowYSmoothProc);
    procPasses.push_back(&m_pImpl->flowProc);
    procPasses.push_back(&m_pImpl->nmsProc);
};

Flow2Pipeline::~Flow2Pipeline() {}
ProcInterface * Flow2Pipeline::getInputFilter() const { return &m_pImpl->grayProc; }

#if USE_MEDIAN
ProcInterface * Flow2Pipeline::getOutputFilter() const { return &m_pImpl->medianProc; }
#else
ProcInterface * Flow2Pipeline::getOutputFilter() const { return &m_pImpl->nmsProc; }
#endif

float Flow2Pipeline::getStrength() const { return m_pImpl->flowProc.getStrength(); }

ProcInterface * Flow2Pipeline::corners() { return &m_pImpl->nmsProc; }

int Flow2Pipeline::render(int position)
{
    // Execute internal filter chain
    getInputFilter()->process(position);
    return 0;
}

int Flow2Pipeline::init(int inW, int inH, unsigned int order, bool prepareForExternalInput)
{
    getInputFilter()->prepare(inW, inH, 0, INT_MAX, 0);
    return 0;
}

int Flow2Pipeline::reinit(int inW, int inH, bool prepareForExternalInput)
{
    getInputFilter()->prepare(inW, inH, 0, INT_MAX, 0);
    return 0;
}

END_OGLES_GPGPU




