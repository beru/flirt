/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "displaylist.h"
#include "drawclip.h"

#include "../action/classes/movieclip_class.h"

ddDisplayList*
dd_newDisplayList()
{
	ddDisplayList* list = dd_malloc(sizeof(struct _ddDisplayList));

	list->drawablesHead = NULL;
	list->drawablesTail = NULL;
	list->bounds = ddInvalidRect;
	list->removed = NULL;
	
	return list;
}


void
dd_destroyDisplayList(ddDisplayList* list)
{
	ddDrawable* d = list->drawablesHead;

	while ( d != NULL )
	{
		ddDrawable* next = d->next;

		dd_destroyDrawable(d);
		d = next;
	}

	dd_free(list);
}


ddDrawable*
ddDisplayList_addCharacter(ddPlayer* player, ddDisplayList* list, ddCharacter* c, int level, int maskLevel)
{
	ddDrawable* d = ddCharacter_instantiate(player, c);

	if ( d == NULL )
		return NULL;

	d->character = c;

	ddDisplayList_addDrawable(player, list, d, level, maskLevel);
	return d;
}


void
ddDisplayList_addDrawable(ddPlayer* player, ddDisplayList* list, ddDrawable* d, int level, int maskLevel)
{
	if ( list->drawablesHead == NULL )
	{
		list->drawablesHead = d;
		list->drawablesTail = d;
	}
	else if ( level > list->drawablesTail->depth )
	{
		ddDrawable_linkDrawableAfter(list->drawablesTail, d);
		list->drawablesTail = d;
	}
	else if ( level == list->drawablesTail->depth )
	{
		ddDisplayList_removeDrawable(player, list, list->drawablesTail);
		
		if ( list->drawablesHead == NULL )
			list->drawablesHead = d;
		else
			ddDrawable_linkDrawableAfter(list->drawablesTail, d);
		
		list->drawablesTail = d;
	}
	else if ( level < list->drawablesHead->depth )
	{
		ddDrawable_linkDrawableBefore(d, list->drawablesHead);
		list->drawablesHead = d;
	}
	else if ( level == list->drawablesHead->depth )
	{
		ddDisplayList_removeDrawable(player, list, list->drawablesHead);
		ddDrawable_linkDrawableBefore(d, list->drawablesHead);
		
		list->drawablesHead = d;
	}
	else
	{
		// take a guess which side will be faster
		int start = list->drawablesHead->depth;
		int end = list->drawablesTail->depth;
		
		if ( level - start < start - end )
		{
			ddDrawable* p = list->drawablesHead;
		
			while ( p->next != NULL && p->depth < level )
				p = p->next;

			if ( p->depth == level )
			{
				ddDrawable_replace(p, d);
				ddDisplayList_removeDrawable(player, list, p);
			}
			else
				ddDrawable_linkDrawableBefore(p, d);
		}
		else
		{
			ddDrawable* p = list->drawablesTail;
			
			while ( p->last != NULL && p->depth > level )
				p = p->last;
			
			if ( p->depth == level )
			{
				ddDrawable_replace(p, d);
				ddDisplayList_removeDrawable(player, list, p);
			}
			else
				ddDrawable_linkDrawableAfter(p, d);
		}
	}
	
	d->depth = level;
	d->maskLevel = maskLevel;

	if ( maskLevel == -1 )
	{
		// if level is masked, set mask on new drawable
		
		ddDrawable* p = d;
		
		while ( p != NULL )
		{
			if ( p->maskLevel > level )
			{
				ddDrawable_setMaskDrawable(d, p);
				break;
			}
			
			p = p->last;
		}
	}
	else
	{
		// set mask on already-placed objects

		ddDrawable* p = d->next;
		
		while ( p != NULL && p->depth < maskLevel )
		{
			ddDrawable_setMaskDrawable(p, d);
			p = p->next;
		}
	}
}


void
ddDisplayList_setMaskDrawable(ddDisplayList* list, ddDrawable* mask)
{
	ddDrawable* d = list->drawablesHead;
	
	while ( d != NULL )
	{
		ddDrawable_setMaskDrawable(d, mask);
		d = d->next;
	}
}


void
ddDisplayList_clearRemoved(ddPlayer* player, ddDisplayList* list)
{
	// add all of last frame's removed clips to update area

}


void
ddDisplayList_getUpdateList(ddDisplayList* list, ddUpdateList* update, ddMatrix matrix)
{
	ddDrawable* d;
	int maskLevel = -1;
	ddRect maskBounds;
	
	list->bounds = ddInvalidRect;

	d = list->removed;

	while ( d != NULL )
	{
		ddDrawable* next = d->next;

#ifdef DD_LOG
		dd_log("finishing removing drawable %p from display list %p", d, list);
#endif

		ddUpdateList_includeRect(update, d->bounds);
		dd_destroyDrawable(d);
		
		d = next;
	}

	list->removed = NULL;
	
	d = list->drawablesHead;
	
	while ( d != NULL )
	{
		ddDrawable_getUpdateList(d, update, matrix);

		// XXX - should shapes deal with mask bounds themselves?
			
		if ( d->maskLevel != -1 )
		{
			maskLevel = d->maskLevel;
			maskBounds = d->bounds;
		}

		if ( maskLevel != -1 && d->depth >= maskLevel )
			maskLevel = -1;
			
		if ( maskLevel != -1 )
		{
			ddRect rect = ddRect_intersectRect(d->bounds, maskBounds);
			list->bounds = ddRect_containRect(list->bounds, rect);
		}
		else
			list->bounds = ddRect_containRect(list->bounds, d->bounds);

		d = d->next;
	}
}


