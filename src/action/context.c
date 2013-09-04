/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>

#include "context.h"

void
ddActionContext_init(ddActionContext* context, ddPlayer* player, ddActionObject* object)
{
	context->next = NULL;
	context->action = NULL;
	context->instrPtr = NULL;
	context->endPtr = NULL;

	context->player = player;
	context->object = ddActionObject_retain(object);
	context->class = object->proto;
	context->clip = object;
	
	ddActionStack_init(&context->stack);
	ddActionLocals_init(&context->locals);

	context->isConstructor = DD_FALSE;
	
	context->actionSkip = 0;

#ifdef DD_INCLUDE_DEBUGGER
	context->label = NULL;
	context->depth = 0;
#endif
	
	ddActionObject_retain(object);
}


ddActionContext*
dd_newActionContext(ddPlayer* player, ddActionObject* object)
{
	ddActionContext* context = dd_malloc(sizeof(ddActionContext));
	ddActionContext_init(context, player, object);

	return context;
}


void
ddActionContext_pushValueContext(ddActionContext** contextPtr, ddActionObject* object)
{
	ddActionContext* context = *contextPtr;
	ddActionContext* newcontext = dd_newActionContext(context->player, object);
	
	newcontext->action = context->action;
	newcontext->instrPtr = context->instrPtr;
	newcontext->endPtr = context->endPtr;
	newcontext->next = context;

#ifdef DD_INCLUDE_DEBUGGER
	newcontext->depth = context->depth + 1;
#endif
	
	*contextPtr = newcontext;
}


void
dd_destroyActionContext(ddActionContext* context)
{
	//int i;

	ddActionLocals_clear(&context->locals, context->object);
	ddActionStack_clear(&context->stack);
	ddActionObject_release(context->object);

	/* XXX - these are leaking,
		but we have to keep these for bytecode functions */
	/*
	 for(i=0; i<context->dictionarySize; ++i)
	 dd_free(context->dictionary[i]);

	 dd_free(context->dictionary);
	 */

#ifdef DD_INCLUDE_DEBUGGER
	if ( context->label != NULL )
		dd_free(context->label);
#endif
	
	dd_free(context);
}


void
ddActionContext_addLocal(ddActionContext* context, char* name, ddActionValue value)
{
	ddActionLocals_addLocal(&context->locals, context->object, name, value);
}


void
ddActionContext_addLocal_flags(ddActionContext* context, char* name,
									ddActionValue value, int flags)
{
	ddActionLocals_addLocal_flags(&context->locals, context->object, name, value, flags);
}


ddActionValue
ddActionContext_popValue(ddActionContext* context)
{
	return ddActionStack_pop(&context->stack);
}


void
ddActionContext_pushValue(ddActionContext* context, ddActionValue val)
{
	ddActionStack_push(&context->stack, val);
}


// XXX - should limit by time instead of cycles
#define LOOP_LIMIT 200000

ddActionValue
ddActionContext_execute(ddPlayer* player, ddActionContext** contextPtr)
{
	int counter = LOOP_LIMIT;
	ddActionOpcode* instrPtr;
	ddActionOpcode* endPtr;
	ddActionValue retval = ddNullValue;
	ddActionContext* context = *contextPtr;
	
	while ( --counter > 0 )
	{
		instrPtr = context->instrPtr;
		endPtr = context->endPtr;

		if ( instrPtr >= endPtr )
		{
			ddActionOpcode op = { ACTION_RETURN, 0, NULL, ddNullValue };

			if ( context->next == NULL )
				break;

			dd_doBytecodeAction(player, &context, &op);

			continue;
		}

		if ( instrPtr->op == ACTION_END )
			break;
		
#ifdef DD_INCLUDE_DEBUGGER
		if ( instrPtr->breakpoint )
		{
			*contextPtr = context;
			return ddBreakpointValue;
		}
#endif

		++context->instrPtr;

		if ( context->actionSkip == 0 )
			dd_doBytecodeAction(player, &context, instrPtr);
		else
			--context->actionSkip;
	}

	if ( counter == 0 )
	{
		dd_warn("action terminated after too many ops!");
		return ddNullValue;
	}

	if ( context->stack.nItems > 0 )
		retval = ddActionContext_popValue(context);

	*contextPtr = context;

	return retval;
}


void
ddActionContext_setAction(ddActionContext* context, ddAction* action)
{
	context->action = action;
	context->instrPtr = action->opcodes;
	context->endPtr = context->instrPtr + action->length;
}


ddPlayer*
ddContext_getPlayer(ddActionContext* context)
{
	return context->player;
}


#ifdef DD_INCLUDE_DEBUGGER
ddAction*
ddActionContext_getAction(ddActionContext* context)
{
	return context->action;
}


int
ddActionContext_getActionOffset(ddActionContext* context)
{
	return context->instrPtr - context->action->opcodes;
}


void
ddActionContext_setLabel(ddActionContext* context, const char* label)
{
	context->label = strdup(label);
}

char*
ddActionContext_getLabel(ddActionContext* context)
{
	return context->label;
}
#endif
