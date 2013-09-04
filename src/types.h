/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_TYPES_H_INCLUDED
#define DD_TYPES_H_INCLUDED

#include <math.h>

typedef signed char SInt8;
typedef unsigned char UInt8;
typedef short SInt16;
typedef unsigned short UInt16;
typedef unsigned long UInt32;
typedef long long SInt64;
typedef int fixed; /* signed 22.10 */

typedef int boolean; /* or is char better? */

#define FIXED_SHIFT 10

#define FIXED_I(i) ((i)<<FIXED_SHIFT)

/* doesn't work for d<1:
	#define FIXED_D(d) (((int)floor(d))<<FIXED_SHIFT) */

#define FIXED_D(d) ((int)floor((d)*(1<<FIXED_SHIFT)+0.5))

/* fixed from twips */
#define FIXED_T(t) (((t)<<FIXED_SHIFT)/20)

/* shouldn't ever need this, inside dd all units are fixed screen coords
   and twips are never exposed to the user */

#define TWIPS_F(f) (((f)*20)>>FIXED_SHIFT)
#define TWIPS_I(i) ((i)*20)

#define DOUBLE_F(f) (((f)<0)?(-(-(f)/(double)(1<<FIXED_SHIFT))):((f)/(double)(1<<FIXED_SHIFT)))
#define FLOAT_F(f) (((f)<0)?(-(-(f)/(float)(1<<FIXED_SHIFT))):((f)/(float)(1<<FIXED_SHIFT)))
#define INT_F(f) ((f)>>FIXED_SHIFT)

/* the fractional part of f, left scaled up as an int */
#define FRACT_F(f) ((f)&((1<<FIXED_SHIFT)-1))

/* XXX - should be optimized: */
static inline fixed
fixed_mult(fixed a, fixed b)
{
	return (fixed)(((long long)a * b) >> FIXED_SHIFT);
}

static inline fixed
fixed_divide(fixed a, fixed b)
{
	return FIXED_D(FLOAT_F(a) / FLOAT_F(b));
}

fixed
fixed_sqrt(unsigned int d);

typedef unsigned int ddColor;

#ifdef DD_COLOR_SAMPLE_ARGB
  #define COLOR_ALPHA(c) (((c)>>24)&0xff)
  #define COLOR_RED(c) (((c)>>16)&0xff)
  #define COLOR_GREEN(c) (((c)>>8)&0xff)
  #define COLOR_BLUE(c) ((c)&0xff)
  #define DDCOLOR(a,r,g,b) ((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff))
  #define SETALPHA(c,a) (((c)&0xffffff)|((a)<<24))
#else
  #define COLOR_RED(c) (((c)>>24)&0xff)
  #define COLOR_GREEN(c) (((c)>>16)&0xff)
  #define COLOR_BLUE(c) (((c)>>8)&0xff)
  #define COLOR_ALPHA(c) ((c)&0xff)
  #define DDCOLOR(a,r,g,b) ((((r)&0xff)<<24)|(((g)&0xff)<<16)|(((b)&0xff)<<8)|((a)&0xff))
  #define SETALPHA(c,a) (((c)&0xffffff00)|(a))
#endif

static inline ddColor
ddColor_scale(ddColor color, int scale) /* 0x100 = 1.0 */
{
	if ( scale == 0xff )
		return color;

	return ((((color&0xff000000)>>8)*scale) & 0xff000000) |
		(((((color&0x00ff0000)*scale) & 0xff000000) |
	(((color&0x0000ff00)*scale) & 0x00ff0000) |
	(((color&0x000000ff)*scale) & 0x0000ff00)) >> 8);
}

static inline ddColor
ddColor_premultiply(ddColor color)
{
	int alpha = COLOR_ALPHA(color);
	
	if ( alpha == 0xff )
		return color;

	return ddColor_scale(SETALPHA(color, 0xff), alpha);
}


#ifndef min
  #define dd_min(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef max
  #define dd_max(a,b) (((a)>(b))?(a):(b))
#endif

#ifdef WIN32
  #include <float.h>
  #define rint(a) floor((a)+0.5)
  #define isnan(a) _isnan(a)
  #define finite(a) _finite(a)
#endif

#ifndef DD_FALSE
  #define DD_FALSE (boolean)0
#endif

#ifndef DD_TRUE
  #define DD_TRUE (boolean)1
#endif

#ifndef M_PI
  #define M_PI 3.14159265358979
#endif

#endif /* DD_TYPES_H_INCLUDED */
