/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <stdio.h>
#include "drawbutton.h"


//static ddPlayer* player = NULL;

extern void
ddPlayer_checkButton(ddPlayer* player, ddDrawButton* button);


static void
dd_destroyDrawButton(ddDrawable* d)
{
	ddDrawButton* b = (ddDrawButton*)d;
	//ddPlayer_checkButton(player, b);
	//dd_warn("Destroying button: 0x%x", b);
	dd_destroyDisplayList(b->displayList);
	dd_destroyDisplayList(b->hitList);
	dd_free(d);
}


static void
ddDrawButton_drawInImage(ddDrawable* d, ddImage* image, ddUpdateList* list,
						 ddMatrix matrix, ddCXform cXform, ddRect clipRect)
{
	ddDrawButton* b = (ddDrawButton*)d;

	if ( b->displayList != NULL )
	{
		ddMatrix m = ddMatrix_multiply(matrix, d->matrix);
		ddCXform c = ddCXform_compose(cXform, d->cXform);

		ddDisplayList_drawInImage(b->displayList, image, list, m, c, clipRect);
	}
}


static void
ddDrawButton_getUpdateList(ddDrawable* d, ddUpdateList* list, ddMatrix matrix)
{
	ddMatrix m = ddMatrix_multiply(matrix, d->matrix);
	
	ddDrawButton* b = (ddDrawButton*)d;

	if ( b->displayList != NULL )
		ddDisplayList_getUpdateList(b->displayList, list, m);

	d->bounds = b->displayList->bounds;

	// XXX - is this neccessary?
	if ( b->hitList != NULL )
		ddDisplayList_getUpdateList(b->hitList, NULL, m);
}


ddDrawButton*
dd_newDrawButton(ddPlayer* player, ddButton* button)
{
	ddDrawButton* b = dd_malloc(sizeof(ddDrawButton));
	ddTimelineItem* item;

	ddDrawable_init((ddDrawable*)b);

	b->parent.type = BUTTON_CHAR;
	b->parent.drawInImage = ddDrawButton_drawInImage;
	b->parent.getUpdateList = ddDrawButton_getUpdateList;
	b->parent.destroy = dd_destroyDrawButton;

	b->displayList = dd_newDisplayList();
	b->hitList = dd_newDisplayList();
	b->buttonDown = DD_FALSE;
	b->frame = DDBUTTON_UPFRAME;
	b->parentClip = NULL;

	item = button->timeline->frames[DDBUTTON_HITFRAME].items;

	while ( item != NULL )
	{
		if ( item->character != NULL )
		{
			ddDrawable* d = ddDisplayList_addCharacter(NULL, b->hitList,
											 item->character, item->level, item->maskdepth);

			ddDrawable_setMatrix(d, item->matrix);
		}

		item = item->next;
	}

	b->parent.bounds = b->displayList->bounds;
	b->button = button;

	//b->listButton = dd_newButtonListEntry(b);

	ddTimeline_updateButtonDisplay(player, button->timeline, b, DDBUTTON_UPFRAME);

	return b;
}


void
ddDrawButton_addChildClipToParent(ddDrawButton* b, ddDrawClip* drawclip, int level)
{
	ddActionMovieClip* clip = drawclip->actionClip;
	char* name = ddActionMovieClip_getName(clip);

	if ( name == NULL )
	{
		name = dd_malloc(16); /* "instancexxxxx" */
		sprintf(name, "instance%i", level);
		ddActionMovieClip_setName(clip, name);
	}

	drawclip->parentClip = b->parentClip;
	ddActionMovieClip_addChild(b->parentClip->actionClip, clip);
}


ddDrawable*
ddDrawButton_addDisplayItem(ddPlayer* player, ddDrawButton* b, ddCharacter* c, int level, int maskdepth)
{
	ddDrawable* d;

	if ( c == NULL )
		return NULL;

	d = ddDisplayList_addCharacter(player, b->displayList, c, level, maskdepth);

	if ( b->parentClip != NULL && d != NULL && c->type == MOVIECLIP_CHAR )
		ddDrawButton_addChildClipToParent(b, (ddDrawClip*)d, level);
	
	return d;
}


void
ddDrawButton_setParentClip(ddDrawButton* b, ddDrawClip* clip)
{
	ddDrawable* d = b->displayList->drawablesHead;
	
	// add button movieclip children to parent

	// XXX - in SWF6 (i think), buttons are script objects, and childrens' _parent point to it,
	// not the containing clip
	
	b->parentClip = clip;

	while ( d != NULL )
	{
		if ( d->character->type == MOVIECLIP_CHAR )
			ddDrawButton_addChildClipToParent(b, (ddDrawClip*)d, d->depth);
		
		d = d->next;
	}
}


