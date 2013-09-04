/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <stdio.h>

#include "player.h"

void
ddPlayer_cleanList(ddList* list);

void
ddPlayer_cleanActiveLists(ddPlayer* player);


void
dd_destroyPlayer(ddPlayer* player)
{
	ddListEntry* d;

	d = player->activeClips.head;

	while ( d != NULL )
	{
		ddListEntry* next = d->next;
		dd_free(d);
		d = next;
	}

	dd_destroyUpdateList(player->updateList);
	dd_destroyEventHandlerSet(&player->eventHandlers);
	dd_destroyActionEngine(player);
	dd_destroyReader(player->reader);
	dd_destroyDrawClip((ddDrawable*)player->playclip);
	dd_destroySoundMixer(player->mixer);
	
	dd_free(player);
}


void
ddPlayer_init(ddPlayer* player)
{
	ddList emptyList = { NULL, NULL };
	
	player->player = NULL;
	player->playclip = NULL;
	player->bounds = ddInvalidRect;
	player->matrix = ddEmptyMatrix;
	player->updateList = dd_newUpdateList();
	player->image = NULL;
	player->reader = NULL;
	player->width = 0;
	player->height = 0;
	player->frameRate = 0;
	player->backgroundColor = (ddColor)0xffffffff;
	player->dragMovie = NULL;
	player->dragConstraint = ddInvalidRect;
	player->keycode = (UInt16)0;
	player->xmouse = (fixed)0;
	player->xmouse = (fixed)0;
	player->activeClips = emptyList;
	player->unloadClips = emptyList;
	player->frameActions = emptyList;
	player->activeButton = NULL;
	player->buttonDown = DD_FALSE;
	player->overButton = DD_FALSE;
	player->registers[0] = ddNullValue;
	player->registers[1] = ddNullValue;
	player->registers[2] = ddNullValue;
	player->registers[3] = ddNullValue;
	player->traceFunction = NULL;
	player->traceUserData = NULL;
	player->startTime = (long)0;
	player->globals = NULL;
	player->actionContext = NULL;

	ddEventHandlerSet_init(&player->eventHandlers);

	dd_initActionEngine(player);
}


ddPlayer*
dd_newPlayer_file(FILE* f, ddImage* image)
{
	ddPlayer* player = dd_malloc(sizeof(ddPlayer));

	ddPlayer_init(player);

	player->width = ddImage_getWidth(image);
	player->height = ddImage_getHeight(image);

	player->image = image;
	player->bounds = dd_makeRect(0, FIXED_I(player->width), 0, FIXED_I(player->height));

	player->reader = dd_newFileReader(f);

	return player;
}


int
ddPlayer_readMovie(ddPlayer* player)
{
	double scale;

	if ( readMovie(player->reader) < 0 )
		return -1;

	player->backgroundColor = ddReader_getBackgroundColor(player->reader);
	player->player = ddReader_getPlayclip(player->reader);
	player->playclip = ddMovieClip_makeRootClip(player, player->player, 0);
	player->frameRate = player->reader->frameRate;

	scale = dd_min(player->width / DOUBLE_F(ddMovieClip_getWidth(player->player)),
			 player->height / DOUBLE_F(ddMovieClip_getHeight(player->player)));

	player->matrix = dd_makeScalingMatrix(FIXED_D(scale), FIXED_D(scale));
	ddDrawable_setMatrix((ddDrawable*)player->playclip, player->matrix);
	
	ddImage_fillRect(player->image, player->bounds, player->reader->background);
	
	player->mixer = dd_newSoundMixer();
	
	return 0;
}


void
ddPlayer_setImage(ddPlayer* player, ddImage* image)
{
	double scale;
	
	player->width = ddImage_getWidth(image);
	player->height = ddImage_getHeight(image);
	
	player->image = image;
	player->bounds = dd_makeRect(0, FIXED_I(player->width), 0, FIXED_I(player->height));
	
	scale = dd_min(player->width / DOUBLE_F(ddMovieClip_getWidth(player->player)),
				   player->height / DOUBLE_F(ddMovieClip_getHeight(player->player)));
	
	player->matrix = dd_makeScalingMatrix(FIXED_D(scale), FIXED_D(scale));

	ddDrawable_setMatrix((ddDrawable*)player->playclip, player->matrix);
	
	ddUpdateList_includeRect(player->updateList, player->bounds);
	ddPlayer_updateDisplay(player, NULL, NULL);
}


