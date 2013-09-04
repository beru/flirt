/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <stdio.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <unistd.h>

#include <math.h>

#include "action.h"

#include "context.h"
#include "function.h"

#include "classes/object_class.h"
#include "classes/function_class.h"
#include "classes/array_class.h"
#include "classes/color_class.h"
#include "classes/date_class.h"
#include "classes/math_class.h"
#include "classes/movieclip_class.h"
#include "classes/string_class.h"


void
dd_initActionEngine(ddPlayer* player)
{
	ddActionObject* globals;
	ddActionKeyObject* key;
	
	player->startTime = dd_getTicks();
	srand(time(NULL));

	ddActionObjectClass = dd_newActionClass();
	ddActionClassClass = dd_newActionClass();
	
	ddActionObject_setClass((ddActionObject*)ddActionObjectClass, ddActionClassClass);
	ddActionClass_setSuperclass(ddActionObjectClass, NULL);
	
	ddActionObject_setClass((ddActionObject*)ddActionClassClass, ddActionClassClass);
	ddActionClass_setSuperclass(ddActionClassClass, ddActionObjectClass);

	ddActionFunctionClass = dd_newActionClass();

	globals = dd_newActionObject();
	key = dd_initKeyObject();
	
	/* System.capabilities object:
language: en-US
input: point
manufacturer: Macromedia Macintosh
os: Mac OS 10.3.4
serverString: A=t&MP3=t&AE=t&VE=t&ACC=f&DEB=t&V=MAC%206%2C0%2C21%2C0&M=Macromedia Macintosh&R=1280x854&DP=72&COL=color&AR=1&I=point&OS=Mac OS 10.3.4&L=en
isDebugger: true
version: MAC 6,0,21,0
hasAudio: true
hasMP3: true
hasAudioEncoder: true
hasVideoEncoder: true
screenResolutionX: 1280
screenResolutionY: 854
screenDPI: 72
screenColor: color
pixelAspectRatio: 1
hasAccessibility: false
		*/
	
	ddActionHash_addValue(globals->properties, strdup("Key"), dd_newActionValue_object((ddActionObject*)key));
	
	ddActionHash_addValue(globals->properties, strdup("Object"),
						  dd_newActionValue_function(dd_initObjectClass()));

	ddActionHash_addValue(globals->properties, strdup("Function"),
						  dd_newActionValue_function(dd_initFunctionClass()));
	
	ddActionHash_addValue(globals->properties, strdup("Math"),
						  dd_newActionValue_object(dd_initMathObject()));
	
	ddActionHash_addValue(globals->properties, strdup("Array"),
						  dd_newActionValue_function(dd_initArrayClass()));
	
	ddActionHash_addValue(globals->properties, strdup("String"),
						  dd_newActionValue_function(dd_initStringClass()));
	
	ddActionHash_addValue(globals->properties, strdup("Color"),
						  dd_newActionValue_function(dd_initColorClass()));
	
	ddActionHash_addValue(globals->properties, strdup("MovieClip"),
						  dd_newActionValue_function(dd_initMovieClipClass()));
	
	ddActionHash_addValue(globals->properties, strdup("Date"),
						  dd_newActionValue_function(dd_initDateClass()));
	
	player->globals = globals;
	player->keyObject = key;
}


void
dd_destroyActionEngine(ddPlayer* player)
{
	dd_destroyActionObject(player->globals);
	dd_destroyActionObject((ddActionObject*)player->keyObject);
}


void
dd_setActionTraceFunction(ddPlayer* player, void (*trace)(const char* message, void* userdata), void* data)
{
	player->traceFunction = trace;
	player->traceUserData = data;
}


ddAction*
dd_newAction()
{
	ddAction* action = dd_malloc(sizeof(struct _ddAction));
	action->next = NULL;
	action->opcodes = NULL;
	action->length = 0;
	return action;
}


void
dd_destroyAction(ddAction* action)
{
	if ( action->next != NULL )
		dd_destroyAction(action->next);
	
	if ( action->opcodes != NULL )
	{
		int i;
		
		for ( i = 0; i < action->length; ++i )
			dd_destroyActionOpcode(action->opcodes[i]);
		
		dd_free(action->opcodes);
	}
	
	dd_free(action);
}


void
ddAction_execute(ddPlayer* player, ddAction* action, ddActionObject* object)
{
	ddActionContext* context = dd_newActionContext(player, object);
	ddActionValue val;
	
	while ( action != NULL )
	{
		context->instrPtr = action->opcodes;
		context->endPtr = action->opcodes + action->length;
	
		val = ddActionContext_execute(player, &context);
	
		ddActionValue_release(val);
		action = action->next;
	}
	
	dd_destroyActionContext(context);
}


ddActionClass*
ddPlayer_addClass(ddPlayer* player, ddActionClass* superclass, const char* name, ddNativeFunction constructor, int nargs)
{
	ddActionClass* class = dd_newActionClass();
	ddActionFunction* ctor = dd_newActionNativeFunction(constructor, nargs);

	ddActionFunction_setPrototype(ctor, (ddActionObject*)class);

	ddActionHash_addValue(player->globals->properties, strdup(name), dd_newActionValue_function(ctor));

	return class;
}


#ifdef DD_INCLUDE_DEBUGGER

int
ddAction_getLength(ddAction* action)
{
	int len = action->length;
	
	if ( action->next != NULL )
		len += ddAction_getLength(action->next);
	
	return len;
}


void
ddAction_setBreakpoint(ddAction* action, int line)
{
	action->opcodes[line].breakpoint = DD_TRUE;
}


void
ddAction_clearBreakpoint(ddAction* action, int line)
{
	action->opcodes[line].breakpoint = DD_FALSE;
}


void
ddAction_getBreakpoints(ddAction* action, int** outLines, int* outNBreaks)
{
	int count = *outNBreaks;
	int* list = *outLines;
	int i;
	
	for ( i = 0; i < action->length; ++i )
	{
		if ( !action->opcodes[i].breakpoint )
			continue;
		
		list = dd_realloc(list, (count + 1) * sizeof(int));
		list[count] = i;
		++count;
	}
	
	*outLines = list;
	*outNBreaks = count;
	
	if ( action->next != NULL )
		ddAction_getBreakpoints(action->next, outLines, outNBreaks);
}

#endif
