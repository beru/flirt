/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_LOCALS_H_INCLUDED
#define DD_ACTION_LOCALS_H_INCLUDED

typedef struct _ddActionLocals ddActionLocals;

#include "hash.h"

/* local vars need to be disposed of when leaving a frame- but chaining a
   separate hash table for each frame will slow down lookups.  best to put the
   locals into the same hash (since they'll be found first if there's a name
   collision) but keep a list so they're easy to pull out */

struct _ddActionLocals
{
	int nLocals;
	ddActionHashBucket** buckets;
};


void
ddActionLocals_init(ddActionLocals* locals);

void
ddActionLocals_clear(ddActionLocals* locals, ddActionObject* object);

void
ddActionLocals_addLocal(ddActionLocals* locals, ddActionObject* object,
			     char* name, ddActionValue value);

void ddActionLocals_addLocal_flags(ddActionLocals* locals,
				   ddActionObject* object, char* name,
				   ddActionValue value, int flags);

#endif /* DD_ACTION_LOCALS_H_INCLUDED */
