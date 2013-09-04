/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "event.h"


void
ddEventHandlerSet_init(ddEventHandlerSet* set)
{
	set->mouseMove = NULL;
	set->mouseDown = NULL;
	set->mouseUp = NULL;
	set->keyDown = NULL;
	set->keyUp = NULL;
	set->enterFrame = NULL;
}


void
dd_destroyEventHandlerSet(ddEventHandlerSet* set)
{
	ddEventHandler_clear(set->mouseMove);
	ddEventHandler_clear(set->mouseDown);
	ddEventHandler_clear(set->mouseUp);
	ddEventHandler_clear(set->keyDown);
	ddEventHandler_clear(set->keyUp);
	ddEventHandler_clear(set->enterFrame);
}


void
ddEventHandler_addHandler(ddEventHandler** list,
						  ddDrawClip* drawclip, ddAction* action)
{
	ddEventHandler* newhandler = dd_malloc(sizeof(struct _ddEventHandler));

	newhandler->clip = drawclip->actionClip;
	newhandler->action = action;
	newhandler->next = *list;
	*list = newhandler;
}


void
ddEventHandler_clear(ddEventHandler* list)
{
	while ( list != NULL )
	{
		ddEventHandler* next = list->next;
		dd_free(list);
		list = next;
	}
}


void
ddEventHandler_removeHandler(ddEventHandler** list, ddDrawClip* drawclip)
{
	ddActionMovieClip* clip = drawclip->actionClip;
	ddEventHandler* handler = *list;

	if ( handler == NULL )
		return;
	
	if ( handler->clip == clip )
	{
		*list = handler->next;
		dd_free(handler);
		return;
	}

	while ( handler->next != NULL )
	{
		if ( handler->next->clip == clip )
		{
			ddEventHandler* kill = handler->next;
			handler->next = handler->next->next;
			dd_free(kill);
			return;
		}

		handler = handler->next;
	}
}


/* called from whatever receives these events from window interface */

void
ddEventHandler_doActions(ddPlayer* player, ddEventHandler* handler)
{
	while ( handler != NULL )
	{
		ddAction_execute(player, handler->action, (ddActionObject*)handler->clip);
		handler = handler->next;
	}
}


void
ddEventHandler_scheduleActions(ddPlayer* player, ddEventHandler* handler)
{
	while ( handler != NULL )
	{
		ddPlayer_addFrameAction(player, handler->action, (ddActionObject*)handler->clip);
		handler = handler->next;
	}
}
