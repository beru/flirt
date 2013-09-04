/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_ACTIONTYPES_H_INCLUDED
#define DD_ACTION_ACTIONTYPES_H_INCLUDED

#include "../dd.h"

typedef struct _ddActionObject ddActionObject;
typedef struct _ddActionFunction ddActionFunction;
typedef struct _ddActionContext ddActionContext;
typedef struct _ddActionClass ddActionClass;

typedef enum
{
	VALUE_NULL,
	VALUE_UNDEF,
	VALUE_INTEGER,
	VALUE_DOUBLE,
	VALUE_BOOLEAN,
	VALUE_STRING,
	VALUE_OBJECT,
	VALUE_FUNCTION,
	VALUE_TEXTFIELD,
	VALUE_PROPERTY,
	VALUE_SUPER,
#ifdef DD_INCLUDE_DEBUGGER
	VALUE_BREAKPOINT,
#endif
} ddActionValueType;

#define VALUE_STATIC -1

typedef struct _ddDrawTextField ddTextFieldValue;

struct _ddActionValue
{
	ddActionValueType type;
	
	union
	{
		int intValue;
		double doubleValue;
		ddActionObject* objectValue;
		ddActionFunction* functionValue;
		char* stringValue;
		ddTextFieldValue* textFieldValue;
	} data;
};

typedef struct _ddActionValue ddActionValue;

typedef ddActionValue (*ddNativeFunction)(ddActionObject* object, ddActionContext* context, int nargs);

#endif /* DD_ACTION_ACTIONTYPES_H_INCLUDED */
