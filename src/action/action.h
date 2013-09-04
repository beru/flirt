/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_ACTION_H_INCLUDED
#define DD_ACTION_ACTION_H_INCLUDED

#include "../dd.h"

typedef struct _ddAction ddAction;

#include "../player/player.h"
#include "opcodes.h"
#include "object.h"

struct _ddAction
{
	ddAction* next;
	ddActionOpcode* opcodes;
	int length;
	
	char** dictionary;
	int dictionarySize;
};

void
dd_initActionEngine(ddPlayer* player);

void
dd_destroyActionEngine(ddPlayer* player);

ddActionClass*
ddPlayer_addClass(ddPlayer* player, ddActionClass* superclass, const char* name, ddNativeFunction constructor, int nargs);

void
dd_setActionTraceFunction(ddPlayer* player, void (*trace)(const char* message, void* userdata), void* userdata);

ddAction*
dd_newAction();

ddActionOpcode*
ddAction_newOpcode(ddAction* action);

void
dd_destroyAction(ddAction* action);

void
ddAction_execute(ddPlayer* player, ddAction* action, ddActionObject* object);

void
dd_doBytecodeAction(ddPlayer* player, ddActionContext** contextptr, ddActionOpcode* op);

ddActionValue
getSlashPath(ddActionContext* context, const char* str);


#ifdef DD_INCLUDE_DEBUGGER

int
ddAction_getLength(ddAction* action);

void
ddAction_setBreakpoint(ddAction* action, int line);

void
ddAction_clearBreakpoint(ddAction* action, int line);

void
ddAction_getBreakpoints(ddAction* action, int** outLines, int* outNBreaks);

#endif /* DD_INCLUDE_DEBUGGER */

#endif /* DD_ACTION_ACTION_H_INCLUDED */
