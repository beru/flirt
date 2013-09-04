/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>

#include "image.h"
#include "alphablend.h"

ddImage*
dd_newImage(int w, int h)
{
	ddImage* image = dd_malloc(sizeof(ddImage));

	image->flags = DDIMAGE_ALLOCEDBUFFER;
	image->data = dd_malloc(w * h * sizeof(ddColor));
	image->width = w;
	image->rowstride = w * sizeof(ddColor);
	image->height = h;

	return image;
}


ddImage*
dd_newImageFromBuffer(UInt8* buffer, int width, int rowBytes, int height)
{
	ddImage* image = dd_malloc(sizeof(ddImage));

	image->flags = DDIMAGE_DEFAULT;
	image->data = buffer;
	image->width = width;
	image->rowstride = rowBytes;
	image->height = height;

	return image;
}


void
dd_destroyImage(ddImage* image)
{
	if ( image->flags & DDIMAGE_ALLOCEDBUFFER )
		dd_free(image->data);

	dd_free(image);
}


void
ddImage_compositeTempBuffer(ddImage* image, ddColor* buffer, int y, int left, int right)
{
	ddColor* p = (ddColor*)(image->data + y * image->rowstride);
	int x;

	p += left;

	// XXX - use vimage on OS X
	
	for ( x = left; x < right; ++x )
	{
		//if ( *p != 0 )
		//	printf("not zero!");

		*p = alpha_blend(*p, buffer[x]);
		++p;
	}	
}


UInt8*
ddImage_getData(ddImage* image)
{
	return image->data;
}


int
ddImage_getRowstride(ddImage* image)
{
	return image->rowstride;
}


int
ddImage_getWidth(ddImage* image)
{
	return image->width;
}


int
ddImage_getHeight(ddImage* image)
{
	return image->height;
}


void
ddImage_setBuffer(ddImage* image, UInt8* buffer)
{
	image->data = buffer;
}


void
ddImage_clear(ddImage* image, ddColor color)
{
	int x, y;

	for ( y = 0; y < image->height; ++y )
	{
		ddColor* row = (ddColor*)(image->data + (y * image->rowstride));

		for ( x = 0; x < image->width; ++x )
			row[x] = color;
	}
}


void
ddImage_clearWhite(ddImage* image)
{
	memset(image->data, 0xff, image->rowstride * image->height * sizeof(ddColor));
}


void
ddImage_clearBlack(ddImage* image)
{
	memset(image->data, 0, image->rowstride * image->height * sizeof(ddColor));
}


void
ddImage_fillRect(ddImage* image, ddRect rect, ddColor color)
{
	int x, y;

	if ( !RECT_VALID(rect) )
		return;

	for ( y = INT_F(rect.top); y < INT_F(rect.bottom); ++y )
	{
		ddColor* row = (ddColor*)(image->data + (y * image->rowstride));
		
		for ( x = INT_F(rect.left); x < INT_F(rect.right); ++x )
			row[x] = color;
	}
}


void
ddImage_blendRect(ddImage* image, ddRect rect, ddColor color)
{
	int x, y;

	if ( !RECT_VALID(rect) )
		return;
	
	// XXX - use vImage on OS X

	for ( y = INT_F(rect.top); y < INT_F(rect.bottom); ++y )
	{
		for ( x = INT_F(rect.left); x < INT_F(rect.right); ++x )
		{
			ddColor *p = (ddColor*)(image->data + y * image->rowstride + x * sizeof(ddColor));
			*p = alpha_blend(*p, color);
		}
	}
}


void
ddImage_clearRect(ddImage* image, ddRect rect)
{
	UInt8 *p, *end;
	int w, d;

	if ( !RECT_VALID(rect) )
		return;

	p = image->data + INT_F(rect.top) * image->rowstride + INT_F(rect.left) * sizeof(ddColor);
	end = p + (INT_F(rect.bottom) - INT_F(rect.top)) * image->rowstride;

	w = (INT_F(rect.right) - INT_F(rect.left)) * sizeof(ddColor);
	d = image->rowstride;

	while ( p < end )
	{
		memset(p, 0, w);
		p += d;
	}
}


void
ddImage_fillRect_white(ddImage* image, ddRect rect)
{
	UInt8 *p, *end;
	int w, d;

	if ( !RECT_VALID(rect) )
		return;

	p = image->data + INT_F(rect.top) * image->rowstride + INT_F(rect.left) * sizeof(ddColor);
	end = p + (INT_F(rect.bottom) - INT_F(rect.top)) * image->rowstride;

	w = (INT_F(rect.right) - INT_F(rect.left)) * sizeof(ddColor);
	d = image->rowstride;

	while ( p < end )
	{
		memset(p, 0xff, w);
		p += d;
	}
}


void
ddImage_fillRect_black(ddImage* image, ddRect rect)
{
	UInt8 *p, *end;
	int w, d;

	if ( !RECT_VALID(rect) )
		return;

	p = image->data + INT_F(rect.top) * image->rowstride + INT_F(rect.left) * sizeof(ddColor);
	end = p + (INT_F(rect.bottom) - INT_F(rect.top)) * image->rowstride;

	w = (INT_F(rect.right) - INT_F(rect.left)) * sizeof(ddColor);
	d = image->rowstride;

	while ( p < end )
	{
		memset(p, 0, w);
		p += d;
	}
}