void
ddDrawButton_unlink(ddPlayer* player, ddDrawButton* b, ddUpdateList* list)
{
	dd_log("%p unlinking", b);

	ddPlayer_removeActiveButton(player, b);

	ddDisplayList_unlink(player, b->displayList, list);
}


void
ddDrawButton_gotoUpState(ddPlayer* player, ddDrawButton* b)
{
	if ( b->frame != DDBUTTON_UPFRAME )
	{
		b->frame = DDBUTTON_UPFRAME;
		ddTimeline_updateButtonDisplay(player, b->button->timeline, b, DDBUTTON_UPFRAME);
	}
}


void
ddDrawButton_gotoOverState(ddPlayer* player, ddDrawButton* b)
{
	if ( b->frame != DDBUTTON_OVERFRAME )
	{
		b->frame = DDBUTTON_OVERFRAME;
		ddTimeline_updateButtonDisplay(player, b->button->timeline, b, DDBUTTON_OVERFRAME);
	}
}


void
ddDrawButton_gotoDownState(ddPlayer* player, ddDrawButton* b)
{
	if ( b->frame != DDBUTTON_DOWNFRAME )
	{
		b->frame = DDBUTTON_DOWNFRAME;
		ddTimeline_updateButtonDisplay(player, b->button->timeline, b, DDBUTTON_DOWNFRAME);
	}
}


int
ddDrawButton_hitTest(ddDrawButton* b, fixed x, fixed y)
{
	return ddDisplayList_hitTest(b->hitList, x, y);
}


static void
ddDrawButton_executeActions(ddPlayer* player, ddDrawButton* b, int flags)
{
	int i;
	int nActions = b->button->nActions;

	for ( i = 0; i < nActions; ++i )
	{
		if ( (flags & b->button->actions[i].flags) != 0 )
		{
			ddAction_execute(player, b->button->actions[i].action,
					(ddActionObject*)b->parentClip->actionClip);
		}
	}
}


void
ddDrawButton_outUpToOverUp(ddPlayer* player, ddDrawButton* b)
{
	ddDrawButton_gotoOverState(player, b);
	ddDrawButton_executeActions(player, b, DDBUTTON_IDLETOOVERUP);
}


void
ddDrawButton_overDownToOverUp(ddPlayer* player, ddDrawButton* b)
{
	ddDrawButton_gotoOverState(player, b);
	ddDrawButton_executeActions(player, b, DDBUTTON_OVERDOWNTOOVERUP);
}


void
ddDrawButton_outDownToOutUp(ddPlayer* player, ddDrawButton* b)
{
	ddDrawButton_gotoUpState(player, b);
	ddDrawButton_executeActions(player, b, DDBUTTON_OUTDOWNTOIDLE);
}


void
ddDrawButton_overUpToOverDown(ddPlayer* player, ddDrawButton* b)
{
	ddDrawButton_gotoDownState(player, b);
	ddDrawButton_executeActions(player, b, DDBUTTON_OVERUPTOOVERDOWN);
}


void
ddDrawButton_overUpToOutUp(ddPlayer* player, ddDrawButton* b)
{
	ddDrawButton_gotoUpState(player, b);
	ddDrawButton_executeActions(player, b, DDBUTTON_OVERUPTOIDLE);
}


void
ddDrawButton_overDownToOutDown(ddPlayer* player, ddDrawButton* b)
{
	ddDrawButton_gotoOverState(player, b);
	ddDrawButton_executeActions(player, b, DDBUTTON_OVERDOWNTOOUTDOWN);
}


void
ddDrawButton_outDownToOverDown(ddPlayer* player, ddDrawButton* b)
{
	ddDrawButton_gotoDownState(player, b);
	ddDrawButton_executeActions(player, b, DDBUTTON_OUTDOWNTOOVERDOWN);
}

#ifdef DD_INCLUDE_DEBUGGER

void
ddDrawButton_getDisplayList(ddDrawButton* button, ddDrawable*** outItems, int* outNItems)
{
	ddDrawable* d = button->displayList->drawablesHead;
	ddDrawable** list = NULL;
	int count = 0;
	
	while ( d != NULL )
	{
		list = dd_realloc(list, (count + 1) * sizeof(ddDrawable*));
		list[count] = d;
		++count;
		d = d->next;
	}
	
	*outItems = list;
	*outNItems = count;
}

/*
void
ddDrawButton_getActions()
{
}
*/

#endif