void
ddDisplayList_drawInImage(ddDisplayList* list, ddImage* image, ddUpdateList* update,
						  ddMatrix matrix, ddCXform cXform, ddRect clipRect)
{
#ifdef DD_RENDER_TOPDOWN
	ddDrawable* d = list->drawablesTail;
#else
	ddDrawable* d = list->drawablesHead;
#endif
	
	while ( d != NULL )
	{
		if ( d->maskLevel == -1 && ddRect_intersectsRect(clipRect, d->bounds) )
			ddDrawable_drawInImage(d, image, update, matrix, cXform, clipRect);

#ifdef DD_RENDER_TOPDOWN
		d = d->last;
#else
		d = d->next;
#endif
	}
}


int
ddDisplayList_hitTest(ddDisplayList* list, fixed x, fixed y)
{
	ddDrawable* d = list->drawablesHead;
	
	while ( d != NULL )
	{
		if ( ddDrawable_hitTest(d, x, y) )
			return DD_TRUE;
		
		d = d->next;
	}

	return DD_FALSE;
}


void
ddDisplayList_swapDepths(ddDisplayList* list, int depth1, int depth2)
{
	// XXX - what about masks?
	
	ddDrawable* d1 = ddDisplayList_drawableAtLevel(list, depth1);
	ddDrawable* d2 = ddDisplayList_drawableAtLevel(list, depth2);

	ddDrawable* next;
	ddDrawable* last;
	
	if ( d1 == NULL || d2 == NULL )
		return;
	
	next = d1->next;
	last = d1->last;
	
	ddDrawable_replace(d2, d1);
	
	d2->next = next;
	d2->last = last;
	
	if ( next != NULL )
		next->last = d2;
	
	if ( last != NULL )
		last->next = d2;
	
	ddDrawable_setNeedsUpdate(d1);
	ddDrawable_setNeedsUpdate(d2);
}


void
ddDisplayList_unlink(ddPlayer* player, ddDisplayList* list, ddUpdateList* update)
{
	ddDrawable* d = list->drawablesHead;

	while ( d != NULL )
	{
		ddDrawable_unlink(player, d, update);
		d = d->next;
	}
}


ddDrawable*
ddDisplayList_drawableAtLevel(ddDisplayList* list, int level)
{
	int start;
	int end;
	ddDrawable* p;
	
	if ( list->drawablesHead == NULL )
		return NULL;
	
	start = list->drawablesHead->depth;
	end = list->drawablesTail->depth;
	
	// take a guess which side will be faster
	// XXX - I'm not sure what I was doing here, but it sure doesn't look right

	if ( level - start < start - end )
	{
		p = list->drawablesHead;
		
		while ( p != NULL && p->depth < level )
			p = p->next;
	}
	else
	{
		p = list->drawablesTail;
		
		while ( p != NULL && p->depth > level )
			p = p->last;
	}

	if ( p == NULL || p->depth != level )
		return NULL;
	else
		return p;
}


void
ddDisplayList_removeDrawableAtLevel(ddPlayer* player, ddDisplayList* list, int level)
{
	ddDisplayList_removeDrawable(player, list, ddDisplayList_drawableAtLevel(list, level));
}


void
ddDisplayList_removeDrawable(ddPlayer* player, ddDisplayList* list, ddDrawable* d)
{
#ifdef DD_LOG
	dd_log("removing drawable %p at level %i from display list %p", d, d->depth, list);
#endif
	
	if ( d == list->drawablesHead )
		list->drawablesHead = d->next;
	
	if ( d == list->drawablesTail )
		list->drawablesTail = d->last;
	
	if ( d->maskLevel != -1 )
	{
		ddDrawable* p = d->next;
		
		while ( p != NULL && p->depth < d->maskLevel )
		{
			ddDrawable_setMaskDrawable(p, NULL);
			p = p->next;
		}
	}
	
	if ( d->next != NULL )
		d->next->last = d->last;
	
	if ( d->last != NULL )
		d->last->next = d->next;
	
	d->next = NULL;
	d->last = NULL;

	// keep removed drawables so we can mark their bounds as needing update-
	// add drawable to list of removed drawables (to be processed next update)
	// (but movie clips are handled by the player..)

	if ( d->type == MOVIECLIP_CHAR )
	{
		ddDrawClip* clip = (ddDrawClip*)d;
		clip->flags |= DRAWCLIP_UNLOAD;
		ddPlayer_addUnloadClip(player, clip);
	}
	else
	{
		if ( d->type == BUTTON_CHAR )
			ddPlayer_removeActiveButton(player, (ddDrawButton*)d);
		
		if ( list->removed != NULL )
			d->next = list->removed;

		list->removed = d;
	}
}


void
ddDisplayList_clearUnmarked(ddPlayer* player, ddDisplayList* list)
{
	ddDrawable* d = list->drawablesHead;

	while ( d != NULL )
	{
		ddDrawable* next = d->next;

		if ( !ddDrawable_getMark(d) )
			ddDisplayList_removeDrawable(player, list, d);
		else
			ddDrawable_clearMark(d);
		
		d = next;
	}
}