void
ddPlayer_step(ddPlayer* player)
{
	ddListEntry* d;

#ifdef DD_LOG
	static int frameNum = 0;
	dd_log("\n-- Frame %i --\n", frameNum++);
#endif
	
	ddSoundMixer_clearStreamBlocks(player->mixer);
	
	d = player->activeClips.head;

	while ( d != NULL )
	{
		ddDrawClip_advancePlayhead(player, (ddDrawClip*)d->item);
		d = d->next;
	}

	ddDrawClip_scheduleClipActions(player, player->playclip, "onEnterFrame");
	ddEventHandler_scheduleActions(player, player->eventHandlers.enterFrame);
	
	// step all clips in the active list
	
	d = player->activeClips.head;

	while ( d != NULL )
	{
		ddDrawClip* clip = (ddDrawClip*)d->item;
		
		if ( d->item != NULL && (clip->flags & DRAWCLIP_UNLOAD) == 0 )
			ddDrawClip_step(player, (ddDrawClip*)d->item);
		
		if ( d->next == NULL )
			; // run all queued actions
		
		d = d->next;
	}

#ifndef DD_INCLUDE_DEBUGGER
	// execute actions

	d = player->frameActions.head;

	while ( d != NULL )
	{
		ddAction* action = d->item;
		ddActionObject* object = d->next->item;
		ddListEntry* next = d->next->next;

		ddAction_execute(player, action, object);

		d = next;
	}

	d = player->frameActions.head;
	
	while ( d != NULL )
	{
		ddListEntry* next = d->next;
		dd_free(d);
		d = next;
	}
	
	player->frameActions.head = NULL;
	player->frameActions.end = NULL;
	
	ddPlayer_cleanActiveLists(player);
	ddPlayer_updateDisplay(player);
#endif /* !DD_INCLUDE_DEBUGGER */
}


#ifdef DD_INCLUDE_DEBUGGER
boolean
ddPlayer_executeFrameActions(ddPlayer* player, ddStepMode mode)
{
	ddListEntry* d = player->frameActions.head;

	while ( d != NULL )
	{
		ddListEntry* next;
		ddActionValue val;

		if ( player->actionContext == NULL )
		{
			ddAction* action = d->item;
			ddActionObject* object = d->next->item;
		
			player->actionContext = dd_newActionContext(player, object);
		
			ddActionContext_setAction(player->actionContext, action);
		}
		
		val = ddActionContext_execute(player, &player->actionContext);
		
		if ( val.type == VALUE_BREAKPOINT )
			return DD_TRUE;

		dd_destroyActionContext(player->actionContext);
		player->actionContext = NULL;
		
		next = d->next->next;
		
		dd_free(d->next);
		dd_free(d);
		
		player->frameActions.head = next;
		
		d = next;
	}
	
	player->frameActions.head = NULL;
	player->frameActions.end = NULL;

	ddPlayer_cleanActiveLists(player);

	return DD_FALSE;
}


int
ddPlayer_actionStackDepth(ddPlayer* player)
{
	if ( player->actionContext == NULL )
		return 0;
	
	return player->actionContext->depth + 1;
}


ddActionContext*
ddPlayer_actionContextAtDepth(ddPlayer* player, int depth)
{
	ddActionContext* context = player->actionContext;
	
	while ( depth-- > 0 && context != NULL )
		context = context->next;
	
	return context;
}
#endif


boolean
ddPlayer_renderFrameSoundIntoBuffer(ddPlayer* player, ddAudioSample* buffer, int* ioSize)
{
	return ddSoundMixer_renderFrameIntoBuffer(player->mixer, buffer, ioSize);
}


void
ddPlayer_cleanActiveLists(ddPlayer* player)
{
	// if an active clip has stopped or unloaded, remove it from the active list

	ddListEntry* d = player->activeClips.head;

	while ( d != NULL )
	{
		ddDrawClip* clip = (ddDrawClip*)d->item;

		if ( clip == NULL )
			continue;

		if ( (clip->flags & (DRAWCLIP_STOPPED | DRAWCLIP_UNLOAD)) != 0 )
		{
			clip->flags &= ~DRAWCLIP_ISPLAYING;
			d->item = NULL;
		}

		d = d->next;
	}

	// update stopped/removed clips

	d = player->unloadClips.head;

	while ( d != NULL )
	{
		if ( d->item != NULL )
		{
			ddDrawClip_unlink(player, (ddDrawClip*)d->item, player->updateList);
			dd_destroyDrawable((ddDrawable*)d->item);
			d->item = NULL;
		}

		d = d->next;
	}

	// clear stopped/removed clips, removed buttons
	ddPlayer_cleanList(&player->activeClips);
	ddPlayer_cleanList(&player->unloadClips);
}


