//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "../common_includes.h"
#include "grayscale.h"

using namespace std;
using namespace ogles_gpgpu;

const GLfloat GrayscaleProc::grayscaleConvVecRGB[3] = {
    0.299, 0.587, 0.114
};

const GLfloat GrayscaleProc::grayscaleConvVecBGR[3] = {
    0.114, 0.587, 0.299
};

const char *GrayscaleProc::fshaderGrayscaleSrc = OG_TO_STR(

#if defined(OGLES_GPGPU_OPENGLES)
precision mediump float;
#endif

uniform sampler2D uInputTex;
uniform vec3 uInputConvVec;
varying vec2 vTexCoord;

void main()
{
    float gray = dot(texture2D(uInputTex, vTexCoord).rgb, uInputConvVec);
    gl_FragColor = vec4(gray, gray, gray, 1.0);
}
);

GrayscaleProc::GrayscaleProc() {
    // set defaults
    inputConvType = GRAYSCALE_INPUT_CONVERSION_NONE;
    setGrayscaleConvType(GRAYSCALE_INPUT_CONVERSION_RGB);
}

void GrayscaleProc::setUniforms() {
    glUniform3fv(shParamUInputConvVec, 1, grayscaleConvVec);        // set additional uniforms
}

void GrayscaleProc::getUniforms() {
    shParamUInputConvVec = shader->getParam(UNIF, "uInputConvVec");
}

void GrayscaleProc::setGrayscaleConvVec(const GLfloat v[3]) {
    inputConvType = GRAYSCALE_INPUT_CONVERSION_CUSTOM;
    memcpy(grayscaleConvVec, v, sizeof(GLfloat) * 3);
}

void GrayscaleProc::setGrayscaleConvType(GrayscaleInputConversionType type) {
    if (inputConvType == type) return;  // no change

    const GLfloat *v = NULL;

    if (type == GRAYSCALE_INPUT_CONVERSION_RGB) {
        v = &grayscaleConvVecRGB[0];
    } else if (type == GRAYSCALE_INPUT_CONVERSION_BGR) {
        v = &grayscaleConvVecBGR[0];
    } else {
        OG_LOGERR(getProcName(), "unknown grayscale input conversion type %d", type);
        v = &grayscaleConvVecRGB[0];    // set default
    }

    memcpy(grayscaleConvVec, v, sizeof(GLfloat) * 3);

    inputConvType = type;
}
