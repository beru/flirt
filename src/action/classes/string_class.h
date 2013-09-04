/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_CLASSES_STRING_H_INCLUDED
#define DD_ACTION_CLASSES_STRING_H_INCLUDED

typedef struct _ddActionString ddActionString;

#include "../../dd.h"
#include "../class.h"
#include "../object.h"

struct _ddActionString
{
	ddActionObject parent;

	char* string;
	int length;
};

extern ddActionClass* ddActionStringClass;

ddActionString*
dd_newActionString(char* string);

ddActionFunction*
dd_initStringClass();

#endif /* DD_ACTION_CLASSES_STRING_H_INCLUDED */
