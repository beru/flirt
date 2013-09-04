/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>
#include <ctype.h>

#include "hash.h"

/* P.J. Weinberger's hash, from the dragon book */

unsigned int
dd_hashFunction(const char* name)
{
	int code = 0, loop;
	const char* p = name;

	while ( *p != '\0' )
	{
		code = (code << 4) + tolower(*p);

		if ( (loop = (code & 0xF0000000)) != 0 )
		{
			code = code ^ (loop >> 24);
			code = code ^ loop;
		}

		++p;
	}

	return code;
}


ddActionHash*
dd_newActionHash()
{
	return dd_newActionHash_size(31); // XXX - #define or something
}


ddActionHash*
dd_newActionHash_size(int size)
{
	int i;
	ddActionHash* hash = dd_malloc(sizeof(ddActionHash));
	hash->buckets = dd_malloc(size * sizeof(ddActionHashBucket));

	for ( i = 0; i < size; ++i )
		hash->buckets[i] = NULL;

	hash->size = size;
	hash->nEntries = 0;

	return hash;
}


void
dd_destroyActionHashBucket(ddActionHashBucket* bucket)
{
	if ( bucket->value.type != VALUE_NULL )
		ddActionValue_release(bucket->value);

	dd_free(bucket->name);
	dd_free(bucket);
}


void
dd_destroyActionHash(ddActionHash* hash)
{
	int i;
	ddActionHashBucket* bucket;
	ddActionHashBucket* next;

	/* destroy buckets and release the objects */

	for ( i = 0; i < hash->size; ++i )
	{
		bucket = hash->buckets[i];

		while ( bucket != NULL )
		{
			next = bucket->next;
			dd_destroyActionHashBucket(bucket);
			bucket = next;
		}
	}

	dd_free(hash->buckets);
	dd_free(hash);
}


ddActionValue
ddActionHash_findValue(ddActionHash* hash, const char* name)
{
	unsigned int hashcode = dd_hashFunction(name);
	ddActionHashBucket* bucket = hash->buckets[hashcode % hash->size];

	while ( bucket != NULL )
	{
		/* probably cheaper to look at hashcode first */
		if ( bucket->hashcode == hashcode && strcasecmp(bucket->name, name) == 0 )
			break;

		bucket = bucket->next;
	}

	if ( bucket == NULL )
		return ddUndefValue;

	return bucket->value;
}


ddActionHashBucket* 
ddActionHash_addValue_flags(ddActionHash* hash, char* name, ddActionValue value, int flags)
{
	// Note that this takes ownership of name
	
	int size = hash->size;
	unsigned int hashcode = dd_hashFunction(name);

	ddActionHashBucket*  bucket = dd_malloc(sizeof(ddActionHashBucket));

	bucket->name = name;
	bucket->value = ddActionValue_copy(value);
	bucket->hashcode = hashcode;
	bucket->flags = flags;

	bucket->next = hash->buckets[hashcode%size];
	hash->buckets[hashcode%size] = bucket;
	++hash->nEntries;

	return bucket;
}


ddActionHashBucket* 
ddActionHash_addValue(ddActionHash* hash, char* name, ddActionValue value)
{
	return ddActionHash_addValue_flags(hash, name, value, HASH_DEFAULTFLAGS);
}


void
ddActionHash_copyValues(ddActionHash* to, ddActionHash* from)
{
	// XXX - could be faster by accessing to's buckets directly
	
	int i;
	
	for ( i = 0; i < from->size; ++i )
	{
		ddActionHashBucket* bucket = from->buckets[i];
		
		while ( bucket != NULL )
		{
			ddActionHash_addValue_flags(to, bucket->name, bucket->value, bucket->flags);
			bucket = bucket->next;
		}
	}
}


void
ddActionHash_removeBucket(ddActionHash* hash, unsigned int hashcode, const char* name)
{
	int size = hash->size;
	ddActionHashBucket* bucket = hash->buckets[hashcode%size];
	ddActionHashBucket* last = NULL;

	while ( bucket != NULL )
	{
		if ( bucket->hashcode == hashcode && strcasecmp(bucket->name, name) == 0 )
		{
			if ( (bucket->flags & HASH_PERMANENT) != 0 )
				return;

			if ( last == NULL )
				hash->buckets[hashcode%size] = bucket->next;
			else
				last->next = bucket->next;

			dd_destroyActionHashBucket(bucket);
			--hash->nEntries;

			return;
		}

		last = bucket;
		bucket = bucket->next;
	}
}


void
ddActionHash_removeValue(ddActionHash* hash, const char* name)
{
	ddActionHash_removeBucket(hash, dd_hashFunction(name), name);
}


void
ddActionHash_setValue(ddActionHash* hash, char* name, ddActionValue value)
{
	int size = hash->size;
	unsigned int hashcode = dd_hashFunction(name);
	ddActionHashBucket* bucket = hash->buckets[hashcode%size];

	while ( bucket != NULL )
	{
		if ( bucket->hashcode == hashcode && strcasecmp(bucket->name, name) == 0)
			break;

		bucket = bucket->next;
	}

	if ( bucket == NULL )
	{
		ddActionHash_addValue(hash, name, value);
	}
	else if ( (bucket->flags & HASH_READONLY) == 0 )
	{
		if ( bucket->value.type != VALUE_NULL )
			ddActionValue_release(bucket->value);

		bucket->value = ddActionValue_copy(value);
		dd_free(name);
	}
}


/* XXX - make sure this doesn't screw up local buckets
(shouldn't, because they'll have the same hashcode as the buckets
 they're shadowing..) */

void
ddActionHash_rehash(ddActionHash* hash, int size)
{
	int i, code;
	ddActionHashBucket* bucket;
	ddActionHashBucket* next;
	ddActionHashBucket** newBuckets = dd_malloc(size * sizeof(ddActionHashBucket*));

	for ( i = hash->size - 1; i >= 0; --i )
	{
		bucket = hash->buckets[i];

		while ( bucket != NULL )
		{
			next = bucket->next;
			code = bucket->hashcode;
			bucket->next = newBuckets[code % size];
			newBuckets[code % size] = bucket;
			bucket = next;
		}
	}

	dd_free(hash->buckets);

	hash->size = size;
	hash->buckets = newBuckets;
}


void
ddActionHash_pushKeys(ddActionHash* hash, ddActionStack* stack)
{
	int i;
	ddActionHashBucket*  bucket;

	for ( i = hash->size - 1; i >= 0; --i )
	{
		bucket = hash->buckets[i];

		while ( bucket != NULL )
		{
			if ( (bucket->flags & HASH_DONTENUM) == 0 )
				ddActionStack_push(stack, dd_newActionValue_string(bucket->name));

			bucket = bucket->next;
		}
	}
}

#ifdef DD_DEBUG

extern void
printValue(ddActionValue val);

#include <stdio.h>

void
ddActionHash_list(ddActionHash* hash)
{
	int i;

	fprintf(stderr, "\nhash %p (%i):\n", hash, hash->nEntries);

	for ( i = 0; i < hash->size; ++i )
	{
		ddActionHashBucket* bucket = hash->buckets[i];
		//fprintf(stderr, "%i: %p\n", i, bucket);
		
		while ( bucket != NULL )
		{
			fprintf(stderr, "%s: ", bucket->name);
			printValue(bucket->value);
			fprintf(stderr, "\n");
			bucket = bucket->next;
		}
	}
}

#endif
