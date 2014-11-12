#ifndef OGLES_GPGPU_COMMON_TOOLS
#define OGLES_GPGPU_COMMON_TOOLS

#include <vector>
#include <string>
#include <sstream>

#define TO_STR_(x) #x
#define TO_STR(x) TO_STR_(x)

using namespace std;

namespace ogles_gpgpu {

class Tools {
public:
    static void checkGLErr(const char *msg);
    
    static bool isPOT(float v);
    static float getBiggerPOTValue(float v);
    
    static vector<string> split(const string &s, char delim = ' ');
};
    
}

#endif