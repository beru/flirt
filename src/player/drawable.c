/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "drawable.h"
#include "drawbutton.h"
#include "drawclip.h"
#include "drawmorph.h"
#include "drawshape.h"
#include "drawtext.h"

void
ddDrawable_linkDrawableAfter(ddDrawable* left, ddDrawable* right)
{
	right->last = left;
	right->next = left->next;

	if ( left->next != NULL )
		left->next->last = right;
	
	left->next = right;
}


void
ddDrawable_linkDrawableBefore(ddDrawable* left, ddDrawable* right)
{
	left->next = right;
	left->last = right->last;

	if ( right->last != NULL )
		right->last->next = left;
	
	right->last = left;
}


void
ddDrawable_replace(ddDrawable* old, ddDrawable* newitem)
{
	newitem->next = old->next;
	newitem->last = old->last;

	if ( newitem->last != NULL )
		newitem->last->next = newitem;
	
	if ( newitem->next != NULL )
		newitem->next->last = newitem;
}


void
ddDrawable_setMatrix(ddDrawable* d, ddMatrix matrix)
{
	if ( d->type == MOVIECLIP_CHAR && !ddDrawClip_isFollowing((ddDrawClip*)d) )
		return;

	d->matrix = matrix;
	
	if ( d->type == MOVIECLIP_CHAR )
		ddDrawClip_recalcMatrices((ddDrawClip*)d);
}


ddMatrix
ddDrawable_getMatrix(ddDrawable* d)
{
	return d->matrix;
}


void
ddDrawable_setCXform(ddDrawable* d, ddCXform cXform, boolean timeline)
{
	if ( d->type == MOVIECLIP_CHAR && timeline && !ddDrawClip_isFollowing((ddDrawClip*)d) )
		return;

	if ( !ddCXform_equals(d->cXform, cXform) )
	{
		d->flags |= DRAWABLE_NEEDSUPDATE;
		d->cXform = cXform;
	}
}


ddCXform
ddDrawable_getCXform(ddDrawable* d)
{
	return d->cXform;
}


/* why a dispatcher instead of a virtual function? Dunno. Maybe faster? */

int
ddDrawable_hitTest(ddDrawable* d, fixed x, fixed y)
{
	switch ( d->type )
	{
		case SHAPE_CHAR:
			return ddDrawShape_hitTest((ddDrawShape*)d, x, y);
		case BUTTON_CHAR:
			return ddDrawButton_hitTest((ddDrawButton*)d, x, y);
		case MOVIECLIP_CHAR:
			return ddDrawClip_hitTest((ddDrawClip*)d, x, y);
		case TEXT_CHAR:
			return ddDrawText_hitTest((ddDrawText*)d, x, y);

			/* bitmaps are only used w/in shapes */
			/* XXX - textfields are prolly just checked against the bounds */

		default:
			return 0;
	}
}


void
ddDrawable_setMaskDrawable(ddDrawable* d, ddDrawable* mask)
{
	// XXX - text can be used as a mask. Morphs? Probably. Movie clips in SWF 6, too.
	// XXX - mark shape as needing redrawing

	if ( mask == NULL )
	{
		if ( d->type == SHAPE_CHAR )
			ddShapeInstance_setMaskShape(((ddDrawShape*)d)->shape, NULL);
		else if ( d->type == MOVIECLIP_CHAR )
			ddDrawClip_setMaskShape((ddDrawClip*)d, NULL);

		return;
	}
	
	if ( mask->type != SHAPE_CHAR )
	{
		dd_warn("Unsupported mask type: %i", mask->type);
		return;
	}

	if ( d->type == SHAPE_CHAR )
		ddShapeInstance_setMaskShape(((ddDrawShape*)d)->shape, ((ddDrawShape*)mask)->shape);

	else if ( d->type == MOVIECLIP_CHAR )
		ddDrawClip_setMaskShape((ddDrawClip*)d, mask);

	else if ( d->type == MORPH_CHAR )
		; // XXX - taken care of in setRatio.  Good enough?

	else
	{
		dd_warn("Unsupported mask target: %i", d->type);
		return;
	}
}


void
ddDrawable_unlink(ddPlayer* player, ddDrawable* d, ddUpdateList* list)
{
	if ( d->type == MOVIECLIP_CHAR )
		ddDrawClip_unlink(player, (ddDrawClip*)d, list);
	else
	{
		ddUpdateList_includeRect(list, d->bounds);
		
		if ( d->type == BUTTON_CHAR )
		{
			ddDrawButton_unlink(player, (ddDrawButton*)d, list);
			ddPlayer_removeActiveButton(player, (ddDrawButton*)d);
		}
	}
}


#ifdef DD_INCLUDE_DEBUGGER

ddCharacterType
ddDrawable_getType(ddDrawable* d)
{
	return d->type;
}

ddRect
ddDrawable_getBounds(ddDrawable* d)
{
	return d->bounds;
}

#endif
