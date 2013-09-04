/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>

#include "object.h"

#include "classes/object_class.h"
#include "classes/movieclip_class.h"
#include "classes/math_class.h"
#include "classes/array_class.h"
#include "classes/string_class.h"
#include "classes/color_class.h"
#include "classes/date_class.h"


void
ddActionObject_setProperty_default(ddActionContext* context, ddActionObject* object,
								   char* name, ddActionValue value)
{
	if ( strcasecmp(name, "__proto__") == 0 )
	{
		if ( object->proto != NULL )
			ddActionObject_release(object->proto);

		object->proto = ddActionValue_getObjectValue(value);
		return;
	}

	ddActionHash_setValue(object->properties, name, value);
}


ddActionValue
ddActionObject_getProperty_default(ddActionContext* context, ddActionObject* object,
								   const char* name)
{
	ddActionValue retval;

	if ( strcasecmp(name, "__proto__") == 0 )
	{
		if ( object->proto == NULL )
			return ddNullValue;
		else
			return dd_newActionValue_object(object->proto);
	}
	else if ( strcasecmp(name, "this") == 0 )
		return dd_newActionValue_object(object);
	
	else if ( strcasecmp(name, "_global") == 0 && context != NULL )
		return dd_newActionValue_object(context->player->globals);

	retval = ddActionHash_findValue(object->properties, name);

	if ( retval.type != VALUE_UNDEF )
		return retval;

	//if ( context->class != NULL )
	//	return ddActionObject_getProperty(context, context->class, name);
	else if ( object->proto != NULL )
		return ddActionObject_getProperty(context, object->proto, name);
	
	return ddUndefValue;
}


void
ddActionObject_init(ddActionObject* object)
{
	/* set default values for generic object */

	object->cachedSetProperty = NULL;
	object->cachedGetProperty = NULL;

	object->properties = dd_newActionHash();
	object->refCount = 1;
	object->proto = (ddActionObject*)ddActionObjectClass;
	
	object->nativeClass = ddActionObjectClass;
}


ddActionObject*
dd_newActionObject()
{
	ddActionObject* object = dd_malloc(sizeof(struct _ddActionObject));
	ddActionObject_init(object);
	return object;
}


void
dd_destroyActionObject(ddActionObject* object)
{
	/* clean up properties */
	ddActionClass* class = object->nativeClass;

	while ( class != NULL && class->destroy == NULL )
		class = (ddActionClass*)class->parent.proto;
	
	if ( class != NULL )
		class->destroy(object);
	
	object->refCount = OBJECT_STATIC;

	dd_destroyActionHash(object->properties);

	if ( object->proto != NULL )
		ddActionObject_release(object->proto);

	dd_free(object);
}


void
ddActionObject_callMethod(ddActionObject* object, char* name,
						  ddActionContext** contextPtr, int nargs)
{
	ddActionValue funcval = ddActionObject_getProperty(*contextPtr, object, name);
	ddActionFunction* function = ddActionValue_getFunctionValue(funcval);

	if ( function == NULL )
	{
		while ( --nargs >= 0 )
			ddActionStack_pop(&(*contextPtr)->stack);

		ddActionStack_push(&(*contextPtr)->stack, ddNullValue);
		return;
	}

	ddActionFunction_call(function, object, contextPtr, NULL, nargs, DD_FALSE);
}


ddActionValue
ddActionObject_runMethod(ddPlayer* player, ddActionObject* object,
						 char* name, int nargs, ddActionValue* args)
{
	ddActionContext* context = dd_newActionContext(player, object);
	ddActionValue funcval = ddActionObject_getProperty(context, object, name);
	ddActionFunction* function;
	ddActionValue val;
	int i;
	
	if ( funcval.type != VALUE_FUNCTION )
	{
		dd_destroyActionContext(context);
		return ddNullValue;
	}

	function = funcval.data.functionValue;

	for ( i = nargs - 1; i >= 0; --i )
		ddActionContext_pushValue(context, args[i]);

	ddActionFunction_call(function, object, &context, NULL, nargs, DD_FALSE);

	if ( function->type == FUNCTION_BYTECODE )
		val = ddActionContext_execute(player, &context);
	else
		val = ddActionContext_popValue(context);

	dd_destroyActionContext(context);
	return val;
}


