/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

/*
void
Function_getProperty()
{
	if ( strcmp(name, "prototype") == 0 )
	{
		ddActionFunction_createPrototype(function);
		return function->prototype;
	}
}
*/



/*
 
 Constructor and Prototype:
 
 constructor function's prototype property is the prototype object
 prototype object's constructor property is the constructor
 
 prototype objects's __proto__ is 
 
*/

#include <string.h>

#include "function_class.h"

ddActionClass* ddActionFunctionClass = NULL;

static ddActionValue
Function_constructor(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


void
dd_destroyActionFunction(ddActionObject* object)
{
	ddActionFunction* function = (ddActionFunction*)object;
	
	if ( function->prototype != NULL )
		ddActionObject_release(function->prototype);
	
	if ( function->type == FUNCTION_BYTECODE )
		dd_destroyAction(function->data.bytecode.action);
}


static ddActionValue
ddActionFunction_getProperty(ddActionContext* context, ddActionObject* object, const char* name)
{
	ddActionFunction* function = (ddActionFunction*)object;
	
	if ( strcasecmp(name, "prototype") == 0 )
		return dd_newActionValue_object(ddActionFunction_getPrototype(function));
	else
		return ddActionObject_getProperty_default(context, object, name);
}


static void
ddActionFunction_setProperty(ddActionContext* context, ddActionObject* object,
							 char* name, ddActionValue value)
{
	ddActionFunction* function = (ddActionFunction*)object;
	
	if ( strcasecmp(name, "prototype") == 0 )
		ddActionFunction_setPrototype(function, ddActionValue_getObjectValue(value));
	else
		ddActionObject_setProperty_default(context, object, name, value);
}


static ddActionValue
Function_call(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Function_apply(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


ddActionFunction*
dd_initFunctionClass()
{
	ddActionFunction* ctor = dd_newActionNativeFunction(Function_constructor, FUNCTION_VARARGS);
	ddActionObject* classObject;

	// class is already created
	classObject = (ddActionObject*)ddActionFunctionClass;
	
	ddActionObject_addNativeMethod(classObject, strdup("call"), Function_call, 0);
	ddActionObject_addNativeMethod(classObject, strdup("apply"), Function_apply, 0);
	
	ddActionFunctionClass->getProperty = ddActionFunction_getProperty;
	ddActionFunctionClass->setProperty = ddActionFunction_setProperty;
	ddActionFunctionClass->destroy = dd_destroyActionFunction;
	
	ddActionFunction_setPrototype(ctor, classObject);

	return ctor;
}
