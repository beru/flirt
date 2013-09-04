
// These are determined in Makefiles now:

//#define DD_PLATFORM_MAC_OS_X
//#define DD_PLATFORM_CYGWIN
//#define DD_PLATFORM_LINUX

//#define DD_BIG_ENDIAN
//#define DD_LITTLE_ENDIAN

// render top-down, so we can ignore geometry that's obscured
#define DD_RENDER_TOPDOWN

// choose the correct sample order for your platform
#define DD_COLOR_SAMPLE_RGBA
//#define DD_COLOR_SAMPLE_ARGB

// comment out if these are missing on your system
#define HAVE_ZLIB
#define HAVE_JPEGLIB
#define HAVE_MAD

// enable debugging/logging features
//#define DD_DEBUG
#define DD_LOG

// include functions to allow poking at the runtime
#define DD_INCLUDE_DEBUGGER
