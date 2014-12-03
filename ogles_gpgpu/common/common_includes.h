/**
 * Common header includes
 */

#include <iostream>
#include <cassert>

#ifdef __APPLE__
    #include "../platform/ios/gl_includes.h"
#else
    #include "../platform/android/gl_includes.h"
#endif

#include "tools.h"
#include "types.h"