/**
 * Common header includes
 */

#include <iostream>
#include <cassert>

#ifdef __APPLE__
    #include "../platform/ios/gl_includes.h"
#else
    // TODO: android
#endif

#include "tools.h"
#include "types.h"