/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "class.h"
#include "classes/object_class.h"

ddActionClass* ddActionClassClass = NULL;

ddActionClass*
dd_newActionClass()
{
	ddActionClass* class = dd_malloc(sizeof(ddActionClass));
	
	ddActionObject_init((ddActionObject*)class);
	
	class->parent.refCount = OBJECT_STATIC;
	
	class->getProperty = NULL;
	class->setProperty = NULL;
	class->destroy = NULL;
	
	ddActionObject_setClass((ddActionObject*)class, ddActionClassClass);
	ddActionClass_setSuperclass(class, ddActionObjectClass);

	return class;
}


void
ddActionClass_setSuperclass(ddActionClass* class, ddActionClass* superclass)
{
	ddActionObject_setPrototype((ddActionObject*)class, (ddActionObject*)superclass);
}


void
ddActionClass_addNativeMethod(ddActionClass* classObject, char* name, ddNativeFunction function, int nargs)
{
	ddActionObject_addNativeMethod((ddActionObject*)classObject, name, function, nargs);
}
