/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "stack.h"

#define STACK_INCREMENT 16


void
ddActionStack_init(ddActionStack* stack)
{
	stack->nItems = 0;
	stack->nAlloced = 0;
	stack->items = NULL;
}


void
ddActionStack_clear(ddActionStack* stack)
{
	if ( stack->nItems > 0 )
		dd_warn("destroying a non-empty stack");

	if ( stack->items != NULL )
	{
		dd_free(stack->items);
		stack->items = NULL;
	}

	stack->nItems = 0;
	stack->nAlloced = 0;
}


ddActionValue
ddActionStack_pop(ddActionStack* stack)
{
	if ( stack->nItems == 0 )
	{
		dd_warn("blown stack!");
		return ddNullValue;
	}

	return stack->items[--stack->nItems];
}


ddActionValue
ddActionStack_top(ddActionStack* stack)
{
	if ( stack->nItems == 0 )
	{
		dd_warn("peeking at empty stack!");
		return ddNullValue;
	}

	return stack->items[stack->nItems-1];
}


void
ddActionStack_push(ddActionStack* stack, ddActionValue value)
{
	if ( stack->nItems == stack->nAlloced )
	{
		stack->nAlloced += STACK_INCREMENT;

		stack->items = dd_realloc(stack->items,
							stack->nAlloced * sizeof(ddActionValue));
	}

	stack->items[stack->nItems] = value;
	ddActionValue_retain(value);
	++stack->nItems;
}
