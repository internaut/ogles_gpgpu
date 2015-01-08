//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "tools.h"

#include "common_includes.h"

#include <cmath>

#ifndef log2f
#define log2f(v) logf((v)) / log(2)
#endif

using namespace ogles_gpgpu;
using namespace std;

#ifdef OGLES_GPGPU_BENCHMARK
clock_t Tools::startTick = 0;
vector<double> Tools::timeMeasurements;
#endif

void Tools::checkGLErr(const char *cls, const char *msg) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
        OG_LOGERR(cls, "%s - GL error '%d' occured", msg, err);
	}
}

bool Tools::isPOT(float v) {
    float pow = log2f(v);
    return ceilf(pow) == pow;
}

float Tools::getBiggerPOTValue(float v) {
    return powf(2.0f, ceilf(log2f(v)));
}

vector<string> Tools::split(const string &s, char delim) {
    vector<string> elems;
    stringstream strs(s);
    string item;
    
    while (getline(strs, item, delim)) {
        elems.push_back(item);
    }
    
    return elems;
}

void Tools::strReplaceAll(string& str, const string& from, const string& to) {
    if (from.empty())
        return;
    
    size_t start_pos = 0;
    
    while((start_pos = str.find(from, start_pos)) != string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

#ifdef OGLES_GPGPU_BENCHMARK
void Tools::resetTimeMeasurement() {
    startTick = 0;
    timeMeasurements.clear();
}

void Tools::startTimeMeasurement() {
    startTick = clock();
}

void Tools::stopTimeMeasurement() {
    double ms = getTicksDiffInMs(startTick, clock());
    timeMeasurements.push_back(ms);
}

double Tools::getTicksDiffInMs(clock_t t1, clock_t t2) {
    return (((double)(t2 - t1) / CLOCKS_PER_SEC) * 1000.0);
}
#endif