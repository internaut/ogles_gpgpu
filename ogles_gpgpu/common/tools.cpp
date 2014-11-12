#include "tools.h"

#include "common_includes.h"

using namespace ogles_gpgpu;
using namespace std;

void Tools::checkGLErr(const char *msg) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
        cerr << msg << " - GL error " << err << " occurred" << endl;
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