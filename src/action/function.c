/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>

#include "function.h"
#include "classes/array_class.h"
#include "classes/function_class.h"

void
ddActionFunction_init(ddActionFunction* function)
{
	ddActionObject_init((ddActionObject*)function);
	
	function->prototype = NULL;
	function->type = FUNCTION_UNDEFINED;
	
	ddActionObject_setClass((ddActionObject*)function, ddActionFunctionClass);
	
#ifdef DD_INCLUDE_DEBUGGER
	function->breakpoint = DD_FALSE;
#endif	
}


ddActionFunction*
dd_newActionBytecodeFunction(ddAction* action, int nargs, char** args)
{
	ddActionFunction* function = dd_malloc(sizeof(struct _ddActionFunction));
	
	ddActionFunction_init(function);
	
	function->type = FUNCTION_BYTECODE;
	function->nargs = nargs;
	function->data.bytecode.args = args;
	function->data.bytecode.action = action;
	function->needsArguments = DD_FALSE;

	return function;
}


ddActionFunction*
dd_newActionNativeFunction(ddNativeFunction native, int nargs)
{
	ddActionFunction* function = dd_malloc(sizeof(struct _ddActionFunction));

	ddActionFunction_init(function);

	function->type = FUNCTION_NATIVE;
	function->nargs = nargs;
	function->data.native = native;

	return function;
}


void
ddActionFunction_call(ddActionFunction* function, ddActionObject* object,
					  ddActionContext** contextPtr, ddActionObject* class,
					  int nargs, boolean isConstructor)
{
	ddActionContext* context = *contextPtr;

	if ( function == NULL )
	{
		int i;

		for ( i = 0; i < nargs; ++i )
			ddActionStack_pop(&context->stack);

		ddActionStack_push(&context->stack, ddNullValue);
		return;
	}

	if ( function->type == FUNCTION_NATIVE )
	{
		ddActionValue retval;

		if ( function->data.native == NULL )
		{
			dd_warn("Empty native function!");
			ddActionStack_push(&context->stack, ddNullValue);
			return;
		}

		if ( function->nargs != FUNCTION_VARARGS && nargs < function->nargs )
		{
			/* XXX - ugh.  Let's just wedge in extra blank vals for now.. */

			int i;
			ddActionStack tmp;

			ddActionStack_init(&tmp);

			for ( i = 0; i < nargs; ++i )
				ddActionStack_push(&tmp, ddActionStack_pop(&context->stack));

			for ( i = 0; i < function->nargs-nargs; ++i )
				ddActionStack_push(&context->stack, ddUndefValue);

			for ( i = 0; i < nargs; ++i )
				ddActionStack_push(&context->stack, ddActionStack_pop(&tmp));

			ddActionStack_clear(&tmp);
		}

		retval = (function->data.native)(object, context, nargs);

		if ( function->nargs != FUNCTION_VARARGS )
		{
			while ( --nargs >= function->nargs )
				ddActionValue_release(ddActionStack_pop(&context->stack));
		}

		// XXX - constructors should copy 'object' object properties so script constructors can
		// set properties before calling super()--even though that sort of thing is frowned upon..
		
		ddActionContext_pushValue(context, retval);
	}
	else
	{
		/* function->type == FUNCTION_BYTECODE */

		int i;
		ddActionArray* args = NULL;

		ddActionContext* newcontext = dd_newActionContext(context->player, object);
		ddActionContext_setAction(newcontext, function->data.bytecode.action);
		newcontext->player = context->player;
		
		newcontext->next = context;
		*contextPtr = newcontext;
		
		newcontext->class = class;

		if ( isConstructor )
			newcontext->isConstructor = DD_TRUE;
		
		/* create argument variables & arguments array */

		if ( function->needsArguments )
			args = dd_newActionArray_size(nargs);

		for ( i = 0; i < nargs; ++i )
		{
			ddActionValue arg = ddActionStack_pop(&context->stack);

			if ( function->needsArguments )
				ddActionArray_setMember(args, i, arg);

			if ( i < function->nargs )
				ddActionContext_addLocal(newcontext, strdup(function->data.bytecode.args[i]), arg);
		}

		if ( function->needsArguments )
		{
			// arguments.callee points back to function (for anonymous functions)
			ddActionObject_setProperty(newcontext, (ddActionObject*)args, strdup("callee"), dd_newActionValue_function(function));
			
			// callee and length
			
			ddActionContext_addLocal_flags(newcontext, strdup("arguments"),
								 dd_newActionValue_object((ddActionObject*)args),
								 HASH_DONTENUM);

			ddActionObject_release((ddActionObject*)args);
		}

		if ( function->nargs != FUNCTION_VARARGS )
		{
			while ( --nargs >= function->nargs )
				ddActionValue_release(ddActionStack_pop(&context->stack));
		}
	}
}


void
ddActionFunction_setNeedsArgumentsArray(ddActionFunction* function)
{
	function->needsArguments = DD_TRUE;
}


ddAction*
ddActionFunction_getBytecodeAction(ddActionFunction* function)
{
	if ( function != NULL && function->type == FUNCTION_BYTECODE )
		return function->data.bytecode.action;
	else
		return NULL;
}


void
ddActionFunction_setPrototype(ddActionFunction* function, ddActionObject* object)
{
	if ( function->prototype == NULL )
	{
		if ( object != NULL )
			ddActionHash_addValue_flags(object->properties, strdup("constructor"), dd_newActionValue_function(function), HASH_READONLY | HASH_DONTENUM);
	}
	else
		ddActionObject_release(function->prototype);
	
	function->prototype = ddActionObject_retain(object);
}


ddActionObject*
ddActionFunction_getPrototype(ddActionFunction* function)
{
	if ( function->prototype == NULL )
	{
		function->prototype = dd_newActionObject();
		
		ddActionHash_addValue_flags(function->prototype->properties, strdup("constructor"), dd_newActionValue_function(function), HASH_READONLY | HASH_DONTENUM);
	}
	
	return function->prototype;
}
