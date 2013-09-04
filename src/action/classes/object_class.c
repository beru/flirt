/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>

#include "object_class.h"
#include "../function.h"
#include "../../player/player.h"

ddActionClass* ddActionObjectClass = NULL;


static ddActionValue
Object_registerClass(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue idval = ddActionContext_popValue(context);
	ddActionValue protoval = ddActionContext_popValue(context);

	char* name = ddActionValue_getStringValue_release(context, idval);
	ddActionObject* proto = ddActionValue_getObjectValue(protoval);

	if ( protoval.type == VALUE_FUNCTION )
		ddPlayer_registerClass(context->player, name, (ddActionFunction*)proto);

	ddActionObject_release(proto);
	ddActionValue_release(protoval);
	
	return ddNullValue;
}


static ddActionValue
Object_addProperty(ddActionObject* object, ddActionContext* context, int nargs)
{
	dd_warn("Object.addProperty not implemented");
	return ddNullValue;
}


static ddActionValue
Object_watch(ddActionObject* object, ddActionContext* context, int nargs)
{
	dd_warn("Object.watch not implemented");
	return ddNullValue;
}


static ddActionValue
Object_unwatch(ddActionObject* object, ddActionContext* context, int nargs)
{
	dd_warn("Object.unwatch not implemented");
	return ddNullValue;
}


static ddActionValue
Object_constructor(ddActionObject* object, ddActionContext* context, int nargs)
{
	// XXX - args can be bool, number, string..?
	
	return dd_newActionValue_object(dd_newActionObject());
}


ddActionFunction*
dd_initObjectClass()
{
	ddActionObject* classObject;
	ddActionFunction* ctor = dd_newActionNativeFunction(Object_constructor, FUNCTION_VARARGS);

	// class is already created
	ddActionObjectClass->setProperty = ddActionObject_setProperty_default;
	ddActionObjectClass->getProperty = ddActionObject_getProperty_default;
	
	classObject = (ddActionObject*)ddActionObjectClass;

	ddActionObject_addNativeMethod((ddActionObject*)ctor, strdup("registerClass"), Object_registerClass, 2);

	ddActionObject_addNativeMethod(classObject, strdup("addProperty"), Object_addProperty, 3);
	ddActionObject_addNativeMethod(classObject, strdup("watch"), Object_watch, 3);
	ddActionObject_addNativeMethod(classObject, strdup("unwatch"), Object_unwatch, 1);

	return ctor;
}