void
ddPlayer_updateDisplay(ddPlayer* player, ddRect** outRects, int* outCount)
{
	int i;
	
	// vector lists are sorted in the getUpdateList cascade
	ddDrawable_getUpdateList((ddDrawable*)player->playclip,
						  player->updateList, ddIdentityMatrix /*player->matrix*/);

	ddUpdateList_constrainToRect(player->updateList, player->bounds);
	ddUpdateList_makeIntegerRects(player->updateList);

#ifdef DD_RENDER_TOPDOWN

	for ( i = 0; i < player->updateList->nRects; ++i )
		ddImage_clearRect(player->image, player->updateList->rects[i]);

#else

	for ( i = 0; i < player->updateList->nRects; ++i )
	{
		ddImage_fillRect(player->image,
				   player->updateList->rects[i], player->reader->background);
	}

#endif /* DD_RENDER_TOPDOWN */

	// shapes are rasterized in the drawInImage cascade
	for ( i = 0; i < player->updateList->nRects; ++i )
	{
		ddDrawable_drawInImage((ddDrawable*)player->playclip,
						player->image, player->updateList, ddIdentityMatrix /*player->matrix*/,
						ddEmptyCXform, player->updateList->rects[i]);
	}

#ifdef DD_RENDER_TOPDOWN

	for ( i = 0; i < player->updateList->nRects; ++i )
	{
		ddImage_blendRect(player->image,
					player->updateList->rects[i],
					player->reader->background);
	}

#endif /* DD_RENDER_TOPDOWN */

	ddUpdateList_getRects(player->updateList, outRects, outCount);
	ddUpdateList_clear(player->updateList);
}


const ddUpdateList*
ddPlayer_getUpdateList(ddPlayer* player)
{
	return player->updateList;
}


float
ddPlayer_getFrameRate(ddPlayer* player)
{
	return player->frameRate;
}


ddColor
ddPlayer_getBackgroundColor(ddPlayer* player)
{
	return player->backgroundColor;
}


void
ddPlayer_startDrag(ddPlayer* player, ddDrawClip* clip, boolean lock, ddRect constraint)
{
	player->dragMovie = clip;

	if ( lock )
	{
		fixed dragx = player->xmouse;
		fixed dragy = player->ymouse;
		
		if ( RECT_VALID(constraint) )
		{
			ddDrawClip_globalToLocal(clip->parentClip, &dragx, &dragy);
			ddRect_constrainPoint(constraint, &dragx, &dragy);
			ddDrawClip_localToGlobal(clip->parentClip, &dragx, &dragy);
		}
		
		ddDrawClip_globalToLocal(player->dragMovie, &dragx, &dragy);

		ddDrawClip_displaceXY(clip, dragx, dragy);
	}

	player->dragConstraint = constraint;
}


void
ddPlayer_stopDrag(ddPlayer* player)
{
	player->dragMovie = NULL;
	player->dragConstraint = ddInvalidRect;
}


void
ddPlayer_addItemToList(ddList* dlist, void* item)
{
	ddListEntry** list = &dlist->head;
	ddListEntry** endPtr = &dlist->end;

#ifdef DD_LOG
	ddListEntry* entry = *list;

	while ( entry != NULL )
	{
		if ( entry->item == item )
			dd_log("%p ALREADY IN LIST", item);

		entry = entry->next;
	}

	entry = dd_malloc(sizeof(ddListEntry));
#else
	ddListEntry* entry = dd_malloc(sizeof(ddListEntry));
#endif

	dd_log("%p active (entry %p)", item, entry);

	entry->item = item;
	entry->next = NULL;
	
	if ( *list == NULL )
		*list = entry;

	if ( *endPtr != NULL )
		(*endPtr)->next = entry;

	*endPtr = entry;
}


void
ddPlayer_insertListItemAfter(ddList* dlist, ddListEntry* entry, void* item)
{
	ddListEntry* newentry = dd_malloc(sizeof(ddListEntry));
	
	newentry->item = item;
	newentry->next = entry;
	
	if ( entry == NULL )
		dlist->head = dlist->end = newentry;
	else
	{
		if ( entry->next == NULL )
			dlist->end = newentry;
		
		entry->next = newentry;
	}
}


void
ddPlayer_removeItemFromList(ddList* dlist, void* item)
{
	// instead of removing the entry, we just blank it and clear it later-
	// this fixes problems with changing the list while we're running through it..

	ddListEntry* entry = dlist->head;
	
	while ( entry != NULL )
	{
		if ( entry->item == item )
		{
			entry->item = NULL;
			return;
		}

		entry = entry->next;
	}
}


