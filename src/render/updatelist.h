/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_RENDER_UPDATELIST_H_INCLUDED
#define DD_RENDER_UPDATELIST_H_INCLUDED

typedef struct _ddUpdateList ddUpdateList;

#include "rect.h"
#include "image.h"

struct _ddUpdateList
{
  int nAlloced;
  int nRects;
  ddRect* rects;
};


ddUpdateList*
dd_newUpdateList();

void
dd_destroyUpdateList(ddUpdateList* list);

void
ddUpdateList_clear(ddUpdateList* list);

int
ddUpdateList_intersectsRect(ddUpdateList* list, ddRect rect);

void
ddUpdateList_constrainToRect(ddUpdateList* list, ddRect bounds);

void
ddUpdateList_fillImageBackground(ddUpdateList* list,
								 ddImage* image, ddColor background);

// round rects out to nearest pixel boundaries
void
ddUpdateList_makeIntegerRects(ddUpdateList* list);

/* make sure this rect is covered by the updatelist */
void
ddUpdateList_includeRect(ddUpdateList* list, ddRect rect);

void
ddUpdateList_getRects(ddUpdateList* list, ddRect** outRects, int* outCount);

ddRect
ddUpdateList_containingRect(ddUpdateList* list);

#endif /* DD_RENDER_UPDATELIST_H_INCLUDED */
