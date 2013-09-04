/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "updatelist.h"

ddUpdateList*
dd_newUpdateList()
{
	ddUpdateList* list = dd_malloc(sizeof(ddUpdateList));
	list->nAlloced = 0;
	list->nRects = 0;
	list->rects = NULL;
	return list;
}


void
dd_destroyUpdateList(ddUpdateList* list)
{
	if ( list->rects != NULL )
		dd_free(list->rects);
	
	dd_free(list);
}


void
ddUpdateList_clear(ddUpdateList* list)
{
	list->nRects = 0;
}


void
ddUpdateList_constrainToRect(ddUpdateList* list, ddRect bounds)
{
	int i, j;

	for ( i = 0, j = 0; i < list->nRects; ++i )
	{
		if ( ddRect_intersectsRect(list->rects[i], bounds) )
		{
			list->rects[j] = ddRect_intersectRect(list->rects[i], bounds);
			++j;
		}
	}

	list->nRects = j;
}


#ifdef DD_RENDER_TOPDOWN

void
ddUpdateList_fillImageBackground(ddUpdateList* list,
								 ddImage* image, ddColor background)
{
	int i;

	for ( i = 0; i < list->nRects; ++i )
		ddImage_blendRect(image, list->rects[i], background);
}

#endif /* DD_RENDER_TOPDOWN */


int
ddUpdateList_intersectsRect(ddUpdateList* list, ddRect rect)
{
	int i;

	for ( i = 0; i < list->nRects; ++i )
	{
		if ( ddRect_intersectsRect(rect, list->rects[i]) )
			return 1;
	}

	return 0;
}


void
ddUpdateList_makeIntegerRects(ddUpdateList* list)
{
	int i;

	for ( i = 0; i < list->nRects; ++i )
		list->rects[i] = ddRect_integerRect(list->rects[i]);
}


#define UPDATELIST_INCREMENT 4

/*
 if new rect is cut fully across either dimension by an existing rect,
 we can include just the difference
 */

void
ddUpdateList_includeRect(ddUpdateList* list, ddRect rect)
{
	int i;
	int j;
	
	if ( !RECT_VALID(rect) )
		return;

	for ( i = 0; i < list->nRects; ++i )
	{
		ddRect hit = list->rects[i];

		if ( ddRect_intersectsRect(rect, hit) )
		{
			hit = ddRect_containRect(hit, rect);

			for ( j = i + 1; j < list->nRects; ++j )
			{
				if ( ddRect_intersectsRect(hit, list->rects[j]) )
				{
					hit = ddRect_containRect(hit, list->rects[j]);
					list->rects[j] = ddInvalidRect;
				}
			}

			list->rects[i] = hit;

			return;
		}
	}

	if ( list->nRects == list->nAlloced )
	{
		list->nAlloced += UPDATELIST_INCREMENT;
		list->rects = dd_realloc(list->rects, list->nAlloced * sizeof(ddRect));
	}

	list->rects[list->nRects] = rect;
	++list->nRects;
}


void
ddUpdateList_getRects(ddUpdateList* list, ddRect** outRects, int* outCount)
{
	if ( outRects != NULL )
		*outRects = list->rects;
	
	if ( outCount != NULL )
		*outCount = list->nRects;
}


ddRect
ddUpdateList_containingRect(ddUpdateList* list)
{
	int i;
	ddRect bounds = ddInvalidRect;
	
	for ( i = 0; i < list->nRects; ++i )
		bounds = ddRect_containRect(bounds, list->rects[i]);

	return bounds;
}