void
ddPlayer_cleanList(ddList* dlist)
{
	// remove entries that have been blanked

	ddListEntry** list = &dlist->head;
	
	dlist->end = NULL;
	
	for ( ;; )
	{
		ddListEntry* entry = *list;

		if ( entry == NULL )
			return;

		if ( entry->item == NULL )
		{
			*list = entry->next;
			dd_free(entry);
		}
		else
		{
			list = &entry->next;
			dlist->end = entry;
		}
	}
}


void
ddPlayer_addFrameAction(ddPlayer* player, ddAction* action, ddActionObject* object)
{
	while ( action != NULL )
	{
		ddPlayer_addItemToList(&player->frameActions, action);
		ddPlayer_addItemToList(&player->frameActions, object);
		
		action = action->next;
	}
}


void
ddPlayer_addActiveClip(ddPlayer* player, ddDrawClip* clip)
{
	if ( clip->parent.type != MOVIECLIP_CHAR )
	{
		dd_warn("Trying to add non-clip to active list!");
		return;
	}

	ddPlayer_addItemToList(&player->activeClips, clip);
}


void
ddPlayer_removeActiveClip(ddPlayer* player, ddDrawClip* clip)
{
	dd_log("%p inactive", clip);
	
	ddPlayer_removeItemFromList(&player->activeClips, clip);
}


void
ddPlayer_removeActiveButton(ddPlayer* player, ddDrawButton* button)
{
	if ( player->activeButton == button )
		player->activeButton = NULL;
}


void
ddPlayer_addUnloadClip(ddPlayer* player, ddDrawClip* clip)
{
	if ( clip->parent.type != MOVIECLIP_CHAR )
	{
		dd_warn("Trying to add non-clip to unload list!");
		return;
	}

	ddPlayer_addItemToList(&player->unloadClips, clip);
}


void
ddPlayer_registerClass(ddPlayer* player, char* name, ddActionFunction* ctor)
{
	ddMovieClip_registerClass(player->player, name, ctor);
}


static ddDrawButton*
ddPlayer_findMouseButton(ddPlayer* player, int x, int y)
{
	return ddDrawClip_findButton(player->playclip, FIXED_I(x), FIXED_I(y));
}


ddCursorType
ddPlayer_doMouseMove(ddPlayer* player, int x, int y)
{
	ddDrawButton* b;
	
	// drag our current drag movie

	if ( player->dragMovie != NULL )
	{
		fixed oldx = player->xmouse;
		fixed oldy = player->ymouse;
		fixed newx;
		fixed newy;
		
		if ( RECT_VALID(player->dragConstraint) )
		{
			ddDrawClip_globalToLocal(player->dragMovie->parentClip, &oldx, &oldy);
			ddRect_constrainPoint(player->dragConstraint, &oldx, &oldy);
			ddDrawClip_localToGlobal(player->dragMovie->parentClip, &oldx, &oldy);
		}

		ddDrawClip_globalToLocal(player->dragMovie, &oldx, &oldy);
		
		newx = player->xmouse = FIXED_I(x);
		newy = player->ymouse = FIXED_I(y);

		if ( RECT_VALID(player->dragConstraint) )
		{
			ddDrawClip_globalToLocal(player->dragMovie->parentClip, &newx, &newy);
			ddRect_constrainPoint(player->dragConstraint, &newx, &newy);
			ddDrawClip_localToGlobal(player->dragMovie->parentClip, &newx, &newy);
		}
		
		ddDrawClip_globalToLocal(player->dragMovie, &newx, &newy);
		
		ddDrawClip_displaceXY(player->dragMovie, newx - oldx, newy - oldy);
	}
	else
	{
		player->xmouse = FIXED_I(x);
		player->ymouse = FIXED_I(y);
	}
	
	/* check if given point is on a button */

	b = ddPlayer_findMouseButton(player, x, y);

	// XXX - if active button is tracked as menu, focus is lost when mouse
	// leaves the button
	
	if ( player->buttonDown )
	{
		if ( player->activeButton == NULL )
			return DD_ARROW_CURSOR;

		else if ( b == player->activeButton )
		{
			if ( !player->overButton )
				ddDrawButton_outDownToOverDown(player, player->activeButton);

			player->overButton = DD_TRUE;
		}
		else
		{
			if ( player->overButton )
				ddDrawButton_overDownToOutDown(player, player->activeButton);

			player->overButton = DD_FALSE;
		}
	}
	else
	{
		if ( b != player->activeButton )
		{
			if ( player->activeButton != NULL )
				ddDrawButton_overUpToOutUp(player, player->activeButton);

			player->overButton = DD_FALSE;
		}

		player->activeButton = b;

		if ( player->activeButton != NULL )
		{
			if ( !player->overButton )
				ddDrawButton_outUpToOverUp(player, player->activeButton);

			player->overButton = DD_TRUE;
		}
	}

	ddEventHandler_doActions(player, player->eventHandlers.mouseMove);

	return (player->overButton) ? DD_BUTTON_CURSOR : DD_ARROW_CURSOR;
}


