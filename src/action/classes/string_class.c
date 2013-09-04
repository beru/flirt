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

#include "string_class.h"

#include "../function.h"
#include "array_class.h"

ddActionClass* ddActionStringClass = NULL;


static ddActionValue
ddActionString_getProperty(ddActionContext* context, ddActionObject* object, const char* name)
{
	ddActionString* string = (ddActionString*)object;
	
	if ( strcasecmp(name, "length") == 0 )
		return dd_newActionValue_int(string->length);

	return ddActionObject_getProperty_default(context, object, name);
}


static void
dd_destroyActionString(ddActionObject* object)
{
	ddActionString* strobj = (ddActionString*)object;
	dd_free(strobj->string);
}


/* string alloced from outside, destroyed inside */

ddActionString*
dd_newActionString(char* string)
{
	ddActionString* strobj = dd_malloc(sizeof(struct _ddActionString));
	ddActionObject_init((ddActionObject*)strobj);

	ddActionObject_setClass((ddActionObject*)strobj, ddActionStringClass);

	strobj->string = string;
	strobj->length = strlen(string);

	return strobj;
}


ddActionValue
String_constructor(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);

	ddActionString* string = dd_newActionString(ddActionValue_getStringValue(context, val));

	ddActionValue_release(val);

	return dd_newActionValue_object((ddActionObject*)string);
}


static ddActionValue
String_charAt(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionString* string = (ddActionString*)object;
	
	char* str;
	char* charat;
	ddActionValue val;
	int pos;
	
	if ( !ddActionObject_isKindOfClass(object, ddActionStringClass) )
		return ddUndefValue;

	str = string->string;
	val = ddActionStack_pop(&context->stack);
	pos = ddActionValue_getIntValue(val);

	if ( pos < 0 || pos >= string->length )
		return ddNullValue;

	charat = dd_malloc(2*sizeof(char));
	charat[0] = str[pos];
	charat[1] = '\0';

	return dd_newActionValue_string(charat);
}


static ddActionValue
String_charCodeAt(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionString* string = (ddActionString*)object;
	
	char* str;
	ddActionValue val;
	int pos;
	
	if ( !ddActionObject_isKindOfClass(object, ddActionStringClass) )
		return ddUndefValue;

	str = string->string;
	val = ddActionStack_pop(&context->stack);
	pos = ddActionValue_getIntValue(val);

	if ( pos < 0 || pos >= string->length )
		return ddNullValue;

	return dd_newActionValue_int(str[pos]);
}


static ddActionValue
String_concat(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionString* string = (ddActionString*)object;
	char* str;
	int len;
	
	if ( !ddActionObject_isKindOfClass(object, ddActionStringClass) )
		return ddUndefValue;
	
	str = strdup(string->string);
	len = string->length;

	while ( --nargs >= 0 )
	{
		ddActionValue val = ddActionStack_pop(&context->stack);
		char* catstr = ddActionValue_getStringValue(context, val);
		int catlen = strlen(catstr);

		str = dd_realloc(str, len + catlen + 1);
		strncpy(str + len, catstr, catlen);
		str[len + catlen] = '\0';

		ddActionValue_release(val);

		len += catlen;
	}

	return dd_newActionValue_string(str);
}


static ddActionValue
String_fromCharCode(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);
	int ival = ddActionValue_getIntValue(val);
	char* charat = dd_malloc(2 * sizeof(char));

	charat[0] = ival & 0xff;
	charat[1] = '\0';

	return dd_newActionValue_string(charat);
}


static ddActionValue
String_indexOf(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionString* string = (ddActionString*)object;
	char* str;
	
	ddActionValue val;
	char* findstr;
	int findlen;
	int len;
	int i;
	
	if ( !ddActionObject_isKindOfClass(object, ddActionStringClass) )
		return ddUndefValue;
	
	str = string->string;

	val = ddActionStack_pop(&context->stack);
	findstr = ddActionValue_getStringValue(context, val);
	findlen = strlen(findstr);

	len = string->length - findlen + 1;

	for ( i = 0; i < len; ++i )
	{
		if ( strncmp(str + i, findstr, findlen) == 0 )
			break;
	}

	ddActionValue_release(val);

	if ( i == len )
		i = -1;

	return dd_newActionValue_int(i);
}


