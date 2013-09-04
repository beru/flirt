/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_RECT_H_INCLUDED
#define DD_RECT_H_INCLUDED

#include "../dd.h"

typedef struct _ddRect ddRect;

struct _ddRect
{
  fixed left;
  fixed right;
  fixed top;
  fixed bottom;
};

#define RECT_INVALID_NUMBER 0x7fffffff
#define RECT_VALID(r) ((r).left != RECT_INVALID_NUMBER)

extern const ddRect ddInvalidRect;

ddRect
dd_makeRect(fixed left, fixed right, fixed top, fixed bottom);

ddRect
ddRect_containPoint(ddRect rect, fixed x, fixed y);

boolean
ddRect_containsPoint(ddRect rect, fixed x, fixed y);

ddRect
ddRect_intersectRect(ddRect r1, ddRect r2);

boolean
ddRect_intersectsRect(ddRect r1, ddRect r2);

ddRect
ddRect_containRect(ddRect r1, ddRect r2);

ddRect
ddRect_constrain(ddRect rect, fixed left, fixed right, fixed top, fixed bottom);

void
ddRect_constrainPoint(ddRect rect, fixed *x, fixed *y);

ddRect
ddRect_integerRect(ddRect rect);

fixed
ddRect_getWidth(ddRect r);

fixed
ddRect_getHeight(ddRect r);

ddRect
ddRect_displace(ddRect r, int dx, int dy);

#endif /* DD_RECT_H_INCLUDED */
