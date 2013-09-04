/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "function.h"
#include "value.h"
#include "classes/string_class.h"
#include "../player/drawtextfield.h"

const ddActionValue ddFalseValue = { VALUE_BOOLEAN, { 0 } };
const ddActionValue ddTrueValue = { VALUE_BOOLEAN, { 1 } };
const ddActionValue ddUndefValue = { VALUE_UNDEF, { 0 } };
const ddActionValue ddNullValue = { VALUE_NULL, { 0 } };
const ddActionValue ddSuperValue = { VALUE_SUPER, { 0 } };

#ifdef DD_INCLUDE_DEBUGGER
const ddActionValue ddBreakpointValue = { VALUE_BREAKPOINT, { 0 } };
#endif


ddActionValue
dd_newActionValue_int(int num)
{
	ddActionValue val;
	val.type = VALUE_INTEGER;
	val.data.intValue = num;
	return val;
}


ddActionValue
dd_newActionValue_double(double num)
{
	ddActionValue val;
	val.type = VALUE_DOUBLE;
	val.data.doubleValue = num;
	return val;
}


ddActionValue
dd_newActionValue_string(const char* string)
{
	ddActionValue val;
	val.type = VALUE_STRING;
	val.data.stringValue = strdup(string);
	return val;
}


ddActionValue
dd_newActionValue_string_noCopy(char* string)
{
	ddActionValue val;
	val.type = VALUE_STRING;
	val.data.stringValue = string;
	return val;
}


ddActionValue
dd_newActionValue_object(ddActionObject* object)
{
	ddActionValue val;

	if ( object == NULL )
		return ddNullValue;

	val.type = VALUE_OBJECT;
	val.data.objectValue = object;
	//ddActionObject_retain(object);
	
	return val;
}


ddActionValue
dd_newActionValue_function(ddActionFunction* function)
{
	ddActionValue val;
	val.type = VALUE_FUNCTION;
	val.data.functionValue = function;
	//ddActionObject_retain((ddActionObject*)function);
	return val;
}


ddActionValue
ddActionValue_copy(ddActionValue val)
{
	if ( val.type == VALUE_OBJECT || val.type == VALUE_FUNCTION )
		ddActionObject_retain(val.data.objectValue);

	if ( val.type == VALUE_STRING )
		val.data.stringValue = strdup(val.data.stringValue);

	return val;
}


void
ddActionValue_release(ddActionValue val)
{
	if ( val.type == VALUE_OBJECT || val.type == VALUE_FUNCTION )
		ddActionObject_release(val.data.objectValue);
	
	else if ( val.type == VALUE_STRING )
		dd_free(val.data.stringValue);
}


ddActionValue
ddActionValue_retain(ddActionValue val)
{
	if ( val.type == VALUE_OBJECT || val.type == VALUE_FUNCTION )
		ddActionObject_retain(val.data.objectValue);
	
	return val;
}


/* XXX - these should all call object's valueOf method */

ddActionFunction*
ddActionValue_getFunctionValue(ddActionValue value)
{
	if ( value.type == VALUE_FUNCTION )
	{
		// XXX - shouldn't retain!
		ddActionObject_retain(value.data.objectValue);
		return value.data.functionValue;
	}
	else
		return NULL;
}


ddActionObject*
ddActionValue_getObjectValue(ddActionValue value)
{
	if ( value.type == VALUE_OBJECT )
	{
		ddActionObject_retain(value.data.objectValue);
		return value.data.objectValue;
	}
	else if ( value.type == VALUE_FUNCTION )
	{
		ddActionObject_retain(value.data.objectValue);
		return (ddActionObject*)value.data.functionValue;
	}
	else if ( value.type == VALUE_STRING )
		return (ddActionObject*)dd_newActionString(strdup(value.data.stringValue));
	else
		return NULL;
}


double
ddActionValue_getDoubleValue(ddActionValue value)
{
	switch ( value.type )
	{
		case VALUE_DOUBLE:
			return value.data.doubleValue;

		case VALUE_INTEGER:
		case VALUE_BOOLEAN:
			return value.data.intValue;

		case VALUE_FUNCTION:
		case VALUE_OBJECT:
#ifdef NAN
			return NAN;
#else
			return nan(NULL);
#endif

		case VALUE_STRING:
		{
			char* ptr;
			double dnum = strtod(value.data.stringValue, &ptr);

			if ( ptr != value.data.stringValue )
				return dnum;

#ifdef NAN
			return NAN;
#else
			return nan(NULL);
#endif
		}
		
		case VALUE_NULL:
		case VALUE_UNDEF:
		default:
			return 0;
	}
}


