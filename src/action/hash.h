/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_HASH_H_INCLUDED
#define DD_ACTION_HASH_H_INCLUDED

#include "../dd.h"

typedef struct _ddActionHash ddActionHash;
typedef struct _ddActionHashBucket ddActionHashBucket;

#include "value.h"
#include "stack.h"

#define HASH_READONLY   (1<<0)  /* this value is read-only */
#define HASH_PERMANENT  (1<<1)  /* this value should not be freed (clip..) */
#define HASH_DONTENUM   (1<<2)  /* don't present this in enumerations */

#ifdef DD_INCLUDE_DEBUGGER
#define HASH_BREAK_ON_SET	(1 << 3)
#define HASH_BREAK_ON_GET	(1 << 4)
#endif

#define HASH_DEFAULTFLAGS  0

struct _ddActionHashBucket
{
	ddActionHashBucket* next;

	char* name;
	unsigned int hashcode; /* for rehashing and faster lookup */
	int flags;

	ddActionValue value;
};

struct _ddActionHash
{
	ddActionHashBucket** buckets;
	int size;
	int nEntries;
};


ddActionHash*
dd_newActionHash();

ddActionHash*
dd_newActionHash_size(int size);

void
dd_destroyActionHash(ddActionHash* hash);

ddActionValue
ddActionHash_findValue(ddActionHash* hash, const char* name);

// These three take ownership of name:

ddActionHashBucket*
ddActionHash_addValue(ddActionHash* hash, char* name, ddActionValue value);

ddActionHashBucket*
ddActionHash_addValue_flags(ddActionHash* hash, char* name, ddActionValue value, int flags);

void
ddActionHash_setValue(ddActionHash* hash, char* name, ddActionValue value);

void
ddActionHash_rehash(ddActionHash* hash, int newSize);

void
ddActionHash_release(ddActionHash* hash);

void
ddActionHash_removeValue(ddActionHash* hash, const char* name);

void
ddActionHash_removeBucket(ddActionHash* hash, unsigned int hashcode, const char* name);

void
ddActionHash_pushKeys(ddActionHash* hash, ddActionStack* stack);

void
ddActionHash_copyValues(ddActionHash* to, ddActionHash* from);

unsigned int
dd_hashFunction(const char* name);

void
ddActionHash_list(ddActionHash* hash);

#endif /* DD_ACTION_HASH_H_INCLUDED */
