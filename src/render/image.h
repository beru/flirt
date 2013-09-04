/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_RENDER_IMAGE_H_INCLUDED
#define DD_RENDER_IMAGE_H_INCLUDED

#include "../dd.h"

typedef struct _ddImage ddImage;

#include "rect.h"

typedef enum
{
	DDIMAGE_DEFAULT = 0,
	DDIMAGE_ALLOCEDBUFFER = 1
} ddImageFlags;

struct _ddImage
{
	ddImageFlags flags;
	UInt16 width;
	UInt16 height;
	UInt16 rowstride;
	UInt8* data;
};

ddImage*
dd_newImage(int width, int height);

ddImage*
dd_newImageFromBuffer(UInt8* buffer, int width, int rowBytes, int height);

void
dd_destroyImage(ddImage* image);

void
ddImage_compositeTempBuffer(ddImage* image, ddColor* buffer, int y, int left, int right);

int
ddImage_getWidth(ddImage* image);

int
ddImage_getRowstride(ddImage* image);

int
ddImage_getHeight(ddImage* image);

void
ddImage_clear(ddImage* image, ddColor color);

void
ddImage_clearWhite(ddImage* image);

void
ddImage_clearBlack(ddImage* image);

void
ddImage_fillRect(ddImage* image, ddRect rect, ddColor color);

void
ddImage_blendRect(ddImage* image, ddRect rect, ddColor color);

void
ddImage_fillRect_white(ddImage* image, ddRect rect);

void
ddImage_fillRect_black(ddImage* image, ddRect rect);

void
ddImage_clearRect(ddImage* image, ddRect rect);

UInt8*
ddImage_getData(ddImage* image);

void
ddImage_setBuffer(ddImage* image, UInt8* buffer);


#define bitmap_loc(b,x,y)  ((b)->data[(y)*((b)->width)+(x)])

static inline ddColor
ddImage_getPixel(ddImage* image, int x, int y)
{
	return *(ddColor*)(image->data + y * image->rowstride + x * sizeof(ddColor));
}


static inline ddColor
ddImage_getTiledPixel_nearest(ddImage* image, fixed x, fixed y)
{
	/* XXX - doesn't do negative numbers properly */

	// XXX - there must be something better than this..
	while ( x < 0 )
		x += FIXED_I(image->width);

	while ( y < 0 )
		y += FIXED_I(image->height);
	
	return ddImage_getPixel(image, (INT_F(x) % image->width), (INT_F(y) % image->height));
}


static inline ddColor
ddImage_getClippedPixel_nearest(ddImage* image, fixed x, fixed y)
{
	return ddImage_getPixel(image, dd_max(dd_min(INT_F(x), image->width - 1), 0), dd_max(dd_min(INT_F(y), image->height - 1), 0));
}


static inline ddColor
interp(fixed f, ddColor a, ddColor b)
{
	int alpha;
	int red;
	int green;
	int blue;
	
	if ( a == b )
		return a;
	
	alpha = COLOR_ALPHA(a) + ((f * (COLOR_ALPHA(b) - COLOR_ALPHA(a))) >> FIXED_SHIFT);
	red = COLOR_RED(a) + ((f * (COLOR_RED(b) - COLOR_RED(a))) >> FIXED_SHIFT);
	green = COLOR_GREEN(a) + ((f * (COLOR_GREEN(b) - COLOR_GREEN(a))) >> FIXED_SHIFT);
	blue = COLOR_BLUE(a) + ((f * (COLOR_BLUE(b) - COLOR_BLUE(a))) >> FIXED_SHIFT);

	return DDCOLOR(alpha, red, green, blue);
}


static inline ddColor
ddImage_getTiledPixel_blended(ddImage* image, fixed x, fixed y)
{
	/* XXX - doesn't do negative numbers properly */
	fixed x1 = x % FIXED_I(image->width);
	fixed y1 = y % FIXED_I(image->height);
	fixed x2 = (x + FIXED_I(1)) % FIXED_I(image->width);
	fixed y2 = (y + FIXED_I(1)) % FIXED_I(image->height);
	
	ddColor d1 = ddImage_getPixel(image, INT_F(x1), INT_F(y1));
	ddColor d2 = ddImage_getPixel(image, INT_F(x2), INT_F(y1));
	ddColor d3 = ddImage_getPixel(image, INT_F(x1), INT_F(y2));
	ddColor d4 = ddImage_getPixel(image, INT_F(x2), INT_F(y2));

	fixed fractx = FRACT_F(x1);
	fixed fracty = FRACT_F(y1);

	/* XXX - edges not dealt with properly */
	return interp(fracty, interp(fractx, d1, d2), interp(fractx, d3, d4));
}


static inline ddColor
ddImage_getClippedPixel_blended(ddImage* image, fixed x, fixed y)
{
	fixed x1 = dd_max(dd_min(x, FIXED_I(image->width - 1)), 0);
	fixed y1 = dd_max(dd_min(y, FIXED_I(image->height - 1)), 0);
	fixed x2 = dd_max(dd_min(x + FIXED_I(1), FIXED_I(image->width - 1)), 0);
	fixed y2 = dd_max(dd_min(y + FIXED_I(1), FIXED_I(image->height - 1)), 0);
	
	ddColor d1 = ddImage_getPixel(image, INT_F(x1), INT_F(y1));
	ddColor d2 = ddImage_getPixel(image, INT_F(x2), INT_F(y1));
	ddColor d3 = ddImage_getPixel(image, INT_F(x1), INT_F(y2));
	ddColor d4 = ddImage_getPixel(image, INT_F(x2), INT_F(y2));

	fixed fractx = FRACT_F(x1);
	fixed fracty = FRACT_F(y1);

	/* XXX - edges not dealt with properly */
	return interp(fracty, interp(fractx, d1, d2), interp(fractx, d3, d4));
}

#endif /* DD_IMAGE_H_INCLUDED */
