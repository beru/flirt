/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_EVENT_H_INCLUDED
#define DD_ACTION_EVENT_H_INCLUDED

/* player (un)registers event-driven actions here,
top-level code has to send events in from window interface */

#include "../dd.h"

typedef struct _ddEventHandler ddEventHandler;
typedef struct _ddEventHandlerSet ddEventHandlerSet;

struct _ddEventHandlerSet
{
	struct _ddEventHandler* mouseMove;
	struct _ddEventHandler* mouseDown;
	struct _ddEventHandler* mouseUp;
	struct _ddEventHandler* keyDown;
	struct _ddEventHandler* keyUp;
	struct _ddEventHandler* enterFrame;
};

#include "action.h"
#include "classes/movieclip_class.h"

struct _ddEventHandler
{
	ddEventHandler* next;

	ddActionMovieClip* clip;
	ddAction* action;
};


void
ddEventHandlerSet_init(ddEventHandlerSet* set);

void
dd_destroyEventHandlerSet(ddEventHandlerSet* set);

void
ddEventHandler_addHandler(ddEventHandler** list,
						  ddDrawClip* clip, ddAction* action);

void
ddEventHandler_clear(ddEventHandler* list);

void
ddEventHandler_removeHandler(ddEventHandler** list, ddDrawClip* drawclip);

/* called from whatever receives these events from window interface */

void
ddEventHandler_doActions(ddPlayer* player, ddEventHandler* handler);

void
ddEventHandler_scheduleActions(ddPlayer* player, ddEventHandler* handler);

#endif /* DD_ACTION_EVENT_H_INCLUDED */
