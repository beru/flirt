/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:
 
 * Redistributions of source code must retain the above copyright notice
   and URL, this list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright
   notice and URL, this list of conditions and the following disclaimer
   in the documentation and/or other materials provided with the distribution.   
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DD_FLIRT_H_INCLUDED
#define DD_FLIRT_H_INCLUDED

#include <math.h>

typedef int fixed; /* signed 22.10 */
typedef int boolean;

#define FIXED_SHIFT 10

#define FIXED_I(i) ((i)<<FIXED_SHIFT)

/* doesn't work for d<1:
#define FIXED_D(d) (((int)floor(d))<<FIXED_SHIFT) */

#define FIXED_D(d) ((int)floor((d)*(1<<FIXED_SHIFT)+0.5))

/* fixed from twips */
#define FIXED_T(t) (((t)<<FIXED_SHIFT)/20)

/* XXX - slight rounding error in above.	need to determine if
((t)<<FIXED_SHIFT)/20 would be feasible */

/* shouldn't ever need this, inside dd all units are fixed screen coords
and twips are never exposed to the user */

#define TWIPS_F(f) (((f)*20)>>FIXED_SHIFT)
#define TWIPS_I(i) ((i)*20)

#define DOUBLE_F(f) ((f)/(double)(1<<FIXED_SHIFT))
#define INT_F(f) ((f)>>FIXED_SHIFT)

/* the fractional part of f, left scaled up as an int */
#define FRACT_F(f) ((f)&((1<<FIXED_SHIFT)-1))

/* XXX - should be optimized: */
static fixed inline fixed_mult(fixed a, fixed b)
{
	return (fixed)(((long long)a*b) >> FIXED_SHIFT);
}

static fixed inline fixed_divide(fixed a, fixed b)
{
	return FIXED_D(DOUBLE_F(a)/DOUBLE_F(b));
}


typedef unsigned int ddColor;

#define COLOR_ALPHA(c) (((c)>>24)&0xff)
#define COLOR_RED(c) (((c)>>16)&0xff)
#define COLOR_GREEN(c) (((c)>>8)&0xff)
#define COLOR_BLUE(c) ((c)&0xff)

#define DDCOLOR(a,r,g,b) ((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff))

ddColor
ddColor_scale(ddColor color, int scale); /* 0x100 = 1.0 */


typedef struct _ddImage ddImage;

ddImage*
dd_newImage(int width, int height);

ddImage*
dd_newImageFromBuffer(char* buffer, int width, int rowBytes, int height);

void
ddImage_setBuffer(ddImage* image, char* buffer);

char*
ddImage_getData(ddImage* image);

int
ddImage_getRowstride(ddImage* image);

void
dd_destroyImage(ddImage* image);

void
ddImage_clear(ddImage* image, ddColor color);


typedef struct _ddRect ddRect;

struct _ddRect
{
	fixed left;
	fixed right;
	fixed top;
	fixed bottom;
};


typedef struct _ddPlayer ddPlayer;

void
dd_destroyPlayer(ddPlayer* player);

ddPlayer*
dd_newPlayer_file(FILE *f, ddImage* image);

int
ddPlayer_readMovie(ddPlayer* player);

void
ddPlayer_setImage(ddPlayer* player, ddImage* image);

void
ddPlayer_step(ddPlayer* player);

void
ddPlayer_updateDisplay(ddPlayer* player, ddRect** outRects, int* outCount);

float
ddPlayer_getFrameRate(ddPlayer* player);

ddColor
ddPlayer_getBackgroundColor(ddPlayer* player);


#define DD_SAMPLE_MAX_POS 0x00ffffff
#define DD_SAMPLE_MAX_NEG 0xff800000

#define DD_SAMPLE_SINT16(s) \
	(short)(((s)>DD_SAMPLE_MAX_POS)?0x7fff:(((s)<DD_SAMPLE_MAX_NEG)?0x8000:(s)))

typedef struct
{
	int left;
	int right;
} ddAudioSample;

boolean
ddPlayer_renderFrameSoundIntoBuffer(ddPlayer* player, ddAudioSample* buffer, int* ioSize);


typedef struct _ddActionMovieClip ddActionMovieClip;

ddActionMovieClip*
ddPlayer_getRootClip(ddPlayer* player);

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

void dd_setErrorFunction(void (*error)(char *msg, ...));
void dd_setWarnFunction(void (*warn)(char *msg, ...));

boolean dd_init();

unsigned long dd_getTicks();

