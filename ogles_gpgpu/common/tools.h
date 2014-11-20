/**
 * Common tools collection.
 */
#ifndef OGLES_GPGPU_COMMON_TOOLS
#define OGLES_GPGPU_COMMON_TOOLS

#include <vector>
#include <string>
#include <sstream>
#include <ctime>

#define TO_STR_(x) #x
#define TO_STR(x) TO_STR_(x)

using namespace std;

namespace ogles_gpgpu {

/**
 * Common tools collection.
 */
class Tools {
public:
    /**
     * Check for an OpenGL error in the previous call(s). Produce error
     * message with prefix <msg>.
     */
    static void checkGLErr(const char *msg);
    
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

#ifdef OGLES_GPGPU_BENCHMARK
    static void resetTimeMeasurement();
    static void startTimeMeasurement();
    static void stopTimeMeasurement();
    
    static float getTicksDiffInMs(clock_t t1, clock_t t2);
    static vector<float> getTimeMeasurements() { return timeMeasurements; }
#endif
    
private:
    
#ifdef OGLES_GPGPU_BENCHMARK
    static clock_t startTick;
    static vector<float> timeMeasurements;
#endif
};
    
}

#endif