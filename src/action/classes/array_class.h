/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_CLASSES_ARRAY_H_INCLUDED
#define DD_ACTION_CLASSES_ARRAY_H_INCLUDED

#include "../../dd.h"
#include "../value.h"
#include "../class.h"
#include "../object.h"

typedef struct _ddActionArray ddActionArray;

struct _ddActionArray
{
	ddActionObject parent;
	int length;
};

extern ddActionClass* ddActionArrayClass;

ddActionArray*
dd_newActionArray();

ddActionArray*
dd_newActionArray_size(int size);

void
ddActionArray_setMember(ddActionArray* array, int member, ddActionValue value);

ddActionFunction*
dd_initArrayClass();

#endif /* DD_ACTION_CLASSES_ARRAY_H_INCLUDED */
