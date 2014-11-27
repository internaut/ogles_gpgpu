#include "tools.h"

#include "common_includes.h"

#include <cmath>

using namespace ogles_gpgpu;
using namespace std;

#ifdef OGLES_GPGPU_BENCHMARK
clock_t Tools::startTick = 0;
vector<float> Tools::timeMeasurements;
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

#ifdef OGLES_GPGPU_BENCHMARK
void Tools::resetTimeMeasurement() {
    startTick = 0;
    timeMeasurements.clear();
}

void Tools::startTimeMeasurement() {
    startTick = clock();
}

void Tools::stopTimeMeasurement() {
    float ms = getTicksDiffInMs(startTick, clock());
    timeMeasurements.push_back(ms);
}

float Tools::getTicksDiffInMs(clock_t t1, clock_t t2) {
    return (float)(((double)(t2 - t1) / CLOCKS_PER_SEC) * 1000.0);
}
#endif