int
ddActionValue_getIntValue(ddActionValue value)
{
	switch ( value.type )
	{
		case VALUE_DOUBLE:
			return (int)floor(value.data.doubleValue);

		case VALUE_INTEGER:
		case VALUE_BOOLEAN:
			return value.data.intValue;

		case VALUE_FUNCTION:
		case VALUE_OBJECT:
			return 0;   // XXX- NAN is a float

		case VALUE_STRING:
		{
			char* ptr;
			long lnum = strtol(value.data.stringValue, &ptr, 0);

			if ( lnum != 0 || errno == 0 )
				return lnum;

			return 0;
		}

		case VALUE_NULL:
		case VALUE_UNDEF: /* XXX ??? */
		default:
			return 0;   // XXX- NAN is a float
	}
}


char*
ddActionValue_getStringValue(ddActionContext* context, ddActionValue value)
{
	switch ( value.type )
	{
		case VALUE_STRING:
			return strdup(value.data.stringValue);

		case VALUE_DOUBLE:
		{
			if ( !finite(value.data.doubleValue) )
				return strdup("Infinity");
			else if ( isnan(value.data.doubleValue) )
				return strdup("NaN");
			else
			{
				char* str = dd_malloc(32); /* XXX - ??? */
				int count = sprintf(str, "%f", value.data.doubleValue) - 1;

				/* strip trailing zeroes- lame, but that's how they do it. */

				while ( str[count] == '0' )
					str[count--] = '\0';
				
				if ( str[count] == '.' )
					str[count] = '\0';
				
				return str;
			}
		}

		case VALUE_INTEGER:
		{
			char* str = dd_malloc(32); /* XXX - ??? */
			sprintf(str, "%i", value.data.intValue);
			return str;
		}

		case VALUE_NULL:
			return strdup("null");

		case VALUE_UNDEF:
			return strdup("undefined");

		case VALUE_BOOLEAN:
		{
			if ( value.data.intValue == 0 )
				return strdup("false");
			else
				return strdup("true");
		}

		case VALUE_OBJECT:
		case VALUE_FUNCTION:
		{
			ddActionValue val;
			char* str = NULL;

			/* XXX - can cause recursion, I'd guess */

			if ( context != NULL )
			{
				val = ddActionObject_runMethod(context->player, value.data.objectValue, "toString", 0, NULL);

				if ( val.type == VALUE_STRING && val.data.stringValue != NULL )
					str = strdup(val.data.stringValue);
			}

			if ( str == NULL )
			{
				if ( value.type == VALUE_FUNCTION )
					str = strdup("[type Function]");
				else
					str = strdup("[type Object]");
			}

			ddActionValue_release(val);

			return str;
		}
		
		case VALUE_TEXTFIELD:
		{
			const char* text = ddDrawTextField_getText(value.data.textFieldValue);
			char* str;
			
			if ( text == NULL )
				return strdup("");

			str = malloc(strlen(text) + strlen("textfield: ''") + 1);
			sprintf(str, "textfield: '%s'", text);
			return str;
		}
		
		default:
			return strdup("[type unknown]"); /* XXX */
	}
}


char*
ddActionValue_getStringValue_release(ddActionContext* context, ddActionValue value)
{
	char* str;
	
	// If we're going to release the value and it's a string, don't bother making a copy

	if ( value.type == VALUE_STRING )
		return value.data.stringValue;

	str = ddActionValue_getStringValue(context, value);
	ddActionValue_release(value);
	return str;
}


int
ddActionValue_getBooleanValue(ddActionValue value)
{
	switch ( value.type )
	{
		case VALUE_BOOLEAN:
		case VALUE_INTEGER:
			return (value.data.intValue != 0);

		case VALUE_DOUBLE:
			return (value.data.doubleValue != 0.0);

		case VALUE_STRING:
		{
			char* str = value.data.stringValue;
			return (str != NULL && str[0] != '\0');
		}
		case VALUE_OBJECT:
			return (value.data.objectValue != NULL); /* XXX */

		case VALUE_FUNCTION:
			return (value.data.functionValue != NULL); /* XXX */

		case VALUE_NULL:
		case VALUE_UNDEF: /* XXX ??? */

		default:
			return 0;
	}
}


const char*
ddActionValue_getTypeName(ddActionValue a)
{
	switch ( a.type )
	{
		case VALUE_NULL:        return "null";
		case VALUE_UNDEF:       return "undefined";
		case VALUE_BOOLEAN:     return "boolean";
		case VALUE_STRING:      return "string";
		case VALUE_INTEGER:
		case VALUE_DOUBLE:      return "number";
		case VALUE_FUNCTION:    return "function";
		case VALUE_OBJECT:      return "object";
		default:                return "unknown type";
	}
}


ddActionValue
ddActionValue_increment(ddActionValue val)
{
	if ( val.type != VALUE_INTEGER )
	{
		val.data.doubleValue = ddActionValue_getDoubleValue(val) + 1.0;
		val.type = VALUE_DOUBLE;
	}
	else
		++val.data.intValue;

	return val;
}


ddActionValue
ddActionValue_decrement(ddActionValue val)
{
	if ( val.type != VALUE_INTEGER )
	{
		val.data.doubleValue = ddActionValue_getDoubleValue(val) - 1.0;
		val.type = VALUE_DOUBLE;
	}
	else
		--val.data.intValue;

	return val;
}