void
ddActionObject_addNativeMethod(ddActionObject* object, char* name,
								 ddNativeFunction function, int nargs)
{
	ddActionFunction* func = dd_newActionNativeFunction(function, nargs);

	ddActionHash_addValue_flags(object->properties, name,
							 dd_newActionValue_function(func),
							 HASH_READONLY | HASH_PERMANENT | HASH_DONTENUM);
}


void
ddActionObject_setPrototype(ddActionObject* object, ddActionObject* proto)
{
	if ( object->proto != NULL )
		ddActionObject_release(object->proto);
	
	object->proto = ddActionObject_retain(proto);
}


void
ddActionObject_setClass(ddActionObject* object, ddActionClass* class)
{
	object->nativeClass = class;
	
	ddActionObject_setPrototype(object, (ddActionObject*)class);
}


ddActionObject*
ddActionObject_getPrototype(ddActionObject* object)
{
	return object->proto;
}


boolean
ddActionObject_isKindOfClass(ddActionObject* object, ddActionClass* classPrototype)
{
	while ( object != NULL && object != (ddActionObject*)classPrototype && object->nativeClass != classPrototype )
		object = object->proto;
	
	if ( object == NULL )
		return DD_FALSE;
	else
		return DD_TRUE;
}


void
ddActionObject_cacheSetPropertyMethod(ddActionObject* object)
{
	ddActionObject* proto = object;
	
	if ( object->cachedSetProperty != NULL )
		return;
	
	while ( proto != NULL &&
			(proto->nativeClass != ddActionClassClass || ((ddActionClass*)proto)->setProperty == NULL) )
		proto = proto->proto;
	
	if ( proto == NULL )
	{
		dd_warn("object has no class!");
		return;
	}
	
	object->cachedSetProperty = ((ddActionClass*)proto)->setProperty;
}


void
ddActionObject_cacheGetPropertyMethod(ddActionObject* object)
{
	ddActionObject* proto = object;
	
	if ( object->cachedGetProperty != NULL )
		return;
	
	while ( proto != NULL &&
			(proto->nativeClass != ddActionClassClass || ((ddActionClass*)proto)->getProperty == NULL) )
		proto = proto->proto;
	
	if ( proto == NULL )
	{
		dd_warn("object has no class!");
		return;
	}
	
	object->cachedGetProperty = ((ddActionClass*)proto)->getProperty;
}


void
ddActionObject_copyProperties(ddActionObject* to, ddActionObject* from)
{
	ddActionHash_copyValues(to->properties, from->properties);
}


void
ddActionObject_release(ddActionObject* object)
{
	if ( object == NULL || OBJECT_ISSTATIC(object) )
		return;

#ifdef DD_LOG
	if ( object->refCount > 1024 )
		dd_log("%p BAD REFCOUNT", object);
	
	dd_log("%p release (%i)", object, object->refCount - 1);
#endif

	if ( --object->refCount <= 0 )
		dd_destroyActionObject(object);
}


ddActionObject*
ddActionObject_retain(ddActionObject* object)
{
	if ( object == NULL || OBJECT_ISSTATIC(object) )
		return object;

	dd_log("%8p retain (%i)", object, object->refCount + 1);

	++object->refCount;

	return object;
}


ddActionFunction*
ddActionObject_getFunctionNamed(ddActionObject* object, const char* name)
{
	ddActionValue val = ddActionObject_getProperty(NULL, object, name);
	
	return ddActionValue_getFunctionValue(val);
}


#ifdef DD_DEBUG

void
ddActionObject_list(ddActionObject* object)
{
	ddActionHash_list(object->properties);
}

#endif


#ifdef DD_INCLUDE_DEBUGGER

void
ddActionObject_getProperties(ddActionObject* object, char*** outNames, ddActionValue** outValues, int* outCount)
{
	ddActionHash* properties = object->properties;
	int count = properties->nEntries;
	ddActionValue* values = dd_malloc(sizeof(ddActionValue) * count);
	char** names = dd_malloc(sizeof(char*) * count);
	int i;
	int nval = 0;

	for ( i = 0; i < properties->size; ++i )
	{
		ddActionHashBucket* bucket = properties->buckets[i];

		while ( bucket != NULL )
		{
			if ( nval >= count )
				dd_log("values > hash->nEntries!");

			names[nval] = bucket->name;
			values[nval] = bucket->value;

			++nval;

			bucket = bucket->next;
		}
	}

	*outNames = names;
	*outValues = values;
	*outCount = count;
}

#endif
