/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>

#include "date_class.h"
#include "../function.h"

ddActionClass* ddActionDateClass = NULL;

static ddActionValue
Date_constructor(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionDate* date;
	
	date = dd_malloc(sizeof(ddActionDate));
	
	ddActionObject_init((ddActionObject*)date);
	
	return dd_newActionValue_object((ddActionObject*)date);
}


static ddActionValue
Date_getDate(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getDay(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getFullYear(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getHours(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getMilliseconds(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getMinutes(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getMonth(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getSeconds(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getTime(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getTimezoneOffset(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getUTCDate(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getUTCDay(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getUTCFullYear(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getUTCHours(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getUTCMilliseconds(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getUTCMinutes(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getUTCMonth(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getUTCSeconds(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_getYear(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_setDate(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_setFullYear(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_setHours(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_setMilliseconds(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_setMinutes(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_setMonth(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_setSeconds(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_setTime(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_setUTCDate(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_setUTCFullYear(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_setUTCHours(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_setUTCMilliseconds(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_setUTCMinutes(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_setUTCMonth(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_setUTCSeconds(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_setYear(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_toString(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


static ddActionValue
Date_UTC(ddActionObject* object, ddActionContext* context, int nargs)
{
	return ddNullValue;
}


ddActionFunction*
dd_initDateClass()
{
	ddActionFunction* ctor = dd_newActionNativeFunction(Date_constructor, FUNCTION_VARARGS);
	ddActionObject* classObject;
	
	ddActionDateClass = dd_newActionClass();
	classObject = (ddActionObject*)ddActionDateClass;
	
	ddActionObject_addNativeMethod(classObject, strdup("getDate"), Date_getDate, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getDay"), Date_getDay, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getFullYear"), Date_getFullYear, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getHours"), Date_getHours, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getMilliseconds"), Date_getMilliseconds, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getMinutes"), Date_getMinutes, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getMonth"), Date_getMonth, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getSeconds"), Date_getSeconds, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getTime"), Date_getTime, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getTimezoneOffset"), Date_getTimezoneOffset, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getUTCDate"), Date_getUTCDate, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getUTCDay"), Date_getUTCDay, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getUTCFullYear"), Date_getUTCFullYear, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getUTCHours"), Date_getUTCHours, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getUTCMilliseconds"), Date_getUTCMilliseconds, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getUTCMinutes"), Date_getUTCMinutes, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getUTCMonth"), Date_getUTCMonth, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getUTCSeconds"), Date_getUTCSeconds, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getYear"), Date_getYear, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setDate"), Date_setDate, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setFullYear"), Date_setFullYear, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setHours"), Date_setHours, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setMilliseconds"), Date_setMilliseconds, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setMinutes"), Date_setMinutes, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setMonth"), Date_setMonth, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setSeconds"), Date_setSeconds, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setTime"), Date_setTime, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setUTCDate"), Date_setUTCDate, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setUTCFullYear"), Date_setUTCFullYear, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setUTCHours"), Date_setUTCHours, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setUTCMilliseconds"), Date_setUTCMilliseconds, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setUTCMinutes"), Date_setUTCMinutes, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setUTCMonth"), Date_setUTCMonth, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setUTCSeconds"), Date_setUTCSeconds, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setYear"), Date_setYear, 0);
	ddActionObject_addNativeMethod(classObject, strdup("toString"), Date_toString, 0);
	ddActionObject_addNativeMethod(classObject, strdup("UTC"), Date_UTC, 0);
	
	ddActionFunction_setPrototype(ctor, classObject);
	
	return ctor;
}
