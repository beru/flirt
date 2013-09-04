/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>

#include "rect.h"

const ddRect ddInvalidRect = { RECT_INVALID_NUMBER, 0, 0, 0 };

ddRect
dd_makeRect(fixed left, fixed right, fixed top, fixed bottom)
{
	ddRect r;

	r.left = left;
	r.right = right;
	r.top = top;
	r.bottom = bottom;

	return r;
}


fixed
ddRect_getWidth(ddRect r)
{
	if ( RECT_VALID(r) )
		return r.right - r.left;
	else
		return 0;
}


fixed
ddRect_getHeight(ddRect r)
{
	if ( RECT_VALID(r) )
		return r.bottom - r.top;
	else
		return 0;
}


ddRect
ddRect_containPoint(ddRect rect, fixed x, fixed y)
{
	if ( RECT_VALID(rect) )
	{
		if ( rect.left > x )
			rect.left = x;

		if ( rect.right < x )
			rect.right = x;

		if ( rect.top > y )
			rect.top = y;

		if ( rect.bottom < y )
			rect.bottom = y;
	}
	else
	{
		rect.left = rect.right = x;
		rect.top = rect.bottom = y;
	}

	return rect;
}


/* returns true if rect contains the given point */
boolean
ddRect_containsPoint(ddRect rect, fixed x, fixed y)
{
	if ( !RECT_VALID(rect) )
		return DD_FALSE;

	return x >= rect.left && x <= rect.right &&
		y >= rect.top && y <= rect.bottom;
}


/* returns true if the two rects intersect */
boolean
ddRect_intersectsRect(ddRect r1, ddRect r2)
{
	if ( !RECT_VALID(r1) || !RECT_VALID(r2) )
		return DD_FALSE;

	return !(r1.left > r2.right || r1.right < r2.left ||
		  r1.top > r2.bottom || r1.bottom < r2.top);
}

/* same as above, but constrains r1 to the intersection of r1 and r2
if such exists */

ddRect
ddRect_intersectRect(ddRect r1, ddRect r2)
{
	if ( !RECT_VALID(r1) )
		return r1;

	if ( !RECT_VALID(r2) )
		return r2;
		
	if ( r1.left > r2.right || r1.right < r2.left ||
		 r1.top > r2.bottom || r1.bottom < r2.top )
		return ddInvalidRect;

	r1.left = dd_max(r1.left, r2.left);
	r1.right = dd_min(r1.right, r2.right);
	r1.top = dd_max(r1.top, r2.top);
	r1.bottom = dd_min(r1.bottom, r2.bottom);

	return r1;
}

/* make r1 big enough to contain r2 */
ddRect
ddRect_containRect(ddRect r1, ddRect r2)
{
	if ( !RECT_VALID(r2) )
		return r1;

	if ( !RECT_VALID(r1) )
		return r2;

	r1.left = dd_min(r1.left, r2.left);
	r1.right = dd_max(r1.right, r2.right);
	r1.top = dd_min(r1.top, r2.top);
	r1.bottom = dd_max(r1.bottom, r2.bottom);

	return r1;
}


ddRect
ddRect_integerRect(ddRect rect)
{
	rect.left = (rect.left >> FIXED_SHIFT) << FIXED_SHIFT;
	rect.top = (rect.top >> FIXED_SHIFT) << FIXED_SHIFT;
	rect.right = ((rect.right + ((1 << FIXED_SHIFT) - 1)) >> FIXED_SHIFT) << FIXED_SHIFT;
	rect.bottom = ((rect.bottom + ((1 << FIXED_SHIFT) - 1)) >> FIXED_SHIFT) << FIXED_SHIFT;

	return rect;
}


ddRect
ddRect_constrain(ddRect rect, fixed left, fixed right, fixed top, fixed bottom)
{
	if ( !RECT_VALID(rect) )
		return rect;

	rect.left = dd_max(rect.left, left);
	rect.right = dd_min(rect.right, right);
	rect.top = dd_max(rect.top, top);
	rect.bottom = dd_min(rect.bottom, bottom);

	return rect;
}


void
ddRect_constrainPoint(ddRect rect, fixed *x, fixed *y)
{
	if ( !RECT_VALID(rect) )
		return;

	if ( *x < rect.left )
		*x = rect.left;
	else if ( *x > rect.right )
		*x = rect.right;

	if ( *y < rect.top )
		*y = rect.top;
	else if ( *y > rect.bottom )
		*y = rect.bottom;
}


ddRect
ddRect_displace(ddRect r, int dx, int dy)
{
	r.left += dx;
	r.right += dx;
	r.top += dy;
	r.bottom += dy;
	
	return r;
}
