/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_VALUE_H_INCLUDED
#define DD_ACTION_VALUE_H_INCLUDED

#include "actiontypes.h"

extern const ddActionValue ddFalseValue;
extern const ddActionValue ddTrueValue;
extern const ddActionValue ddUndefValue;
extern const ddActionValue ddNullValue;
extern const ddActionValue ddSuperValue;

#ifdef DD_INCLUDE_DEBUGGER
extern const ddActionValue ddBreakpointValue;
#endif

ddActionValue
dd_newActionValue_int(int num);

ddActionValue
dd_newActionValue_double(double num);

ddActionValue
dd_newActionValue_string(const char* string);

ddActionValue
dd_newActionValue_string_noCopy(char* string);

ddActionValue
dd_newActionValue_object(ddActionObject* object);

ddActionValue
dd_newActionValue_function(ddActionFunction* function);

static inline ddActionValue
dd_newActionValue_boolean(int num)
{
	if ( num )
		return ddTrueValue;
	else
		return ddFalseValue;
}

ddActionValue
ddActionValue_copy(ddActionValue val);

ddActionValue
ddActionValue_retain(ddActionValue val);

void
ddActionValue_release(ddActionValue val);

ddActionFunction*
ddActionValue_getFunctionValue(ddActionValue value);

ddActionObject*
ddActionValue_getObjectValue(ddActionValue value);

double
ddActionValue_getDoubleValue(ddActionValue value);

int
ddActionValue_getIntValue(ddActionValue value);

char*
ddActionValue_getStringValue(ddActionContext* context, ddActionValue value);

char*
ddActionValue_getStringValue_release(ddActionContext* context, ddActionValue value);

int
ddActionValue_getBooleanValue(ddActionValue value);

ddActionValue
ddActionValue_increment(ddActionValue val);

ddActionValue
ddActionValue_decrement(ddActionValue val);

const char*
ddActionValue_getTypeName(ddActionValue a);

#endif /* DD_ACTION_VALUE_H_INCLUDED */
