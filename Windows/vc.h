#pragma once

#ifdef _MSC_VER

#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#define snprintf _snprintf
#define strcasecmp stricmp

#ifndef __cplusplus
#define inline __inline
#endif // __cplusplus

// http://www.velocityreviews.com/forums/t532986-rounding-functions-in-microsoft-visual-c-c.html
double rint(double x);

#include <float.h>
#define INFINITY (DBL_MAX+DBL_MAX)
#define NAN (INFINITY-INFINITY)

#define finite _finite
#define isnan _isnan

#endif // #ifdef _MSC_VER