void
ddPlayer_doMouseDown(ddPlayer* player, int x, int y)
{
	player->buttonDown = DD_TRUE;

	if ( player->activeButton != NULL )
		ddDrawButton_overUpToOverDown(player, player->activeButton);

	ddEventHandler_doActions(player, player->eventHandlers.mouseDown);
}


void
ddPlayer_doMouseUp(ddPlayer* player, int x, int y)
{
	player->buttonDown = DD_FALSE;

	if ( player->activeButton == NULL )
		return;

	if ( player->overButton )
		ddDrawButton_overDownToOverUp(player, player->activeButton);
	else
	{
		ddDrawButton* b = ddPlayer_findMouseButton(player, x, y);

		ddDrawButton_outDownToOutUp(player, player->activeButton);

		if ( b != NULL && b != player->activeButton )
		{
			ddDrawButton_outUpToOverUp(player, b);
			player->overButton = DD_TRUE;
		}
		
		player->activeButton = b;
	}

	ddEventHandler_doActions(player, player->eventHandlers.mouseUp);
}


void
ddPlayer_doKeyDown(ddPlayer* player, UInt16 code)
{
	/* XXX - if textfield has focus, send key to it */
	/* XXX - buttons can also grab keys, not sure when tho */

	player->keycode = code;
	ddEventHandler_doActions(player, player->eventHandlers.keyDown);
}


void
ddPlayer_doKeyUp(ddPlayer* player, UInt16 code)
{
	/* XXX - if textfield has focus, send key to it */

	player->keycode = code;
	ddEventHandler_doActions(player, player->eventHandlers.keyUp);
}


void
ddPlayer_registerMouseMoveListener(ddPlayer* player, ddDrawClip* clip, ddAction* action)
{
	ddEventHandler_addHandler(&player->eventHandlers.mouseMove, clip, action);
}


void
ddPlayer_registerMouseDownListener(ddPlayer* player, ddDrawClip* clip, ddAction* action)
{
	ddEventHandler_addHandler(&player->eventHandlers.mouseDown, clip, action);
}


void
ddPlayer_registerMouseUpListener(ddPlayer* player, ddDrawClip* clip, ddAction* action)
{
	ddEventHandler_addHandler(&player->eventHandlers.mouseUp, clip, action);
}


void
ddPlayer_registerKeyDownListener(ddPlayer* player, ddDrawClip* clip, ddAction* action)
{
	ddEventHandler_addHandler(&player->eventHandlers.keyDown, clip, action);
}


void
ddPlayer_registerKeyUpListener(ddPlayer* player, ddDrawClip* clip, ddAction* action)
{
	ddEventHandler_addHandler(&player->eventHandlers.keyUp, clip, action);
}


void
ddPlayer_registerEnterFrameListener(ddPlayer* player, ddDrawClip* clip, ddAction* action)
{
	ddEventHandler_addHandler(&player->eventHandlers.enterFrame, clip, action);
}


void
ddPlayer_removeClipEventHandlers(ddPlayer* player, ddDrawClip* clip)
{
	ddEventHandler_removeHandler(&player->eventHandlers.mouseMove, clip);
	ddEventHandler_removeHandler(&player->eventHandlers.mouseDown, clip);
	ddEventHandler_removeHandler(&player->eventHandlers.mouseUp, clip);
	ddEventHandler_removeHandler(&player->eventHandlers.keyDown, clip);
	ddEventHandler_removeHandler(&player->eventHandlers.keyUp, clip);
	ddEventHandler_removeHandler(&player->eventHandlers.enterFrame, clip);
}

#ifdef DD_INCLUDE_DEBUGGER
ddMovieClip*
ddPlayer_getMovieClip(ddPlayer* player)
{
	return player->player;
}

ddDrawClip*
ddPlayer_getPlayClip(ddPlayer* player)
{
	return player->playclip;
}

ddActionObject*
ddPlayer_getGlobals(ddPlayer* player)
{
	return player->globals;
}
#endif


ddActionMovieClip*
ddPlayer_getRootClip(ddPlayer* player)
{
	return player->playclip->actionClip;
}
