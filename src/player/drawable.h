/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_DRAWABLE_H_INCLUDED
#define DD_PLAYER_DRAWABLE_H_INCLUDED

#include "../dd.h"

typedef struct _ddDrawable ddDrawable;

#include "character.h"

#include "../render/image.h"
#include "../render/rect.h"
#include "../render/matrix.h"
#include "../render/cxform.h"
#include "../render/updatelist.h"

/* when we move the timeline, each display item is marked if it's still
in the frame.  Then we clear out the unmarked items.. */

#define DRAWABLE_MARK		 (1<<0)
#define DRAWABLE_NEEDSUPDATE (1<<1)

struct _ddDrawable
{
	ddCharacterType type;
	ddCharacter* character;

	void (*getUpdateList)(struct _ddDrawable* d,
					   ddUpdateList* list, ddMatrix matrix);

	void (*drawInImage)(struct _ddDrawable* d, ddImage* image, ddUpdateList* list,
					 ddMatrix matrix, ddCXform cXform, ddRect clipRect);

	void (*destroy)(struct _ddDrawable* d);

	int flags;
	int depth;
	int maskLevel;
	ddDrawable* maskShape;

	// pointers for linked list
	ddDrawable* next;
	ddDrawable* last;

	ddMatrix matrix;
	ddRect bounds;
	ddCXform cXform;
};


static inline void
ddDrawable_init(ddDrawable* d)
{
	d->flags = 0;
	d->matrix = ddIdentityMatrix;
	d->bounds = ddInvalidRect;
	d->cXform = ddEmptyCXform;
	d->maskLevel = -1;
	d->maskShape = NULL;
	d->next = NULL;
	d->last = NULL;
}

static inline void
ddDrawable_getUpdateList(ddDrawable* d, ddUpdateList* list, ddMatrix matrix)
{
	d->getUpdateList(d, list, matrix);

	if ( d->flags & DRAWABLE_NEEDSUPDATE )
		ddUpdateList_includeRect(list, d->bounds);

	d->flags = 0;
}

static inline void
ddDrawable_drawInImage(ddDrawable* d, ddImage* image, ddUpdateList* list,
					   ddMatrix matrix, ddCXform cXform, ddRect clipRect)
{
	d->drawInImage(d, image, list, matrix, cXform, clipRect);
}

static inline void
dd_destroyDrawable(ddDrawable* d)
{
	d->destroy(d);
}

static inline void
ddDrawable_setMaskLevel(ddDrawable* d, int level)
{
	d->maskLevel = level;
}

void
ddDrawable_setMaskDrawable(ddDrawable* d, ddDrawable* mask);

void
ddDrawable_linkDrawableAfter(ddDrawable* left, ddDrawable* right);

void
ddDrawable_linkDrawableBefore(ddDrawable* left, ddDrawable* right);

void
ddDrawable_replace(ddDrawable* old, ddDrawable* new);

int
ddDrawable_hitTest(ddDrawable* d, fixed x, fixed y);

void
ddDrawable_setMatrix(ddDrawable* d, ddMatrix matrix);

ddMatrix
ddDrawable_getMatrix(ddDrawable* d);

void
ddDrawable_setCXform(ddDrawable* d, ddCXform cXform, boolean timeline);

ddCXform
ddDrawable_getCXform(ddDrawable* d);

static inline int ddDrawable_getDepth(ddDrawable* d)
{
	return d->depth;
}

static inline void ddDrawable_setNeedsUpdate(ddDrawable* d)
{
	d->flags |= DRAWABLE_NEEDSUPDATE;
}

static inline void ddDrawable_setMark(ddDrawable* d)
{
	d->flags |= DRAWABLE_MARK;
}
static inline void ddDrawable_clearMark(ddDrawable* d)
{
	d->flags &= ~DRAWABLE_MARK;
}
static inline int ddDrawable_getMark(ddDrawable* d)
{
	return (d->flags & DRAWABLE_MARK);
}

#endif /* DD_PLAYER_DRAWABLE_H_INCLUDED */
