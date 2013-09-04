/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_STACK_H_INCLUDED
#define DD_ACTION_STACK_H_INCLUDED


// stack.h

typedef struct _ddActionStack ddActionStack;

#include "../dd.h"
#include "value.h"

struct _ddActionStack
{
	int nItems;
	int nAlloced;
	ddActionValue* items;
};


void
ddActionStack_init(ddActionStack* stack);

void
ddActionStack_clear(ddActionStack* stack);

ddActionValue
ddActionStack_pop(ddActionStack* stack);

void
ddActionStack_push(ddActionStack* stack, ddActionValue value);

ddActionValue
ddActionStack_top(ddActionStack* stack);

#endif /* DD_ACTION_STACK_H_INCLUDED */
