/**
 * Common header includes
 */

#include <iostream>
#include <cassert>

#ifdef __APPLE__
    #include "../platform/ios/gl_includes.h"
	#include "macros.h"
#else
    #include "../platform/android/gl_includes.h"
	#include "../platform/android/macros.h"
#endif

#include "tools.h"
#include "types.h"