static ddActionValue
String_lastIndexOf(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionString* string = (ddActionString*)object;

	char* str;
	int len;
	int i;

	ddActionValue val;
	char* findstr;
	int findlen;
	
	if ( !ddActionObject_isKindOfClass(object, ddActionStringClass) )
		return ddUndefValue;
		
	str = string->string;
	len = string->length;

	val = ddActionStack_pop(&context->stack);
	findstr = ddActionValue_getStringValue(context, val);
	findlen = strlen(findstr);

	for ( i = len - findlen; i >= 0; --i )
	{
		if ( strncmp(str + i, findstr, findlen) == 0 )
			break;
	}

	ddActionValue_release(val);

	return dd_newActionValue_int(i);
}


static char*
substr(char* str, int from, int to)
{
	char* sub = dd_malloc(sizeof(char) * (to - from + 1));
	strncpy(sub, str + from, to - from);
	sub[to - from] = '\0';
	return sub;
}


static ddActionValue
String_slice(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionString* string = (ddActionString*)object;
	ddActionValue val1;
	int start, end;

	if ( !ddActionObject_isKindOfClass(object, ddActionStringClass) )
		return ddUndefValue;
	
	if ( nargs == 0 )
		return ddNullValue;

	val1 = ddActionStack_pop(&context->stack);
	--nargs;

	start = ddActionValue_getIntValue(val1);
	ddActionValue_release(val1);

	if ( nargs > 0 )
	{
		ddActionValue val2 = ddActionStack_pop(&context->stack);
		--nargs;

		end = ddActionValue_getIntValue(val2);
		ddActionValue_release(val2);

		if ( end < 0 )
			end += string->length;
		else if ( end > string->length )
			end = string->length;
	}
	else
		end = string->length;

	while ( --nargs >= 0 )
		ddActionValue_release(ddActionStack_pop(&context->stack));

	if ( end <= 0 || start >= end )
		return ddNullValue;

	return dd_newActionValue_string(substr(string->string, start, end));
}


static ddActionValue
String_split(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionString* string = (ddActionString*)object;

	ddActionArray* array;
	char* str;

	char* findstr;
	int findlen;

	char c, *p;
	int i;

	if ( !ddActionObject_isKindOfClass(object, ddActionStringClass) )
		return ddUndefValue;

	array = dd_newActionArray();
	str = string->string;

	if ( nargs > 0 )
	{
		ddActionValue val = ddActionStack_pop(&context->stack);
		findstr = ddActionValue_getStringValue(context, val);
		findlen = strlen(findstr);

		while ( --nargs > 0 )
			ddActionValue_release(ddActionStack_pop(&context->stack));
	}
	else
	{
		findstr = strdup(",");
		findlen = 1;
	}

	for ( i = 0; ; ++i )
	{
		p = strstr(str, findstr);

		if ( p == NULL )
		{
			ddActionArray_setMember(array, i, dd_newActionValue_string(strdup(str)));
			break;
		}

		c = *p;
		*p = '\0';
		ddActionArray_setMember(array, i, dd_newActionValue_string(strdup(str)));
		*p = c;

		str = p + findlen;
	}

	dd_free(findstr);

	return dd_newActionValue_object((ddActionObject*)array);
}


static ddActionValue
String_substr(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionString* string = (ddActionString*)object;
	ddActionValue val1;
	int start, length;

	if ( !ddActionObject_isKindOfClass(object, ddActionStringClass) )
		return ddUndefValue;
	
	if ( nargs == 0 )
		return ddNullValue;

	val1 = ddActionStack_pop(&context->stack);
	--nargs;

	start = ddActionValue_getIntValue(val1);
	ddActionValue_release(val1);

	if (start < 0 )
	{
		start = string->length - start;

		if ( start < 0 )
			start = 0;
	}

	if ( nargs > 0 )
	{
		ddActionValue val2 = ddActionStack_pop(&context->stack);
		--nargs;

		length = ddActionValue_getIntValue(val2);
		ddActionValue_release(val2);
	}
	else
		length = string->length - start;

	while ( --nargs >= 0 )
		ddActionValue_release(ddActionStack_pop(&context->stack));

	if ( length <= 0 || start >= string->length )
		return ddNullValue;

	return dd_newActionValue_string(substr(string->string, start, start+length));
}


