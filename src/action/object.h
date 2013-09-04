/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_OBJECT_H_INCLUDED
#define DD_ACTION_OBJECT_H_INCLUDED

#include "actiontypes.h"
#include "value.h"
#include "hash.h"

struct _ddActionObject
{
	int refCount;
	ddActionObject* proto;
	ddActionHash* properties;

	void (*cachedSetProperty)(ddActionContext* context, ddActionObject* obj, char* name, ddActionValue value);
	ddActionValue (*cachedGetProperty)(ddActionContext* context, ddActionObject* obj, const char* name);

	ddActionClass* nativeClass;
};

#define OBJECT_STATIC -1
#define OBJECT_ISSTATIC(o) ((o)->refCount == OBJECT_STATIC)


#include "action.h"
#include "context.h"

typedef enum
{
	FUNCTION_UNDEFINED,
	FUNCTION_NATIVE,
	FUNCTION_BYTECODE
} ddActionFunctionType;

#define FUNCTION_VARARGS -1  /* if it's doing its own arg handling */

struct _ddActionFunction
{
	ddActionObject parent;

	ddActionObject* prototype;
	
	ddActionFunctionType type;
	int nargs;
	boolean needsArguments;

	union
	{
		ddNativeFunction native;

		struct
		{
			ddAction* action;
			char** args;
		} bytecode;

	} data;

#ifdef DD_INCLUDE_DEBUGGER
	boolean breakpoint;
#endif
};

#include "context.h"

ddActionFunction*
dd_newActionBytecodeFunction(ddAction* action, int nargs, char** args);

ddActionFunction*
dd_newActionNativeFunction(ddNativeFunction function, int nargs);

void
ddActionFunction_call(ddActionFunction* function, ddActionObject* object,
					  ddActionContext** contextPtr, ddActionObject* class,
					  int nargs, boolean isConstructor);

void
ddActionFunction_setNeedsArgumentsArray(ddActionFunction* function);

void
ddActionObject_callMethod(ddActionObject* object, char* name, ddActionContext** contextPtr, int nargs);

void
ddActionObject_addNativeMethod(ddActionObject* object, char* name, ddNativeFunction function, int nargs);

void
ddActionObject_setPrototype(ddActionObject* object, ddActionObject* proto);

void
ddActionObject_setClass(ddActionObject* object, ddActionClass* class);

ddActionObject*
ddActionObject_getPrototype(ddActionObject* object);

boolean
ddActionObject_isKindOfClass(ddActionObject* object, ddActionClass* classPrototype);

ddActionObject*
dd_newActionObject();

void
ddActionObject_init(ddActionObject* object);

/* always call this, not the object's destroy method! */
void
dd_destroyActionObject(ddActionObject* object);

void
ddActionObject_setProperty_default(ddActionContext* context, ddActionObject* object,
								   char* name, ddActionValue value);

ddActionValue
ddActionObject_getProperty_default(ddActionContext* context, ddActionObject* object,
								   const char* name);

void
ddActionObject_cacheSetPropertyMethod(ddActionObject* object);

void
ddActionObject_cacheGetPropertyMethod(ddActionObject* object);

static inline void
ddActionObject_setProperty(ddActionContext* context, ddActionObject* object, char* name, ddActionValue value)
{
	if ( object->cachedSetProperty == NULL )
		ddActionObject_cacheSetPropertyMethod(object);
	
	if ( object != NULL )
		object->cachedSetProperty(context, object, name, value);
}

static inline ddActionValue
ddActionObject_getProperty(ddActionContext* context, ddActionObject* object, const char* name)
{
	if ( object == NULL || name == NULL )
		return ddUndefValue;
	else
	{
		if ( object->cachedGetProperty == NULL )
			ddActionObject_cacheGetPropertyMethod(object);
		
		return object->cachedGetProperty(context, object, name);
	}
}


void
ddActionObject_copyProperties(ddActionObject* to, ddActionObject* from);

ddActionObject*
ddActionObject_retain(ddActionObject* object);

void
ddActionObject_release(ddActionObject* object);

ddActionFunction*
ddActionObject_getFunctionNamed(ddActionObject* object, const char* name);

#endif /* DD_ACTION_OBJECT_H_INCLUDED */
