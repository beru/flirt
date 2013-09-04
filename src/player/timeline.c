/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>

#include "timeline.h"
#include "drawmorph.h"

#define TIMELINE_INCREMENT 16

ddTimeline*
dd_newTimeline()
{
	ddTimeline* t = dd_malloc(sizeof(ddTimeline));

	t->nFrames = 0;
	t->frames = NULL;

	ddTimeline_nextFrame(t);

	return t;
}


ddTimeline*
dd_newButtonTimeline()
{
	/* button timelines only have four frames and no actions */
	ddTimeline* t = dd_newTimeline();

	ddTimeline_nextFrame(t);
	ddTimeline_nextFrame(t);
	ddTimeline_nextFrame(t);
	ddTimeline_nextFrame(t);

	return t;
}


static void
dd_destroyTimelineItem(ddTimelineItem* item)
{
#ifdef DD_DEBUG
	printf("%p -> destroyTimelineItem\n", item);
#endif

	if ( item->name != NULL )
		dd_free(item->name);

	// XXX - clip actions are leaked

	dd_free(item);
}


void
dd_destroyTimelineFrame(ddTimelineFrame* frame)
{
	ddTimelineItem* item = frame->items;
	ddTimelineItem* next;

	while ( item != NULL )
	{
		next = item->next;
		dd_destroyTimelineItem(item);
		item = next;
	}

	if ( frame->actions != NULL )
		dd_destroyAction(frame->actions);
}


void
dd_destroyTimeline(ddTimeline* t)
{
	int i;

	for ( i = 0; i < t->nFrames; ++i )
		dd_destroyTimelineFrame(&t->frames[i]);

	if ( t->frames != NULL )
		dd_free(t->frames);

	dd_free(t);
}


ddTimelineItem*
ddTimeline_findItem(ddTimeline* timeline, int depth)
{
	ddTimelineItem* item = timeline->frames[timeline->nFrames-1].items;

	while ( item != NULL )
	{
		if ( item->level == depth )
			return item;

		item = item->next;
	}

	return NULL;
}


ddTimelineItem*
dd_newTimelineItem()
{
	int i;
	ddTimelineItem* newitem = dd_malloc(sizeof(ddTimelineItem));

#ifdef DD_DEBUG
	printf("%p -> newTimelineItem\n", newitem);
#endif

	newitem->next = NULL;
	newitem->level = -1;
	newitem->ratio = -1;
	newitem->maskdepth = -1;
	newitem->name = NULL;
	newitem->character = NULL;
	newitem->matrix = ddIdentityMatrix;
	newitem->cXform = ddEmptyCXform;

	for ( i = 0; i < DD_NUMEVENTHANDLERS; ++i )
		newitem->actions[i] = NULL;

	return newitem;
}


ddTimelineItem*
ddTimeline_addItem(ddTimeline* timeline, int depth, int frame)
{
	ddTimelineItem* newitem = dd_newTimelineItem();

	ddTimelineItem* last = NULL;
	ddTimelineItem* item = timeline->frames[frame].items;

	while ( item != NULL && item->level < depth )
	{
		last = item;
		item = item->next;
	}

	if ( last == NULL )
		timeline->frames[frame].items = newitem;
	else
		last->next = newitem;

	newitem->next = item;
	newitem->level = depth;

	return newitem;
}


void
ddTimeline_removeItem(ddTimeline* timeline, int depth)
{
	ddTimelineItem* last = NULL;
	ddTimelineItem* item = timeline->frames[timeline->nFrames-1].items;

	while ( item != NULL )
	{
		if ( item->level == depth )
		{
			if ( last == NULL )
				timeline->frames[timeline->nFrames-1].items = item->next;
			else
				last->next = item->next;

			dd_destroyTimelineItem(item);

			break;
		}

		last = item;
		item = item->next;
	}
}


