/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_CLASSES_COLOR_H_INCLUDED
#define DD_ACTION_CLASSES_COLOR_H_INCLUDED

typedef struct _ddActionColor ddActionColor;

#include "../../dd.h"
#include "../value.h"
#include "../class.h"
#include "../object.h"

#include "../../render/cxform.h"

struct _ddActionColor
{
	ddActionObject parent;
	//ddCXform cXform;
	ddActionMovieClip* clip;
};

extern ddActionClass* ddActionColorClass;

ddActionFunction*
dd_initColorClass();

#endif /* DD_ACTION_CLASSES_COLOR_H_INCLUDED */
