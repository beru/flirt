/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_FUNCTION_H_INCLUDED
#define DD_ACTION_FUNCTION_H_INCLUDED

#include "object.h"

/* everything's in object.h for stupid circularity problem */

ddActionValue
ddActionObject_runMethod(ddPlayer* player, ddActionObject* object,
						 char* name, int nargs, ddActionValue* args);

ddAction*
ddActionFunction_getBytecodeAction(ddActionFunction* function);

void
ddActionFunction_setPrototype(ddActionFunction* function, ddActionObject* object);

ddActionObject*
ddActionFunction_getPrototype(ddActionFunction* function);

#endif /* DD_ACTION_FUNCTION_H_INCLUDED */