void
ddTimeline_placeCharacter(ddTimeline* timeline, ddCharacter* c, int depth,
						  ddMatrix matrix, ddCXform cXform, int ratio,
						  int maskdepth, char* name)
{
	ddTimelineItem* item = ddTimeline_findItem(timeline, depth);

	if ( item == NULL )
		item = ddTimeline_addItem(timeline, depth, timeline->nFrames - 1);

	if ( c != NULL )
		item->character = c;

	if ( matrix.b < -1000000)
		dd_log("bad matrix!");
	
	item->matrix = matrix;
	item->cXform = cXform;

	if ( ratio != -1 )
		item->ratio = ratio;

	if ( maskdepth != -1 )
		item->maskdepth = maskdepth;
}


void
ddTimeline_placeButtonCharacter(ddTimeline* timeline, ddCharacter* c,
								int frame, int depth, ddMatrix matrix,
								ddCXform cXform)
{
	ddTimelineItem* item = ddTimeline_addItem(timeline, depth, frame);

	item->character = c;
	item->matrix = matrix;
	item->cXform = cXform;
}


void
ddTimeline_removeCharacter(ddTimeline* timeline, ddCharacter* c, int depth)
{
	ddTimelineItem* last = NULL;
	ddTimelineItem* item = timeline->frames[timeline->nFrames-1].items;

	while ( item != NULL )
	{
		if ( item->level == depth && item->character == c )
		{
			if ( last == NULL )
				timeline->frames[timeline->nFrames-1].items = item->next;
			else
				last->next = item->next;

			item->next = NULL;

			dd_destroyTimelineItem(item);

			break;
		}

		last = item;
		item = item->next;
	}
}

#ifdef DD_INCLUDE_DEBUGGER
static int count = 0;
#endif

static ddTimelineItem*
ddTimelineItem_duplicate(ddTimelineItem* item)
{
	int i;
	ddTimelineItem* newitem = dd_malloc(sizeof(struct _ddTimelineItem));

#ifdef DD_DEBUG
	printf("%p -> duplicateTimelineItem (level %i)\n", newitem, item->level);
#endif

#ifdef DD_INCLUDE_DEBUGGER
	newitem->count = ++count;
#endif

	newitem->level = item->level;
	newitem->ratio = item->ratio;
	newitem->maskdepth = item->maskdepth;
	newitem->character = item->character;
	newitem->matrix = item->matrix;
	newitem->cXform = item->cXform;
	
	/* XXX- could avoid this copying with reference counting.. */

	if ( item->name != NULL )
		newitem->name = strdup(item->name);
	else
		newitem->name = NULL;

	for ( i = 0; i < DD_NUMEVENTHANDLERS; ++i )
		newitem->actions[i] = item->actions[i];

	if ( item->next != NULL )
		newitem->next = ddTimelineItem_duplicate(item->next);
	else
		newitem->next = NULL;

	return newitem;
}


void
ddTimeline_nextFrame(ddTimeline* timeline)
{
	int nFrames = timeline->nFrames;
	
	timeline->frames = dd_realloc(timeline->frames, (nFrames + 1) * sizeof(ddTimelineFrame));

	if ( nFrames > 0 && timeline->frames[nFrames-1].items != NULL )
	{
		timeline->frames[nFrames].items =
			ddTimelineItem_duplicate(timeline->frames[nFrames-1].items);
	}
	else
		timeline->frames[nFrames].items = NULL;

	timeline->frames[nFrames].actions = NULL;
	timeline->frames[nFrames].actiontails = NULL;
	timeline->frames[nFrames].streamblock = NULL;
	timeline->frames[nFrames].sounds = NULL;
	
	++timeline->nFrames;
}


void
ddTimelineItem_registerEventHandlers(ddPlayer* player, ddTimelineItem* item, ddDrawClip* clip)
{
	ddAction* action;

	if ( (action = item->actions[DDEVENTHANDLER_ONLOAD]) != NULL )
		ddDrawClip_setOnLoadAction(clip, action);

	if ( (action = item->actions[DDEVENTHANDLER_ENTERFRAME]) != NULL )
		ddPlayer_registerEnterFrameListener(player, clip, action);

	if ( (action = item->actions[DDEVENTHANDLER_MOUSEMOVE]) != NULL )
		ddPlayer_registerMouseMoveListener(player, clip, action);

	if ( (action = item->actions[DDEVENTHANDLER_MOUSEDOWN]) != NULL )
		ddPlayer_registerMouseDownListener(player, clip, action);

	if ( (action = item->actions[DDEVENTHANDLER_MOUSEUP]) != NULL )
		ddPlayer_registerMouseUpListener(player, clip, action);

	if ( (action = item->actions[DDEVENTHANDLER_KEYDOWN]) != NULL )
		ddPlayer_registerKeyDownListener(player, clip, action);

	if ( (action = item->actions[DDEVENTHANDLER_KEYUP]) != NULL )
		ddPlayer_registerKeyUpListener(player, clip, action);
}


