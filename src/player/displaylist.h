/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_DISPLAYLIST_H_INCLUDED
#define DD_PLAYER_DISPLAYLIST_H_INCLUDED

#include "../dd.h"

typedef struct _ddDisplayList ddDisplayList;

#include "player.h"

#include "../render/matrix.h"
#include "../render/cxform.h"
#include "../render/updatelist.h"
#include "drawable.h"
#include "character.h"

struct _ddDisplayList
{
	// union of drawables' bounds for fast exclusion of drawing, hittest
	ddRect bounds;
	
	ddDrawable* drawablesHead;
	ddDrawable* drawablesTail;
	
	// keep a list of removed drawables so we can include them in the update list
	ddDrawable* removed;
};


ddDisplayList*
dd_newDisplayList();

void
dd_destroyDisplayList(ddDisplayList* list);

void
ddDisplayList_getUpdateList(ddDisplayList* list, ddUpdateList* update, ddMatrix matrix);

void
ddDisplayList_drawInImage(ddDisplayList* list,
						  ddImage* image, ddUpdateList* update,
						  ddMatrix matrix, ddCXform cXform, ddRect clipRect);

ddDrawable*
ddDisplayList_addCharacter(ddPlayer* player, ddDisplayList* list,
						   ddCharacter* c, int level, int maskLevel);

void
ddDisplayList_addDrawable(ddPlayer* player, ddDisplayList* list,
						  ddDrawable* d, int level, int maskLevel);

ddDrawable*
ddDisplayList_drawableAtLevel(ddDisplayList* list, int level);

void
ddDisplayList_removeDrawable(ddPlayer* player, ddDisplayList* list, ddDrawable* drawable);

void
ddDisplayList_removeDrawableAtLevel(ddPlayer* player, ddDisplayList* list, int level);

void
ddDisplayList_clearRemoved(ddPlayer* player, ddDisplayList* list);

void
ddDisplayList_setMaskDrawable(ddDisplayList* list, ddDrawable* mask);

int
ddDisplayList_hitTest(ddDisplayList* list, fixed x, fixed y);

void
ddDisplayList_swapDepths(ddDisplayList* list, int depth1, int depth2);

void
ddDisplayList_clearUnmarked(ddPlayer* player, ddDisplayList* list);

void
ddDisplayList_unlink(ddPlayer* player, ddDisplayList* list, ddUpdateList* update);

#endif /* DD_PLAYER_DISPLAYLIST_H_INCLUDED */