static ddActionValue
String_substring(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionString* string = (ddActionString*)object;
	ddActionValue val1;
	int start, end;
	
	if ( !ddActionObject_isKindOfClass(object, ddActionStringClass) )
		return ddUndefValue;

	if ( nargs == 0 )
		return ddNullValue;

	val1 = ddActionStack_pop(&context->stack);
	--nargs;

	start = ddActionValue_getIntValue(val1);
	ddActionValue_release(val1);

	if ( nargs > 0 )
	{
		ddActionValue val2 = ddActionStack_pop(&context->stack);
		--nargs;

		end = ddActionValue_getIntValue(val2);
		ddActionValue_release(val2);
	}
	else
		end = string->length;

	if ( start < 0 )
		start = 0;

	if ( end > string->length )
		end = string->length;

	if ( start >= end )
	{
		int tmp = start;
		start = end;
		end = tmp;
	}

	while ( --nargs >= 0 )
		ddActionValue_release(ddActionStack_pop(&context->stack));

	return dd_newActionValue_string(substr(string->string, start, end));
}


static ddActionValue
String_toLowerCase(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionString* string = (ddActionString*)object;
	char* str;
	int i;
	
	if ( !ddActionObject_isKindOfClass(object, ddActionStringClass) )
		return ddUndefValue;

	str = strdup(string->string);
	
	for ( i = 0; str[i] != '\0'; ++i )
		str[i] = tolower(str[i]);

	return dd_newActionValue_string(str);
}


static ddActionValue
String_toUpperCase(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionString* string = (ddActionString*)object;
	char* str;
	int i;

	if ( !ddActionObject_isKindOfClass(object, ddActionStringClass) )
		return ddUndefValue;
	
	str = strdup(string->string);

	for ( i = 0; str[i] != '\0'; ++i )
		str[i] = toupper(str[i]);

	return dd_newActionValue_string(str);
}


static ddActionValue
String_toString(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionString* string = (ddActionString*)object;

	if ( !ddActionObject_isKindOfClass(object, ddActionStringClass) )
		return ddUndefValue;
	
	return dd_newActionValue_string(string->string);
}


static ddActionValue
String_valueOf(ddActionObject* object, ddActionContext* context, int nargs)
{
	return String_toString(object, context, nargs);
}


ddActionFunction*
dd_initStringClass()
{
	ddActionObject* classObject;
	ddActionFunction* ctor = dd_newActionNativeFunction(String_constructor, 1);

	ddActionStringClass = dd_newActionClass();
	classObject = (ddActionObject*)ddActionStringClass;
 
	ddActionStringClass->getProperty = ddActionString_getProperty;
	ddActionStringClass->destroy = dd_destroyActionString;
	
	ddActionObject_addNativeMethod(classObject, strdup("charAt"), String_charAt, 1);
	ddActionObject_addNativeMethod(classObject, strdup("charCodeAt"), String_charCodeAt, 1);
	ddActionObject_addNativeMethod(classObject, strdup("concat"), String_concat, FUNCTION_VARARGS);
	ddActionObject_addNativeMethod((ddActionObject*)ctor, strdup("fromCharCode"), String_fromCharCode, 1);
	ddActionObject_addNativeMethod(classObject, strdup("indexOf"), String_indexOf, 1);
	ddActionObject_addNativeMethod(classObject, strdup("lastIndexOf"), String_lastIndexOf, 1);
	ddActionObject_addNativeMethod(classObject, strdup("slice"), String_slice, FUNCTION_VARARGS);
	ddActionObject_addNativeMethod(classObject, strdup("split"), String_split, FUNCTION_VARARGS);
	ddActionObject_addNativeMethod(classObject, strdup("substr"), String_substr, FUNCTION_VARARGS);
	ddActionObject_addNativeMethod(classObject, strdup("substring"), String_substring, FUNCTION_VARARGS);
	ddActionObject_addNativeMethod(classObject, strdup("toLowerCase"), String_toLowerCase, 1);
	ddActionObject_addNativeMethod(classObject, strdup("toUpperCase"), String_toUpperCase, 1);
	ddActionObject_addNativeMethod(classObject, strdup("toString"), String_toString, 0);
	ddActionObject_addNativeMethod(classObject, strdup("valueOf"), String_valueOf, 0);

	ddActionFunction_setPrototype(ctor, classObject);

	return ctor;
}