void
ddTimeline_updateClipDisplay(ddPlayer* player, ddTimeline* timeline, ddDrawClip* clip)
{
	ddTimelineItem* item;
	ddDrawable* drawable;
	ddDisplayList* displayList = clip->displayList;
	int frameNum = clip->currentFrame;

	if ( frameNum >= timeline->nFrames )
	{
		dd_warn("frameNum > timeline->nFrames");
		return;
	}

	item = timeline->frames[frameNum].items;

	while ( item != NULL )
	{
		boolean isnew = DD_FALSE;
		
		// XXX - if items are ordered by depth, we could avoid a lot of searching:
		drawable = ddDisplayList_drawableAtLevel(displayList, item->level);

		if ( drawable != NULL && drawable->character != item->character )
		{
			ddDisplayList_removeDrawableAtLevel(player, displayList, item->level);
			drawable = NULL;
		}

		if ( drawable == NULL && item->character != NULL )
		{
			drawable = ddDrawClip_addDisplayItem(player, clip, item->character,
										item->level, item->name, item->maskdepth);
			
			isnew = DD_TRUE;
		}

		if ( drawable != NULL )
		{
			ddDrawable_setMatrix(drawable, item->matrix);
			ddDrawable_setCXform(drawable, item->cXform, DD_TRUE);

			if ( drawable->type == MORPH_CHAR && item->ratio != -1 )
				ddDrawMorph_setRatio((ddDrawMorph*)drawable, item->ratio);

			ddDrawable_setMark(drawable);

			if ( isnew && item->character->type == MOVIECLIP_CHAR )
			{
				ddDrawClip* clip = (ddDrawClip*)drawable;
				
				ddTimelineItem_registerEventHandlers(player, item, clip);
				ddDrawClip_step(player, clip);
				
				if ( clip->clip->framesTotal == 1 )
					clip->flags |= DRAWCLIP_STOPPED;
			}
		}
		
		item = item->next;
	}

	ddDisplayList_clearUnmarked(player, displayList);
}


/* it'd be nice and easy to just wipe the current display list and throw in
the new state's chars, but it doesn't work that way.  If a clip is in, say,
both the up and over states, its onLoad handler shouldn't be called in the
transition. */

void
ddTimeline_updateButtonDisplay(ddPlayer* player, ddTimeline* timeline, ddDrawButton* button, int frame)
{
	ddTimelineItem* item;
	ddDrawable* drawable;
	ddDisplayList* displayList = button->displayList;

	if ( frame > 3 )
		return; /* XXX - warn? */

	item = timeline->frames[frame].items;

	while ( item != NULL )
	{
		// XXX - if items are ordered by depth, we could avoid a lot of searching:
		drawable = ddDisplayList_drawableAtLevel(displayList, item->level);

		if ( item->character != NULL )
		{
			if ( drawable != NULL && drawable->character != item->character )
			{
				ddDisplayList_removeDrawableAtLevel(player, displayList, item->level);
				drawable = NULL;
			}

			if ( drawable == NULL )
			{
				drawable = ddDrawButton_addDisplayItem(player, button, item->character,
										   item->level, item->maskdepth);

				if ( item->character->type == MOVIECLIP_CHAR && drawable != NULL )
					ddTimelineItem_registerEventHandlers(player, item, (ddDrawClip*)drawable);
			}
		}

		if ( drawable != NULL )
		{
			ddDrawable_setMatrix(drawable, item->matrix);
			ddDrawable_setCXform(drawable, item->cXform, DD_TRUE);

			if ( drawable->type == MORPH_CHAR && item->ratio != -1 )
				ddDrawMorph_setRatio((ddDrawMorph*)drawable, item->ratio);

			ddDrawable_setMark(drawable);
		}

		item = item->next;
	}

	ddDisplayList_clearUnmarked(player, displayList);
}


