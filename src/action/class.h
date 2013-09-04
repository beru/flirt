/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_CLASS_H_INCLUDED
#define DD_ACTION_CLASS_H_INCLUDED

#include "actiontypes.h"
#include "object.h"

extern ddActionClass* ddActionClassClass;

struct _ddActionClass
{
	ddActionObject parent;

	void (*destroy)(ddActionObject* obj);
	void (*setProperty)(ddActionContext* context, ddActionObject* obj, char* name, ddActionValue value);
	ddActionValue (*getProperty)(ddActionContext* context, ddActionObject* obj, const char* name);
};


ddActionClass*
dd_newActionClass();

void
ddActionClass_setSuperclass(ddActionClass* classObject, ddActionClass* superclass);

void
ddActionClass_addNativeMethod(ddActionClass* classObject, char* name, ddNativeFunction function, int nargs);

#endif /* DD_ACTION_CLASS_H_INCLUDED */
