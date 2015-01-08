//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

/**
 * Common tools collection.
 */
#ifndef OGLES_GPGPU_COMMON_TOOLS
#define OGLES_GPGPU_COMMON_TOOLS

#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <cstdio>

using namespace std;

namespace ogles_gpgpu {

/**
 * Common tools collection.
 */
class Tools {
public:
    /**
     * Check for an OpenGL error in the previous call(s). Produce error
     * message in class <cls> with prefix <msg>.
     */
    static void checkGLErr(const char *cls, const char *msg);
    
    /**
     * Check if <v> is a power-of-two (POT) value.
     */
    static bool isPOT(float v);
    
    /**
     * Get the next bigger POT value if <v> is not a POT value.
     */
    static float getBiggerPOTValue(float v);
    
    /**
     * Split a string <s> by delimiter <delim>.
     */
    static vector<string> split(const string &s, char delim = ' ');
    
    /**
     * Replace all strings <from> in <str> by <to>.
     * Code from http://stackoverflow.com/a/3418285.
     */
    static void strReplaceAll(string& str, const string& from, const string& to);

#ifdef OGLES_GPGPU_BENCHMARK
    static void resetTimeMeasurement();
    static void startTimeMeasurement();
    static void stopTimeMeasurement();
    
    static double getTicksDiffInMs(clock_t t1, clock_t t2);
    static vector<double> getTimeMeasurements() { return timeMeasurements; }
#endif
    
private:
    
#ifdef OGLES_GPGPU_BENCHMARK
    static clock_t startTick;
    static vector<double> timeMeasurements;
#endif
};
    
}

#endif