void
ddTimeline_playFrameSounds(ddPlayer* player, ddTimeline* timeline, ddDrawClip* clip)
{
	ddTimelineFrame* frame;
	
	if ( clip->currentFrame > timeline->nFrames )
	{
		dd_warn("clip->currentFrame > timeline->nFrames\n");
		return;
	}
	
	frame = &timeline->frames[clip->currentFrame];
	
	if ( frame->streamblock != NULL )
		ddSoundMixer_playSoundStreamBlock(player->mixer, clip, frame->streamblock);
	
	ddSoundInstance* sound = frame->sounds;
	
	while ( sound != NULL )
	{
		if ( sound->stop )
			ddSoundMixer_stopSoundInstance(player->mixer, sound);
		else
		{
			if ( sound->sound == NULL )
			{
				ddCharacter* c = ddMovieClip_getCharacter(player->player, sound->characterid);
				
				if ( c == NULL )
					dd_warn("requested character doesn't exist");
				else if ( c->type != SOUND_CHAR )
					dd_warn("requested sound id isn't a sound character");
				else
					sound->sound = (ddSound*)c;
			}
			
			ddSoundMixer_playSoundInstance(player->mixer, clip, sound);
		}
		
		sound = sound->next;
	}
}


void
ddTimeline_addAction(ddTimeline* timeline, ddAction* action)
{
	ddTimelineFrame* frame = &timeline->frames[timeline->nFrames-1];

	if ( frame->actions == NULL )
	{
		frame->actions = action;
		frame->actiontails = action;
	}
	else
	{
		frame->actiontails->next = action;
		frame->actiontails = action;
	}
}


void
ddTimeline_scheduleActions(ddPlayer* player, ddTimeline* timeline, ddDrawClip* clip, int frameNum)
{
	if ( frameNum >= timeline->nFrames )
	{
		/* XXX */
		dd_warn("frameNum > timeline->nFrames");
		return;
	}

	if ( timeline->frames[frameNum].actions != NULL )
		ddPlayer_addFrameAction(player, timeline->frames[frameNum].actions, (ddActionObject*)clip->actionClip);
}


void
ddTimeline_addSoundInstance(ddTimeline* timeline, ddSoundInstance* sound)
{
	ddTimelineFrame* frame = &timeline->frames[timeline->nFrames-1];
	
	sound->next = frame->sounds;
	frame->sounds = sound;
}

void
ddTimeline_addSoundStreamBlock(ddTimeline* timeline, ddSoundStreamBlock* block)
{
	ddTimelineFrame* frame = &timeline->frames[timeline->nFrames-1];

	if ( frame->streamblock != NULL )
	{
		dd_warn("Timeline frame already has sound stream block");
		dd_destroySoundStreamBlock(frame->streamblock);
	}
	
	frame->streamblock = block;
}

#ifdef DD_INCLUDE_DEBUGGER

void
ddTimeline_getFrames(ddTimeline* timeline, ddTimelineFrame** outFrames, int* outNFrames)
{
	if ( outFrames != NULL )
		*outFrames = timeline->frames;

	if ( outNFrames != NULL )
		*outNFrames = timeline->nFrames;
}

ddTimelineItem*
ddTimelineFrame_getItems(ddTimelineFrame* frame)
{
	return frame->items;
}

ddAction*
ddTimelineFrame_getActions(ddTimelineFrame* frame)
{
	return frame->actions;
}

ddSoundStreamBlock*
ddTimelineFrame_getSoundStreamBlock(ddTimelineFrame* frame)
{
	return frame->streamblock;
}

ddTimelineItem*
ddTimelineItem_getNext(ddTimelineItem* item)
{
	return item->next;
}

#ifdef DD_INCLUDE_DEBUGGER

ddCharacter*
ddTimelineItem_getCharacter(ddTimelineItem* item)
{
	return item->character;
}

const char*
ddTimelineItem_getName(ddTimelineItem* item)
{
	return item->name;
}

#endif

#endif
