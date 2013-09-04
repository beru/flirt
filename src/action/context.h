/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_CONTEXT_H_INCLUDED
#define DD_ACTION_CONTEXT_H_INCLUDED

#include "../dd.h"

#include "locals.h"
#include "stack.h"
#include "object.h"
#include "opcodes.h"

#include "../player/player.h"

struct _ddActionContext
{
	ddActionContext* next;

	ddPlayer* player;
	ddActionLocals locals;
	ddActionStack stack;
	ddActionObject* object;
	ddActionObject* class;
	ddActionObject* clip;
	
	ddAction* action;
	ddActionOpcode* instrPtr;  /* pointer into instructions */
	ddActionOpcode* endPtr;    /* end of instructions- needed b/c functions
								  don't always have returns */
	
	boolean isConstructor;

#ifdef DD_INCLUDE_DEBUGGER
	char* label;
	int depth;
#endif
	
	int actionSkip;
};

ddActionContext*
dd_newActionContext(ddPlayer* player, ddActionObject* object);

void
ddActionContext_pushValueContext(ddActionContext** contextPtr, ddActionObject* object);

void
dd_destroyActionContext(ddActionContext* context);

ddActionValue
ddActionContext_execute(ddPlayer* player, ddActionContext** context);

void
ddActionContext_addLocal(ddActionContext* context, char* name, ddActionValue value);

void
ddActionContext_addLocal_flags(ddActionContext* context, char* name,
							   ddActionValue value, int flags);

ddActionValue
ddActionContext_popValue(ddActionContext* context);

void
ddActionContext_pushValue(ddActionContext* context, ddActionValue val);

void
ddActionContext_setAction(ddActionContext* context, ddAction* action);

ddPlayer*
ddContext_getPlayer(ddActionContext* context);

#ifdef DD_INCLUDE_DEBUGGER
void
ddActionContext_setLabel(ddActionContext* context, const char* label);

ddAction*
ddActionContext_getAction(ddActionContext* context);

int
ddActionContext_getActionOffset(ddActionContext* context);
#endif

#endif /* DD_ACTION_CONTEXT_H_INCLUDED */