void
dd_setActionTraceFunction(ddPlayer* player, void (*trace)(const char* message, void* userdata), void* userdata);

typedef enum
{
	DD_ARROW_CURSOR = 0,
	DD_BUTTON_CURSOR,
	DD_TEXTFIELD_CURSOR
} ddCursorType;

ddCursorType
ddPlayer_doMouseMove(ddPlayer* player, int x, int y);

void
ddPlayer_doMouseDown(ddPlayer* player, int x, int y);

void
ddPlayer_doMouseUp(ddPlayer* player, int x, int y);

void
ddPlayer_doKeyDown(ddPlayer* player, unsigned short code);

void
ddPlayer_doKeyUp(ddPlayer* player, unsigned short code);


typedef struct _ddMovieClip ddMovieClip;

ddMovieClip*
ddPlayer_getMovieClip(ddPlayer* player);

typedef struct _ddDrawClip ddDrawClip;

void
ddDrawClip_renderInImage(ddDrawClip* clip, ddImage* image, ddColor background);

ddDrawClip*
ddActionMovieClip_getDrawClip(ddActionMovieClip* clip);

typedef struct _ddActionObject ddActionObject;
typedef struct _ddActionHash ddActionHash;
typedef struct _ddActionContext ddActionContext;
typedef struct _ddActionFunction ddActionFunction;

typedef enum
{
	VALUE_NULL,
	VALUE_UNDEF,
	VALUE_INTEGER,
	VALUE_DOUBLE,
	VALUE_BOOLEAN,
	VALUE_STRING,
	VALUE_OBJECT,
	VALUE_FUNCTION
} ddActionValueType;


struct _ddActionValue
{
	ddActionValueType type;
	
	union
	{
		int intValue;
		double doubleValue;
		ddActionObject* objectValue;
		ddActionFunction* functionValue;
		char* stringValue;
	} data;
};


typedef struct _ddActionValue ddActionValue;

extern ddActionValue ddNullValue;
extern ddActionValue ddTrueValue;
extern ddActionValue ddFalseValue;

ddActionObject*
ddActionValue_getObjectValue(ddActionValue value);

ddActionObject*
ddActionObject_retain(ddActionObject* object);

void
ddActionObject_release(ddActionObject* object);

char*
ddActionValue_getStringValue(ddActionContext* context, ddActionValue value);

int
ddActionValue_getIntValue(ddActionValue value);

ddActionValue
dd_newActionValue_object(ddActionObject* object);

typedef struct _ddActionClass ddActionClass;

struct _ddActionObject
{
	int refCount;
	ddActionObject* proto;
	ddActionHash* properties;
	
	void (*cachedSetProperty)(ddActionContext* context, ddActionObject* obj, char* name, ddActionValue value);
	ddActionValue (*cachedGetProperty)(ddActionContext* context, ddActionObject* obj, const char* name);
	
	ddActionClass* nativeClass;
};

void
ddActionObject_init(ddActionObject* object);

extern ddActionClass* ddActionArrayClass;
extern ddActionClass* ddActionColorClass;
extern ddActionClass* ddActionDateClass;
extern ddActionClass* ddActionFunctionClass;
extern ddActionClass* ddActionMovieClipClass;
extern ddActionClass* ddActionObjectClass;
extern ddActionClass* ddActionStringClass;

#define FUNCTION_VARARGS -1  /* if it's doing its own arg handling */

typedef ddActionValue (*ddNativeFunction)(ddActionObject* object, ddActionContext* context, int nargs);

ddActionClass*
ddPlayer_addClass(ddPlayer* player, ddActionClass* superclass, const char* name, ddNativeFunction constructor, int nargs);

ddActionClass*
dd_newActionClass();

void
ddActionClass_setSuperclass(ddActionClass* classObject, ddActionClass* superclass);

void
ddActionClass_addNativeMethod(ddActionClass* classObject, char* name, ddNativeFunction function, int nargs);

ddActionFunction*
dd_newActionNativeFunction(ddNativeFunction function, int nargs);

boolean
ddActionObject_isKindOfClass(ddActionObject* object, ddActionClass* classPrototype);

void
ddActionObject_setClass(ddActionObject* object, ddActionClass* class);

ddActionObject*
ddActionObject_getPrototype(ddActionObject* object);

ddActionValue
ddActionContext_popValue(ddActionContext* context);

ddPlayer*
ddContext_getPlayer(ddActionContext* context);

#endif /* DD_FLIRT_H_INCLUDED */
