/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "locals.h"
#include "object.h"


void
ddActionLocals_init(ddActionLocals* locals)
{
	locals->nLocals = 0;
	locals->buckets = NULL;
}


void
ddActionLocals_clear(ddActionLocals* locals, ddActionObject* object)
{
	int i;

	for ( i = locals->nLocals - 1; i >= 0; --i )
	{
		/* note: if we rehash properly, lookup time won't be a problem */

		ddActionHash_removeBucket(object->properties,
							locals->buckets[i]->hashcode,
							locals->buckets[i]->name);
	}

	dd_free(locals->buckets);
	locals->buckets = NULL;
	locals->nLocals = 0;
}


void
ddActionLocals_addLocal(ddActionLocals* locals, ddActionObject* object,
						char* name, ddActionValue value)
{
	locals->buckets = dd_realloc(locals->buckets,
							  (locals->nLocals + 1) * sizeof(ddActionHashBucket));

	locals->buckets[locals->nLocals] =
		ddActionHash_addValue(object->properties, name, value);

	++locals->nLocals;
}


void
ddActionLocals_addLocal_flags(ddActionLocals* locals,
							  ddActionObject* object, char* name,
							  ddActionValue value, int flags)
{
	locals->buckets = dd_realloc(locals->buckets,
							  (locals->nLocals + 1) * sizeof(ddActionHashBucket));

	locals->buckets[locals->nLocals] =
		ddActionHash_addValue_flags(object->properties, name, value, flags);

	++locals->nLocals;
}
