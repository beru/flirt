/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <stdio.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <unistd.h>

#include <math.h>

#include "opcodes.h"
#include "context.h"

#include "classes/array_class.h"
#include "classes/movieclip_class.h"
#include "classes/string_class.h"


ddActionOpcode*
ddAction_newOpcode(ddAction* action)
{
	ddActionOpcode* op;
	
	action->opcodes = dd_realloc(action->opcodes, (action->length + 1) * sizeof(ddActionOpcode));

	op = &action->opcodes[action->length++];

	op->op = ACTION_END;
	op->arg = NULL;
	op->val = ddNullValue;
	
#ifdef DD_INCLUDE_DEBUGGER
	op->breakpoint = DD_FALSE;
#endif
	
	return op;
}


void
dd_destroyActionOpcode(ddActionOpcode op)
{
	if ( op.arg != NULL )
		dd_free(op.arg);

	ddActionValue_release(op.val);
}


static void
popContext(ddActionContext** contextPtr)
{
	ddActionContext* oldcontext = *contextPtr;

	if ( oldcontext == NULL )
	{
		dd_warn("popped empty context");
		return;
	}

	*contextPtr = oldcontext->next;
	dd_destroyActionContext(oldcontext);
}


ddActionValue
getSlashPath(ddActionContext* context, const char* str)
{
	ddActionObject* clip;
	ddActionValue val = ddNullValue;

	// XXX - needs to parse dot paths, too

	if ( str[0] == '/' )
	{
		clip = (ddActionObject*)ddPlayer_getRootClip(context->player);
		val = dd_newActionValue_object(clip);
		++str;
	}
	else
	{
		clip = context->object;
		val = dd_newActionValue_object(clip);
	}
	
	while ( *str != '\0' && clip != NULL )
	{
		char* p = strchr(str, '/');
		
		if ( p != NULL )
		{
			char c = *p;
			*p = '\0';
			val = ddActionObject_getProperty(context, clip, str);
			str = p + 1;
			*p = c;
		}
		else
		{
			p = strchr(str, ':');

			if ( p != NULL )
			{
				ddActionObject* object;

				if ( p != str )
				{
					char c = *p;
					*p = '\0';
					val = ddActionObject_getProperty(context, clip, str);
					object = ddActionValue_getObjectValue(val);
					*p = c;

					if ( object != NULL )
					{
						val = ddActionObject_getProperty(context, object, p + 1);
						ddActionObject_release(object);
					}
				}
				else
					val = ddActionObject_getProperty(context, clip, p + 1);
			}
			else
				val = ddActionObject_getProperty(context, clip, str);
			
			break;
		}

		if ( val.type == VALUE_OBJECT )
			clip = ddActionValue_getObjectValue(val);
	}

	return val;
}


void
setSlashPath(ddActionContext* context, char* inStr, ddActionValue setval)
{
	char* str = inStr;
	ddActionObject* clip;
	ddActionValue val;

	if ( str[0] == '/' )
	{
		clip = (ddActionObject*)ddPlayer_getRootClip(context->player);
		++str;
	}
	else
		clip = context->object;

	while ( *str != '\0' && clip != NULL )
	{
		char* p = strchr(str, '/');

		if ( p != NULL )
		{
			*p = '\0';
			val = ddActionObject_getProperty(context, clip, str);
			str = p + 1;
		}
		else
		{
			p = strchr(str, ':');

			if ( p != NULL )
			{
				ddActionObject* object;

				if ( p != str )
				{
					*p = '\0';
					val = ddActionObject_getProperty(context, clip, str);
					object = ddActionValue_getObjectValue(val);
				}
				else
					object = clip;

				if ( object != NULL )
					ddActionObject_setProperty(context, object, strdup(p + 1), setval);
			}
			else
				dd_warn("setSlashPath found no :");

			break;
		}

		if ( val.type == VALUE_OBJECT )
			clip = ddActionValue_getObjectValue(val);
	}

	dd_free(inStr);
}


#define push(v) ddActionStack_push(&context->stack, (v))
#define pop() ddActionStack_pop(&context->stack)

static inline void
action_getURL(ddActionContext* context, ddActionOpcode* op)
{
	/* arg: string url, string window */
	// XXX - should deal with bad formatting
	char* arg1 = op->arg;
	char* arg2 = op->val.data.stringValue;
	dd_warn("GetURL action (\"%s\", \"%s\") not implemented", arg1, arg2);
}


static inline void
action_waitForFrame(ddPlayer* player, ddActionContext* context, ddActionOpcode* op)
{
	/* arg: short frame to wait for, int8 # of actions to skip */
	int frame = op->val.data.intValue;
	
	// XXX - make sure context object is a movie clip
	int framesLoaded =
		ddDrawClip_getFramesLoaded(((ddActionMovieClip*)context->object)->movieClip);

	if ( framesLoaded < frame )
		context->actionSkip = op->data;
}


static inline void
action_setTarget(ddActionContext* context, ddActionOpcode* op)
{
	/* find named target, set context->object */

	if ( op->arg[0] != '\0' )
	{
		ddActionValue val = getSlashPath(context, op->arg);
		ddActionObject* target = ddActionValue_getObjectValue(val);

		if ( target != NULL )
			context->object = target;
	}
	else
		context->object = context->clip;
}


static inline void
action_add(ddActionContext* context)
{
	ddActionValue aval = pop();
	ddActionValue bval = pop();
	double a = ddActionValue_getDoubleValue(aval);
	double b = ddActionValue_getDoubleValue(bval);

	push(dd_newActionValue_double(b+a));

	ddActionValue_release(aval);
	ddActionValue_release(bval);	
}


static inline void
action_subtract(ddActionContext* context)
{
	ddActionValue aval = pop();
	ddActionValue bval = pop();
	double a = ddActionValue_getDoubleValue(aval);
	double b = ddActionValue_getDoubleValue(bval);

	push(dd_newActionValue_double(b - a));

	ddActionValue_release(aval);
	ddActionValue_release(bval);
}


static inline void
action_subtract_int(ddActionContext* context, int num)
{
	ddActionValue val = pop();

	if ( val.type == VALUE_INTEGER )
	{
		val.data.intValue -= num;
		push(val);
	}
	else
	{
		double d = ddActionValue_getDoubleValue(val);
		push(dd_newActionValue_double(d - num));
		ddActionValue_release(val);
	}
}


static inline void
action_subtract_double(ddActionContext* context, double num)
{
	ddActionValue val = pop();
	double d = ddActionValue_getDoubleValue(val);
	push(dd_newActionValue_double(d - num));

	ddActionValue_release(val);
}


static inline void
action_multiply(ddActionContext* context)
{
	ddActionValue aval = pop();
	ddActionValue bval = pop();
	double a = ddActionValue_getDoubleValue(aval);
	double b = ddActionValue_getDoubleValue(bval);

	push(dd_newActionValue_double(b * a));

	ddActionValue_release(aval);
	ddActionValue_release(bval);	
}


static inline void
action_multiply_int(ddActionContext* context, int num)
{
	ddActionValue val = pop();

	if ( val.type == VALUE_INTEGER )
	{
		val.data.intValue *= num;
		push(val);
	}
	else
	{
		double d = ddActionValue_getDoubleValue(val);
		push(dd_newActionValue_double(d * num));
		ddActionValue_release(val);
	}
}


static inline void
action_multiply_double(ddActionContext* context, double num)
{
	ddActionValue val = pop();
	double d = ddActionValue_getDoubleValue(val);
	push(dd_newActionValue_double(d * num));

	ddActionValue_release(val);
}


static inline void
action_divide(ddActionContext* context)
{
	ddActionValue aval = pop();
	ddActionValue bval = pop();
	double a = ddActionValue_getDoubleValue(aval);
	double b = ddActionValue_getDoubleValue(bval);

	push(dd_newActionValue_double(b/a));

	ddActionValue_release(aval);
	ddActionValue_release(bval);
}


static inline void
action_divide_int(ddActionContext* context, int num)
{
	ddActionValue val = pop();

	// XXX - modulo probably doesn't work right with negative num
	if ( val.type == VALUE_INTEGER && (val.data.intValue % num) == 0 )
	{
		val.data.intValue /= num;
		push(val);
	}
	else
	{
		double d = ddActionValue_getDoubleValue(val);
		push(dd_newActionValue_double(d / num));
		ddActionValue_release(val);
	}
}


static inline void
action_divide_double(ddActionContext* context, double num)
{
	ddActionValue val = pop();
	double d = ddActionValue_getDoubleValue(val);
	push(dd_newActionValue_double(d / num));

	ddActionValue_release(val);
}


static inline void
action_equal(ddActionContext* context)
{
	ddActionValue val1 = pop();
	ddActionValue val2 = pop();
	double a = ddActionValue_getDoubleValue(val1);
	double b = ddActionValue_getDoubleValue(val2);

	if ( a == b )
		push(ddTrueValue);
	else
		push(ddFalseValue);


	ddActionValue_release(val1);
	ddActionValue_release(val2);	
}


static inline void
action_lessThan(ddActionContext* context)
{
	ddActionValue val1 = pop();
	ddActionValue val2 = pop();
	float a = ddActionValue_getDoubleValue(val1);
	float b = ddActionValue_getDoubleValue(val2);

	if ( a < b )
		push(ddTrueValue);
	else
		push(ddFalseValue);

	ddActionValue_release(val1);
	ddActionValue_release(val2);
}


static inline void
action_logicalOr(ddActionContext* context)
{
	ddActionValue val1 = pop();
	ddActionValue val2 = pop();
	int a = ddActionValue_getBooleanValue(val1);
	int b = ddActionValue_getBooleanValue(val2);

	if ( a || b )
		push(ddTrueValue);
	else
		push(ddFalseValue);

	ddActionValue_release(val1);
	ddActionValue_release(val2);
}


static inline void
action_logicalAnd(ddActionContext* context)
{
	ddActionValue val1 = pop();
	ddActionValue val2 = pop();
	int a = ddActionValue_getBooleanValue(val1);
	int b = ddActionValue_getBooleanValue(val2);

	if ( a && b )
		push(ddTrueValue);
	else
		push(ddFalseValue);

	ddActionValue_release(val1);
	ddActionValue_release(val2);
}


static inline void
action_logicalNot(ddActionContext* context)
{
	ddActionValue val = pop();
	int a = ddActionValue_getBooleanValue(val);

	if ( a == 0 )
		push(ddTrueValue);
	else
		push(ddFalseValue);

	ddActionValue_release(val);
}


static inline void
action_stringEq(ddActionContext* context)
{
	ddActionValue val1 = pop();
	ddActionValue val2 = pop();

	char* a = ddActionValue_getStringValue_release(context, val1);
	char* b = ddActionValue_getStringValue_release(context, val2);

	if ( strcmp(a, b) == 0 )
		push(ddTrueValue);
	else
		push(ddFalseValue);

	dd_free(a);
	dd_free(b);
}


static inline void
action_stringLength(ddActionContext* context)
{
	ddActionValue val = pop();

	if ( val.type == VALUE_OBJECT && val.data.objectValue->nativeClass == ddActionStringClass )
	{
		/* shortcut for string object */
		ddActionString* string = (ddActionString*)val.data.objectValue;
		push(dd_newActionValue_int(string->length));
		ddActionValue_release(val);
	}
	else
	{
		char* a = ddActionValue_getStringValue_release(context, val);
		push(dd_newActionValue_int(strlen(a)));
		dd_free(a);
	}
}


static inline void
action_substring(ddActionContext* context)
{
	ddActionValue lenval = pop();
	ddActionValue startval = pop();

	char* str = ddActionValue_getStringValue_release(context, pop());
	int slen = strlen(str);
	int start = ddActionValue_getIntValue(startval);
	int len = ddActionValue_getIntValue(lenval);

	if ( len > 0 && start < slen )
	{
		char* res;

		if ( start + len > slen )
			len = slen - start;
		
		res = dd_malloc(len + 1);
		strncpy(res, str + start, len);
		res[len] = '\0';

		push(dd_newActionValue_string_noCopy(res));
	}
	else
		push(ddNullValue);

	dd_free(str);

	ddActionValue_release(startval);
	ddActionValue_release(lenval);
}


static inline void
action_int(ddActionContext* context)
{
	ddActionValue val = pop();
	push(dd_newActionValue_int(ddActionValue_getIntValue(val)));
	ddActionValue_release(val);
}


static inline void
action_getNamedVariable(ddActionContext* context, const char* name)
{
	ddActionValue val;

	if ( strchr(name, '/') || strchr(name, ':') )
		val = getSlashPath(context, name);
	else
	{
		val = ddActionObject_getProperty(context, context->object, name);

		if ( val.type == VALUE_UNDEF )
			val = ddActionObject_getProperty(context, context->player->globals, name);
	}

	push(ddActionValue_copy(val));
}


static inline void
action_getVariable(ddActionContext* context)
{
	char* name = ddActionValue_getStringValue_release(context, pop());
	action_getNamedVariable(context, name);
	dd_free(name);
}


static inline void
action_setVariable(ddActionContext* context)
{
	ddActionValue val = pop();
	char* name = ddActionValue_getStringValue_release(context, pop());

	if ( strchr(name, '/') || strchr(name, ':') )
		setSlashPath(context, name, val);
	else
		ddActionObject_setProperty(context, context->object, name, val);
}


static inline void
action_setNamedVariable(ddActionContext* context, char* name, ddActionValue val)
{
	if ( strchr(name, '/') || strchr(name, ':') )
		setSlashPath(context, name, val);
	else
		ddActionObject_setProperty(context, context->object, name, val);
}


static inline void
action_setTargetExpression(ddActionContext* context)
{
	ddActionValue targetVal;
	ddActionObject* target;
	char* name = ddActionValue_getStringValue_release(context, pop());

	targetVal = getSlashPath(context, name);
	target = ddActionValue_getObjectValue(targetVal);

	if ( target != NULL )
		context->object = target;

	dd_free(name);

	//dd_warn("setTargetExpression action might be broken..");
}


static inline void
action_stringConcat(ddActionContext* context)
{
	char* str2 = ddActionValue_getStringValue_release(context, pop());
	char* str1 = ddActionValue_getStringValue_release(context, pop());

	str1 = dd_realloc(str1, strlen(str1) + strlen(str2) + 1);
	strcat(str1, str2);

	push(dd_newActionValue_string_noCopy(str1));

	dd_free(str2);
}


static inline void
action_getProperty(ddActionContext* context)
{
	ddActionValue val = pop();
	ddActionValue target = pop();

	ddActionObject* targetObj;

	if ( target.type != VALUE_OBJECT )
	{
		char* tmp = ddActionValue_getStringValue_release(context, target);
		targetObj = ddActionValue_getObjectValue(getSlashPath(context, tmp));
		dd_free(tmp);
	}
	else
	{
		targetObj = ddActionValue_getObjectValue(target);
		ddActionValue_release(target);
	}

	if ( targetObj == NULL )
		targetObj = context->object;

	if ( targetObj != NULL && targetObj->proto == (ddActionObject*)ddActionMovieClipClass )
	{
		push(ddActionMovieClip_getPropertyNum(context->player,
										(ddActionMovieClip*)targetObj,
										ddActionValue_getIntValue(val)));
	}
	else
		push(ddNullValue);

	ddActionValue_release(val);
}


static inline void
action_setProperty(ddActionContext* context)
{
	ddActionValue value = pop();
	ddActionValue prop = pop();
	ddActionValue target = pop();

	ddActionObject* targetObj;
	
	if ( target.type != VALUE_OBJECT )
	{
		char* tmp = ddActionValue_getStringValue_release(context, target);
		targetObj = ddActionValue_getObjectValue(getSlashPath(context, tmp));
		dd_free(tmp);
	}
	else
	{
		targetObj = ddActionValue_getObjectValue(target);
		ddActionValue_release(target);
	}
	
	if ( targetObj == NULL )
		targetObj = context->object;

	if ( targetObj != NULL && targetObj->proto == (ddActionObject*)ddActionMovieClipClass )
	{
		ddActionMovieClip_setPropertyNum((ddActionMovieClip*)targetObj,
								   ddActionValue_getIntValue(prop),
								   value);
	}
	
	ddActionValue_release(prop);
	ddActionValue_release(value);
}


static inline void
action_trace(ddActionContext* context)
{
	char* str = ddActionValue_getStringValue_release(context, pop());
	ddPlayer* player = context->player;
	
	if ( player->traceFunction != NULL )
		player->traceFunction(str, player->traceUserData);

	dd_free(str);
}


static inline void
action_stringCompare(ddActionContext* context)
{
	char* a = ddActionValue_getStringValue_release(context, pop());
	char* b = ddActionValue_getStringValue_release(context, pop());

	if ( strcmp(a, b) < 0 )
		push(ddTrueValue);
	else
		push(ddFalseValue);

	dd_free(a);
	dd_free(b);
}


static inline void
action_random(ddActionContext* context)
{
	ddActionValue a = pop();
	int num = ddActionValue_getIntValue(a);

	push(dd_newActionValue_int(rand() % num));

	ddActionValue_release(a);
}


static inline void
action_with(ddActionContext** contextptr, ddActionOpcode* op)
{
	ddActionContext* context = *contextptr;
	int size = op->val.data.intValue;
	ddActionOpcode* instrPtr = context->instrPtr;

	ddActionValue objval = pop();
	ddActionObject* object = ddActionValue_getObjectValue(objval);

	context->instrPtr = instrPtr + size;
	ddActionContext_pushValueContext(contextptr, object);
	context = *contextptr;
	context->instrPtr = instrPtr;
	context->endPtr = instrPtr + size;

#ifdef DD_INCLUDE_DEBUGGER
	ddActionContext_setLabel(context, "<with>");
#endif
	
	ddActionValue_release(objval);
	ddActionObject_release(object);
}


static inline void
action_shiftRight2(ddActionContext* context)
{
	ddActionValue a = pop();
	ddActionValue b = pop();

	unsigned long anum = ddActionValue_getIntValue(a);
	int bnum = ddActionValue_getIntValue(b);

	push(dd_newActionValue_double(bnum >> anum));

	ddActionValue_release(a);
	ddActionValue_release(b);
}


static inline void
action_shiftRight(ddActionContext* context)
{
	ddActionValue a = pop();
	ddActionValue b = pop();

	int anum = ddActionValue_getIntValue(a);
	int bnum = ddActionValue_getIntValue(b);

	push(dd_newActionValue_int(bnum >> anum));

	ddActionValue_release(a);
	ddActionValue_release(b);
}


static inline void
action_shiftRight_int(ddActionContext* context, int shift)
{
	ddActionValue val = pop();
	int num = ddActionValue_getIntValue(val);

	push(dd_newActionValue_int(num >> shift));

	ddActionValue_release(val);
}


static inline void
action_shiftLeft(ddActionContext* context)
{
	ddActionValue a = pop();
	ddActionValue b = pop();

	int anum = ddActionValue_getIntValue(a);
	int bnum = ddActionValue_getIntValue(b);

	push(dd_newActionValue_int(bnum << anum));

	ddActionValue_release(a);
	ddActionValue_release(b);
}


static inline void
action_shiftLeft_int(ddActionContext* context, int shift)
{
	ddActionValue val = pop();
	int num = ddActionValue_getIntValue(val);

	push(dd_newActionValue_int(num << shift));

	ddActionValue_release(val);
}


static inline void
action_enumerate(ddActionContext* context)
{
	/* for(x in a) - copy all x in a to stack */
	ddActionValue var;
	char* name = ddActionValue_getStringValue_release(context, pop());

	var = ddActionObject_getProperty(context, context->object, name);

	dd_free(name);

	/* first there's a null so that we know where to stop */
	push(ddNullValue);

	if ( var.type == VALUE_OBJECT)
		ddActionHash_pushKeys(var.data.objectValue->properties, &context->stack);

	ddActionValue_release(var);
}


static inline void
action_enumerate2(ddActionContext* context)
{
	/* for(x in a) - copy all x in a to stack */
	ddActionValue var = pop();

	/* first there's a null so that we know where to stop */
	push(ddNullValue);

	if ( var.type == VALUE_OBJECT)
		ddActionHash_pushKeys(var.data.objectValue->properties, &context->stack);

	ddActionValue_release(var);
}


static inline void
action_setRegister(ddActionContext* context, ddActionOpcode* op)
{
	unsigned char num = op->data;

	ddActionValue_release(context->player->registers[num]);

	context->player->registers[num] = ddActionValue_retain(ddActionValue_copy(ddActionStack_top(&context->stack)));
}


static inline void
action_bitwiseXor(ddActionContext* context)
{
	ddActionValue a = pop();
	ddActionValue b = pop();

	int anum = ddActionValue_getIntValue(a);
	int bnum = ddActionValue_getIntValue(b);

	push(dd_newActionValue_int(bnum ^ anum));

	ddActionValue_release(a);
	ddActionValue_release(b);
}


static inline void
action_bitwiseOr(ddActionContext* context)
{
	ddActionValue a = pop();
	ddActionValue b = pop();

	int anum = ddActionValue_getIntValue(a);
	int bnum = ddActionValue_getIntValue(b);

	push(dd_newActionValue_int(bnum | anum));

	ddActionValue_release(a);
	ddActionValue_release(b);
}


static inline void
action_bitwiseAnd(ddActionContext* context)
{
	ddActionValue a = pop();
	ddActionValue b = pop();

	int anum = ddActionValue_getIntValue(a);
	int bnum = ddActionValue_getIntValue(b);

	push(dd_newActionValue_int(bnum & anum));

	ddActionValue_release(a);
	ddActionValue_release(b);
}


static inline void
action_modulo(ddActionContext* context)
{
	ddActionValue a = pop();
	ddActionValue b = pop();

	int anum = ddActionValue_getIntValue(a);
	int bnum = ddActionValue_getIntValue(b);

	push(dd_newActionValue_int(bnum % anum));

	ddActionValue_release(a);
	ddActionValue_release(b);
}


static inline void
action_modulo_int(ddActionContext* context, int mod)
{
	ddActionValue val = pop();
	int num = ddActionValue_getIntValue(val);

	push(dd_newActionValue_int(num % mod));

	ddActionValue_release(val);
}


static inline void
action_increment(ddActionContext* context)
{
	ddActionValue a = pop();
	ddActionValue copy = ddActionValue_increment(a);
	ddActionValue_release(a);

	push(copy);
}


static inline void
action_decrement(ddActionContext* context)
{
	ddActionValue a = pop();
	ddActionValue copy = ddActionValue_decrement(a);
	ddActionValue_release(a);

	push(copy);
}


static inline void
action_setNamedMember(ddActionContext* context, char* name, ddActionValue val)
{
	ddActionValue objval = pop();
	ddActionObject* object = ddActionValue_getObjectValue(objval);

	if ( object != NULL )
		ddActionObject_setProperty(context, object, name, val);

	ddActionValue_release(objval);
	ddActionObject_release(object);
}


static inline void
action_setMember(ddActionContext* context)
{
	ddActionValue value = pop();
	ddActionValue member = pop();
	ddActionValue objval = pop();

	ddActionObject* object = ddActionValue_getObjectValue(objval);

	if ( object != NULL )
	{
		char* name = ddActionValue_getStringValue_release(context, member);
		ddActionObject_setProperty(context, object, name, value);
	}
	else
		ddActionValue_release(member);
	
	ddActionValue_release(objval);
	ddActionObject_release(object);
}


static inline void
action_getMember(ddActionContext* context)
{
	ddActionValue member = pop();
	ddActionValue objval = pop();

	ddActionObject* object = ddActionValue_getObjectValue(objval);

	if ( object != NULL )
	{
		char *name = ddActionValue_getStringValue_release(context, member);

		push(ddActionObject_getProperty(context, object, name));
		dd_free(name);
	}
	else
	{
		push(ddNullValue);
		ddActionValue_release(member);
	}
	
	ddActionValue_release(objval);
	ddActionObject_release(object);
}


static inline void
action_getNamedMember(ddActionContext* context, const char* name)
{
	ddActionValue objval = pop();
	ddActionObject* object = ddActionValue_getObjectValue(objval);

	if ( object != NULL )
		push(ddActionValue_copy(ddActionObject_getProperty(context, object, name)));
	else
		push(ddNullValue);

	ddActionValue_release(objval);
	ddActionObject_release(object);
}


static inline void
action_swap(ddActionContext* context)
{
	ddActionValue val1 = pop();
	ddActionValue val2 = pop();
	push(val1);
	push(val2);
}


static inline void
action_newEqual(ddActionContext* context)
{
	ddActionValue a = pop();
	ddActionValue b = pop();

	if ( a.type == VALUE_UNDEF && b.type == VALUE_UNDEF )
		push(ddTrueValue);

	else if ( a.type == VALUE_NULL && b.type == VALUE_NULL )
		push(ddTrueValue);

	else if ( a.type == VALUE_NULL || b.type == VALUE_NULL )
		push(ddFalseValue);

	else if ( ((a.type == VALUE_INTEGER) || (a.type == VALUE_DOUBLE)) &&
		   ((b.type == VALUE_INTEGER) || (b.type == VALUE_DOUBLE)))
	{
		double anum = ddActionValue_getDoubleValue(a);
		double bnum = ddActionValue_getDoubleValue(b);

		if ( anum == bnum )
			push(ddTrueValue);
		else
			push(ddFalseValue);
	}
	else
	{
		char* astr = ddActionValue_getStringValue_release(context, a);
		char* bstr = ddActionValue_getStringValue_release(context, b);

		if ( strcmp(astr, bstr) == 0 )
			push(ddTrueValue);
		else
			push(ddFalseValue);

		dd_free(astr);
		dd_free(bstr);
	}
}


static inline void
action_equal_double(ddActionContext* context, double num)
{
	ddActionValue val = pop();

	if ( val.type == VALUE_NULL || val.type == VALUE_UNDEF )
	{
		push(ddFalseValue);
	}
	else
	{
		double valnum = ddActionValue_getDoubleValue(val);

		if ( valnum == num )
			push(ddTrueValue);
		else
			push(ddFalseValue);
	}

	ddActionValue_release(val);
}


static inline void
action_equal_string(ddActionContext* context, const char* str)
{
	ddActionValue val = pop();

	if ( val.type == VALUE_NULL || val.type == VALUE_UNDEF )
	{
		push(ddFalseValue);
	}
	else if ( val.type == VALUE_INTEGER || val.type == VALUE_DOUBLE )
	{
		ddActionValue testval = dd_newActionValue_string_noCopy((char*)str);
		double testnum = ddActionValue_getDoubleValue(testval);
		double num = ddActionValue_getDoubleValue(val);

		push((testnum == num) ? ddTrueValue : ddFalseValue);
	}
	else if ( val.type == VALUE_STRING )
	{
		push((strcmp(val.data.stringValue, str) == 0) ? ddTrueValue : ddFalseValue);
	}
	else
	{
		char* teststr = ddActionValue_getStringValue_release(context, val);

		push((strcmp(teststr, str) == 0) ? ddTrueValue : ddFalseValue);
		dd_free(teststr);
	}
}


static inline void
action_strictEq(ddActionContext* context)
{
	ddActionValue a = pop();
	ddActionValue b = pop();
	boolean ret = DD_FALSE;

	if ( a.type == VALUE_STRING && b.type == VALUE_STRING )
		ret = (strcmp(a.data.stringValue, b.data.stringValue) == 0);

	else if ( a.type == VALUE_INTEGER )
	{
		if ( b.type == VALUE_INTEGER )
			ret = (a.data.intValue == b.data.intValue);
		else if ( b.type == VALUE_DOUBLE )
			ret = (a.data.intValue == b.data.doubleValue);
	}
	else if ( a.type == VALUE_DOUBLE )
	{
		if ( b.type == VALUE_INTEGER )
			ret = (a.data.doubleValue == b.data.intValue);
		else if ( b.type == VALUE_DOUBLE )
			ret = (a.data.doubleValue == b.data.doubleValue);
	}
	else if ( (a.type == VALUE_OBJECT || a.type == VALUE_FUNCTION) &&
		   (b.type == VALUE_OBJECT || b.type == VALUE_FUNCTION) )
	{
		ret = (a.data.objectValue == b.data.objectValue);
	}
	else if ( a.type == VALUE_BOOLEAN && b.type == VALUE_BOOLEAN )
		ret = (a.data.intValue == b.data.intValue);

	ddActionValue_release(a);
	ddActionValue_release(b);

	push(ret ? ddTrueValue : ddFalseValue);
}


static inline void
action_newLessThan(ddActionContext* context)
{
	ddActionValue b = pop();
	ddActionValue a = pop();

	if ( a.type == VALUE_STRING && b.type == VALUE_STRING )
	{
		char* astr = ddActionValue_getStringValue_release(context, a);
		char* bstr = ddActionValue_getStringValue_release(context, b);

		if ( strcmp(astr, bstr) < 0 )
			push(ddTrueValue);
		else
			push(ddFalseValue);

		dd_free(astr);
		dd_free(bstr);
	}
	else
	{
		double anum = ddActionValue_getDoubleValue(a);
		double bnum = ddActionValue_getDoubleValue(b);

		if ( isnan(anum) || isnan(bnum) )
			push(ddUndefValue);
		
		if ( anum < bnum )
			push(ddTrueValue);
		else
			push(ddFalseValue);
	}
}


static inline void
action_lessThan_double(ddActionContext* context, double num)
{
	ddActionValue val = pop();
	double testnum = ddActionValue_getDoubleValue(val);
	
	if ( isnan(testnum) )
		push(ddUndefValue);
	else
		push((testnum < num) ? ddTrueValue : ddFalseValue);

	ddActionValue_release(val);
}


static inline void
action_lessThan_string(ddActionContext* context, const char* str)
{
	ddActionValue val = pop();

	if ( val.type == VALUE_INTEGER || val.type == VALUE_DOUBLE )
	{
		ddActionValue testval = dd_newActionValue_string_noCopy((char*)str);
		double testnum = ddActionValue_getDoubleValue(testval);
		double num = ddActionValue_getDoubleValue(val);

		push((testnum < num) ? ddTrueValue : ddFalseValue);
	}
	else if ( val.type == VALUE_STRING )
	{
		push((strcmp(val.data.stringValue, str) == 0) ? ddTrueValue : ddFalseValue);
	}
	else
		push(ddFalseValue);

	ddActionValue_release(val);
}


static inline void
action_newGreaterThan(ddActionContext* context)
{
	ddActionValue b = pop();
	ddActionValue a = pop();

	if ( a.type == VALUE_STRING && b.type == VALUE_STRING )
	{
		char* astr = ddActionValue_getStringValue_release(context, a);
		char* bstr = ddActionValue_getStringValue_release(context, b);

		if ( strcmp(astr, bstr) > 0 )
			push(ddTrueValue);
		else
			push(ddFalseValue);

		dd_free(astr);
		dd_free(bstr);
	}
	else
	{
		double anum = ddActionValue_getDoubleValue(a);
		double bnum = ddActionValue_getDoubleValue(b);

		if ( isnan(anum) || isnan(bnum) )
			push(ddUndefValue);

		if ( anum > bnum )
			push(ddTrueValue);
		else
			push(ddFalseValue);
	}
}


static inline void
action_greaterThan_double(ddActionContext* context, double num)
{
	ddActionValue val = pop();
	double testnum = ddActionValue_getDoubleValue(val);

	if ( isnan(testnum) )
		push(ddUndefValue);
	else
		push((testnum > num) ? ddTrueValue : ddFalseValue);

	ddActionValue_release(val);
}


static inline void
action_greaterThan_string(ddActionContext* context, const char* str)
{
	ddActionValue val = pop();

	if ( val.type == VALUE_INTEGER || val.type == VALUE_DOUBLE )
	{
		ddActionValue testval = dd_newActionValue_string_noCopy((char*)str);
		double testnum = ddActionValue_getDoubleValue(testval);
		double num = ddActionValue_getDoubleValue(val);

		push((testnum > num) ? ddTrueValue : ddFalseValue);
	}
	else if ( val.type == VALUE_STRING )
	{
		push((strcmp(str, val.data.stringValue) == 0) ? ddTrueValue : ddFalseValue);
	}
	else
		push(ddFalseValue);

	ddActionValue_release(val);
}


static inline void
action_newAdd(ddActionContext* context)
{
	ddActionValue a = pop();
	ddActionValue b = pop();

	if ( a.type == VALUE_UNDEF )
	{
		push(b);
	}
	else if ( b.type == VALUE_UNDEF )
	{
		push(a);
	}
	else if ( a.type == VALUE_STRING || b.type == VALUE_STRING )
	{
		char* astr = ddActionValue_getStringValue_release(context, a);
		char* bstr = ddActionValue_getStringValue_release(context, b);

		bstr = dd_realloc(bstr, strlen(bstr) + strlen(astr) + 1);
		strcat(bstr, astr);
		dd_free(astr);

		push(dd_newActionValue_string_noCopy(bstr));
	}
	else
	{
		double anum = ddActionValue_getDoubleValue(a);
		double bnum = ddActionValue_getDoubleValue(b);

		push(dd_newActionValue_double(anum + bnum));

		ddActionValue_release(a);
		ddActionValue_release(b);
	}
}


static inline void
action_add_int(ddActionContext* context, int num)
{
	ddActionValue val = pop();

	if ( val.type == VALUE_INTEGER )
	{
		val.data.intValue += num;
		push(val);
	}
	else
	{
		double dnum = ddActionValue_getDoubleValue(val);
		push(dd_newActionValue_double(dnum + num));
		ddActionValue_release(val);
	}
}


static inline void
action_add_double(ddActionContext* context, double num)
{
	ddActionValue val = pop();
	double dnum = ddActionValue_getDoubleValue(val);
	push(dd_newActionValue_double(dnum + num));
	ddActionValue_release(val);
}


static inline void
action_add_string(ddActionContext* context, const char* str)
{
	char* lstr = ddActionValue_getStringValue_release(context, pop());

	lstr = dd_realloc(lstr, strlen(lstr) + strlen(str) + 1);
	strcat(lstr, str);

	push(dd_newActionValue_string_noCopy(lstr));
}


static inline void
action_initObject(ddActionContext* context)
{
	ddActionObject* object = dd_newActionObject();
	ddActionValue val = pop();
	int i;
	int count = ddActionValue_getIntValue(val);

	for( i = 0; i < count; ++i )
	{
		ddActionValue val = pop();
		char* name = ddActionValue_getStringValue_release(context, pop());

		ddActionObject_setProperty(context, object, name, val);
	}

	push(dd_newActionValue_object(object));

	ddActionValue_release(val);
}


static inline void
action_return(ddPlayer* player, ddActionContext** contextptr)
{
	// pop the context
	// if something was on the function stack, push it on the old stack

	ddActionContext* context = *contextptr;
	ddActionOpcode* instrPtr = context->instrPtr;
	ddActionValue value;

	if ( context->isConstructor )
		value = dd_newActionValue_object(context->object);
	else if ( context->stack.nItems != 0 )
		value = pop();
	else
		value = ddNullValue;

	ddActionValue_retain(value);
	
	popContext(contextptr);
	context = *contextptr;

	// if we're landing at the same location, that was a 'with'- so don't push
	if ( context->instrPtr != instrPtr )
		push(value);

	ddActionValue_release(value);
}


static inline void
action_namedNew(ddActionContext** contextPtr, const char* class)
{
	ddActionContext* context = *contextPtr;
	ddActionValue nargsval = pop();
	int nargs = ddActionValue_getIntValue(nargsval);

	ddActionValue ctorval = ddActionObject_getProperty(context, context->object, class);
	ddActionFunction* ctor;

	if ( ctorval.type == VALUE_UNDEF )
		ctorval = ddActionObject_getProperty(context, context->player->globals, class);

	ctor = ddActionValue_getFunctionValue(ctorval);

	if ( ctor != NULL )
	{
		ddActionObject* object = dd_newActionObject();
		
		ddActionObject_setPrototype(object, ctor->prototype);
		
		ddActionFunction_call(ctor, object, contextPtr, NULL, nargs, DD_TRUE);
	}
	else
	{
		while ( nargs-- > 0 )
			ddActionValue_release(pop());

		push(ddNullValue);
	}

	ddActionValue_release(nargsval);
	ddActionValue_release(ctorval);
}


static inline void
action_newMethod(ddActionContext** contextPtr)
{
	ddActionContext* context = *contextPtr;
	char* name = ddActionValue_getStringValue_release(context, pop());
	ddActionValue objectval = pop();
	ddActionValue nargsval = pop();

	ddActionObject* object;
	int nargs = ddActionValue_getIntValue(nargsval);
	ddActionFunction* ctor;
	
	if ( name != NULL )
	{
		ddActionValue ctorval;
		
		object = ddActionValue_getObjectValue(objectval);
		ctorval = ddActionObject_getProperty(context, object, name);
		ctor = ddActionValue_getFunctionValue(ctorval);
		ddActionValue_release(ctorval);
	}
	else
	{
		// object is constructor function
		ctor = ddActionValue_getFunctionValue(objectval);
		object = dd_newActionObject();
	}
	
	if ( ctor != NULL )
	{
		ddActionObject_setPrototype(object, ctor->prototype);
		ddActionFunction_call(ctor, object, contextPtr, NULL, nargs, DD_TRUE);
	}
	else
	{
		while ( nargs-- > 0 )
			ddActionValue_release(pop());
		
		push(ddNullValue);
	}

	ddActionValue_release(objectval);
	dd_free(name);
}


static inline void
action_new(ddActionContext** contextPtr)
{
	ddActionContext* context = *contextPtr;
	char* name = ddActionValue_getStringValue_release(context, pop());
	
	action_namedNew(contextPtr, name);
	dd_free(name);
}


static inline void
action_callNamedMethod(ddPlayer* player, ddActionContext** contextptr, const char* name)
{
	ddActionContext* context = *contextptr;
	ddActionValue objectVal = pop();
	ddActionValue nargsVal = pop();

	ddActionObject* object;
	int nargs = ddActionValue_getIntValue(nargsVal);

	ddActionValue funcval;
	ddActionFunction* function = NULL;

	if ( objectVal.type == VALUE_SUPER )
	{
		ddActionObject* super = context->object->proto;
		
		if ( context->class != NULL )
			super = context->class;
		
		if ( super->proto != NULL )
		{
			funcval = ddActionObject_getProperty(context, super, name);
			function = ddActionValue_getFunctionValue(funcval);

			ddActionFunction_call(function, context->object, contextptr, super->proto, nargs, DD_FALSE);
		}
		else
			push(ddNullValue);
	}
	else
	{
		object = ddActionValue_getObjectValue(objectVal);
		
		funcval = ddActionObject_getProperty(context, object, name);
		function = ddActionValue_getFunctionValue(funcval);

		if ( function == NULL )
			dd_warn("Couldn't find method \"%s\"", name);

		ddActionFunction_call(function, object, contextptr, NULL, nargs, DD_FALSE);
	}
	
#ifdef DD_INCLUDE_DEBUGGER
	if ( *contextptr != context )
		ddActionContext_setLabel(*contextptr, name);
#endif

	ddActionValue_release(funcval);
	ddActionValue_release(nargsVal);
	ddActionValue_release(objectVal);
	ddActionObject_release(object);
}


static inline void
action_callMethod(ddPlayer* player, ddActionContext** contextptr)
{
	ddActionContext* context = *contextptr;
	char* name = ddActionValue_getStringValue_release(context, pop());

	action_callNamedMethod(player, contextptr, name);
	dd_free(name);
}


static inline void
action_callNamedFunctionNargs(ddPlayer* player, ddActionContext** contextptr, const char* name, int nargs)
{
	ddActionContext* context = *contextptr;
	ddActionValue funcval;
	ddActionFunction* function = NULL;

	if ( strcasecmp(name, "super") == 0 )
	{
		ddActionObject* class = context->object->proto;
		
		if ( context->class != NULL )
			class = context->class;

		if ( class->proto != NULL )
		{
			funcval = ddActionHash_findValue(class->proto->properties, "constructor");
			function = ddActionValue_getFunctionValue(funcval);
			
			ddActionFunction_call(function, context->object, contextptr, class->proto, nargs, DD_TRUE);
		}
		else
			push(ddNullValue);
	}
	else
	{
		funcval = ddActionObject_getProperty(context, context->object, name);
		function = ddActionValue_getFunctionValue(funcval);
	
		if ( function == NULL )
			dd_warn("Couldn't find function \"%s\"", name);

		ddActionFunction_call(function, context->object, contextptr, NULL, nargs, DD_FALSE);
	}

#ifdef DD_INCLUDE_DEBUGGER
	if ( *contextptr != context )
		ddActionContext_setLabel(*contextptr, name);
#endif
	
	ddActionValue_release(funcval);
}


static inline void
action_callNamedFunction(ddPlayer* player, ddActionContext** contextptr, const char* name)
{
	ddActionContext* context = *contextptr;
	ddActionValue nargsVal = pop();

	int nargs = ddActionValue_getIntValue(nargsVal);

	action_callNamedFunctionNargs(player, contextptr, name, nargs);

	ddActionValue_release(nargsVal);
}


static inline void
action_callFunction(ddPlayer* player, ddActionContext** contextptr)
{
	ddActionContext* context = *contextptr;
	char* name = ddActionValue_getStringValue_release(context, pop());
	
	action_callNamedFunction(player, contextptr, name);
	dd_free(name);
}


static inline void
action_deleteNamed(ddActionContext* context, const char* name)
{
	/*
  // char* name = ddActionValue_getStringValue_release(context, pop());

	 // XXX - returns true if var deleted, else false
  // XXX - also doesn't seem to delete from a function's locals,
  // but the this object
	 */

	dd_warn("delete action not implemented");
}


static inline void
action_delete(ddActionContext* context)
{
	char* name = ddActionValue_getStringValue_release(context, pop());

	action_deleteNamed(context, name);
	dd_free(name);
}


static inline void
action_gotoExpression(ddActionContext* context, boolean play)
{
	ddActionValue val = pop();

	if ( val.type != VALUE_NULL )
	{
		char* label = ddActionValue_getStringValue_release(context, val);

		// XXX - should make sure it's a movie clip
		ddActionMovieClip_gotoFrameLabel(context, (ddActionMovieClip*)context->object, label, play);
		dd_free(label);
	}
}


static inline void
action_namedVarEquals(ddActionContext* context, char* name, ddActionValue val)
{
	/* add a variable to this context and set its value */

	// 'var a = 1;' on the clip's timeline adds the var to the clip scope,
	// not the short-lived context.  Are they reusing contexts?

	// XXX - note this causes 'var _x = 1;' to have side effects..

	if ( context->next == NULL )
		ddActionObject_setProperty(context, context->object, name, val);
	else
		ddActionContext_addLocal(context, name, val);
}


static inline void
action_var(ddActionContext* context)
{
	char* name = ddActionValue_getStringValue_release(context, pop());

	action_namedVarEquals(context, name, ddUndefValue);
}


static inline void
action_varEquals(ddActionContext* context)
{
	ddActionValue val = pop();
	char* name = ddActionValue_getStringValue_release(context, pop());

	action_namedVarEquals(context, name, val);

	ddActionValue_release(val);
}	


static inline void
action_initArray(ddActionContext* context)
{
	ddActionArray* array = dd_newActionArray();
	ddActionValue val = pop();
	int i, count = ddActionValue_getIntValue(val);

	for ( i = 0; i < count; ++i )
		ddActionArray_setMember(array, i, pop());

	push(dd_newActionValue_object((ddActionObject*)array));

	ddActionValue_release(val);
}


static inline void
action_typeof(ddActionContext* context)
{
	ddActionValue a = pop();

	if ( a.type == VALUE_NULL )
		push(dd_newActionValue_string("null"));
	else
		push(dd_newActionValue_string(ddActionValue_getTypeName(a)));

	ddActionValue_release(a);
}


static inline void
action_callNamedFrame(ddPlayer* player, ddActionContext* context, const char* label)
{
	/* execute a frame's actions */

	ddActionMovieClip* clipobj = (ddActionMovieClip*)context->object;
	ddMovieClip* clip;
	int frame;
	
	char* p = strchr(label, ':');
	
	if ( p != NULL )
	{
		ddActionValue val;
		ddActionObject* target;
		char* str = strdup(label);
		
		str[p-label] = '\0';
		
		val = getSlashPath(context, str);
		target = ddActionValue_getObjectValue(val);
		
		if ( target != NULL && target->proto == (ddActionObject*)ddActionMovieClipClass )
			clipobj = (ddActionMovieClip*)target;
		else
			return;
		
		label = p + 1;
	}

	clip = clipobj->movieClip->clip;
	frame = ddMovieClip_findFrameLabel(clip, label);

	if ( frame != -1 )
	{
		ddAction* action = clip->timeline->frames[frame].actions;

		/* XXX - can cause endless recursion */
		if ( action != NULL )
			ddAction_execute(player, action, (ddActionObject*)clipobj);
	}
	else
		dd_warn("Couldn't find frame named '%s'", label);
}


static inline void
action_callFrame(ddPlayer* player, ddActionContext* context)
{
	char* label = ddActionValue_getStringValue_release(context, pop());

	action_callNamedFrame(player, context, label);
	dd_free(label);
}


static inline void
action_branchIfTrue(ddPlayer* player, ddActionContext* context, int offset)
{
	ddActionValue val = pop();

	if ( ddActionValue_getBooleanValue(val) )
		context->instrPtr += offset;

	ddActionValue_release(val);
}


static inline void
action_branchIfFalse(ddPlayer* player, ddActionContext* context, int offset)
{
	ddActionValue val = pop();

	if ( !ddActionValue_getBooleanValue(val) )
		context->instrPtr += offset;

	ddActionValue_release(val);
}


static inline void
action_duplicateClip(ddActionContext* context)
{
	// target can be either a clip or a path
	
	ddActionValue depthval = pop();
	ddActionValue nameval = pop();
	ddActionValue targetval = pop();

	int depth = ddActionValue_getIntValue(depthval);
	char* name = ddActionValue_getStringValue_release(context, nameval);
	ddActionObject* target;

	if ( targetval.type != VALUE_OBJECT )
	{
		char* tmp = ddActionValue_getStringValue_release(context, targetval);
		target = ddActionValue_getObjectValue(getSlashPath(context, tmp));
		dd_free(tmp);
	}
	else
	{
		target = ddActionValue_getObjectValue(targetval);
		ddActionValue_release(targetval);
	}

	if ( target != NULL && target->proto == (ddActionObject*)ddActionMovieClipClass && name != NULL )
	{
		ddActionMovieClip* parent = (ddActionMovieClip*)context->object;
		ddActionMovieClip* clip = (ddActionMovieClip*)target;

		ddDrawClip_duplicateClip(context->player, clip->movieClip, parent->movieClip, depth, name);
	}

	dd_free(name);
	ddActionValue_release(depthval);
}


static inline void
action_removeClip(ddActionContext* context)
{
	// target is either a path or an object
	ddActionValue targetval = pop();
	ddActionObject* target;

	if ( targetval.type != VALUE_OBJECT )
	{
		char* tmp = ddActionValue_getStringValue_release(context, targetval);
		target = ddActionValue_getObjectValue(getSlashPath(context, tmp));
		dd_free(tmp);
	}
	else
	{
		target = ddActionValue_getObjectValue(targetval);
		ddActionValue_release(targetval);
	}
	
	if ( target != NULL && target->proto == (ddActionObject*)ddActionMovieClipClass )
	{
		// XXX - make sure context object is a movieclip
		ddActionMovieClip_removeChild((ddActionMovieClip*)context->object,
								(ddActionMovieClip*)target);
	}
}


void
action_startDrag(ddActionContext* context)
{
	ddActionValue targetval = pop(); // doesn't actually matter, apparently..
	ddActionValue lockval = pop();
	ddActionValue constrainval = pop();
	ddRect constraint = ddInvalidRect;

	boolean lock = ddActionValue_getBooleanValue(lockval);

	/*
	ddActionObject* target;

	if ( targetval.type != VALUE_OBJECT )
	{
		char* path = ddActionValue_getStringValue(context, targetval);
		ddActionValue val = getSlashPath(context, path);
		target = ddActionValue_getObjectValue(val);
		ddActionValue_release(val);
		dd_free(path);
	}
	else
		target = ddActionValue_getObjectValue(targetval);
	 */
	
	if ( ddActionValue_getBooleanValue(constrainval) )
	{
		ddActionValue bottomval = pop();
		ddActionValue rightval = pop();
		ddActionValue topval = pop();
		ddActionValue leftval = pop();

		constraint.left = FIXED_I(ddActionValue_getIntValue(leftval));
		constraint.right = FIXED_I(ddActionValue_getIntValue(rightval));
		constraint.top = FIXED_I(ddActionValue_getIntValue(topval));
		constraint.bottom = FIXED_I(ddActionValue_getIntValue(bottomval));

		ddActionValue_release(leftval);
		ddActionValue_release(rightval);
		ddActionValue_release(topval);
		ddActionValue_release(bottomval);
	}

	ddActionValue_release(targetval);
	ddActionValue_release(constrainval);
	ddActionValue_release(lockval);

	if ( context->object->proto == (ddActionObject*)ddActionMovieClipClass)
	{
		ddDrawClip* clip = ((ddActionMovieClip*)context->object)->movieClip;
		ddPlayer_startDrag(context->player, clip, lock, constraint);
	}

	//ddActionObject_release(target);
}


void
dd_doBytecodeAction(ddPlayer* player, ddActionContext** contextptr, ddActionOpcode* op)
{
	ddActionContext* context = *contextptr;
	
	switch ( op->op )
	{
		/* v3 actions: */

		case ACTION_END:
			return;

		case ACTION_NEXTFRAME:
			ddActionMovieClip_nextFrame(context->object, context, 0);
			break;

		case ACTION_PREVFRAME:
			ddActionMovieClip_prevFrame(context->object, context, 0);
			break;

		case ACTION_PLAY:
			ddActionMovieClip_play(context->object, context, 0);
			break;

		case ACTION_STOP:
			ddActionMovieClip_stop(context->object, context, 0);
			break;

		case ACTION_TOGGLEQUALITY:
			dd_warn("ToggleQuality action not implemented");
			/* ddActionMovieClip_toggleQuality(object, context, 0); */
			break;

		case ACTION_STOPSOUNDS:
			dd_warn("StopSounds action not implemented");
			/* ddActionMovieClip_stopSounds(object, context, 0); */
			break;

		case ACTION_GOTOFRAME:
			// flash 3 gotoframe is zero-based, f4 is one-based
			push(dd_newActionValue_int(op->val.data.intValue + 1));
			ddActionMovieClip_gotoAndStop(context->object, context, 1);
			break;

		case ACTION_GETURL:
			action_getURL(context, op);
			break;
		
		case ACTION_WAITFORFRAME:
			action_waitForFrame(player, context, op);
			break;

		case ACTION_SETTARGET:  /* arg: string target */
			action_setTarget(context, op);
			break;

		case ACTION_GOTOLABEL:  /* arg: string label */
			ddActionMovieClip_gotoFrameLabel(context, (ddActionMovieClip*)context->object,
									op->arg, 0);
			break;


		/* v4 actions: */

		case ACTION_ADD:
			action_add(context);
			break;

		case ACTION_SUBTRACT:
			action_subtract(context);
			break;

		case ACTION_MULTIPLY:
			action_multiply(context);
			break;

		case ACTION_DIVIDE:
			action_divide(context);
			break;

		case ACTION_EQUAL:
			action_equal(context);
			break;

		case ACTION_LESSTHAN:
			action_lessThan(context);
			break;

		case ACTION_LOGICALOR:
			action_logicalOr(context);
			break;

		case ACTION_LOGICALAND:
			action_logicalAnd(context);
			break;

		case ACTION_LOGICALNOT:
			action_logicalNot(context);
			break;

		case ACTION_STRINGEQ:
			action_stringEq(context);
			break;

		case ACTION_STRINGLENGTH:
			action_stringLength(context);
			break;

		case ACTION_SUBSTRING:
			action_substring(context);
			break;

		case ACTION_POP:
			ddActionValue_release(pop());
			break;

		case ACTION_INT:
			action_int(context);
			break;

		case ACTION_GETVARIABLE:
			action_getVariable(context);
			break;

		case ACTION_SETVARIABLE:
			action_setVariable(context);
			break;

		case ACTION_SETTARGETEXPRESSION:
			action_setTargetExpression(context);
			break;

		case ACTION_STRINGCONCAT:
			action_stringConcat(context);
			break;

		case ACTION_GETPROPERTY:
			action_getProperty(context);
			break;

		case ACTION_SETPROPERTY:
			action_setProperty(context);
			break;

		case ACTION_DUPLICATECLIP:
			action_duplicateClip(context);
			break;

		case ACTION_REMOVECLIP:
			action_removeClip(context);
			break;

		case ACTION_TRACE:
			action_trace(context);
			break;

		case ACTION_STARTDRAGMOVIE:
			action_startDrag(context);
			break;

		case ACTION_STOPDRAGMOVIE:
			ddPlayer_stopDrag(context->player);
			break;

		case ACTION_STRINGCOMPARE:
			action_stringCompare(context);
			break;

		case ACTION_RANDOM:
			action_random(context);
			break;

		case ACTION_MBLENGTH:
			dd_warn("mbLength ation not implemented");
			break;

		case ACTION_ORD:
		{
			char* str = ddActionValue_getStringValue_release(context, pop());

			if ( str != NULL )
				push(dd_newActionValue_int((unsigned char)str[0]));
			else
				push(dd_newActionValue_int(0));
			
			break;
		}
		case ACTION_CHR:
		{
			ddActionValue val = pop();
			int num = ddActionValue_getIntValue(val);
			char* str = strdup("x");

			str[0] = num;

			push(dd_newActionValue_string_noCopy(str));
			ddActionValue_release(val);

			break;
		}
		case ACTION_GETTIMER:
			push(dd_newActionValue_double(dd_getTicks(player) - player->startTime));
			break;

		case ACTION_MBSUBSTRING:
			dd_warn("mbSubstring action not implemented");
			break;

		case ACTION_MBORD:
			dd_warn("mbOrd action not implemented");
			break;

		case ACTION_MBCHR:
			dd_warn("mbChr action not implemented");
			break;

		case ACTION_WAITFORFRAMEEXPRESSION:
			dd_warn("waitForFrameExpression action not implemented");
			break;

		case ACTION_PUSHDATA:
			dd_warn("unexpected PUSHDATA op at runtime");
			break;

		case ACTION_BRANCHALWAYS:
			context->instrPtr += op->val.data.intValue;
			break;

		case ACTION_GETURL2:
		{
			char* target = ddActionValue_getStringValue_release(context, pop()); // target
			char* file = ddActionValue_getStringValue_release(context, pop()); // file
			dd_warn("getURL2 action (\"%s\", \"%s\") not implemented", file, target);
			dd_free(target);
			dd_free(file);
			break;
		}

		case ACTION_BRANCHIFTRUE:
			action_branchIfTrue(player, context, op->val.data.intValue);
			break;

		case ACTION_CALLFRAME:
			action_callFrame(player, context);
			break;

		case ACTION_GOTOEXPRESSION:
			action_gotoExpression(context, op->data);
			break;

			/* v5 actions: */

		case ACTION_DELETE:
			action_delete(context);
			break;

		case ACTION_CALLFUNCTION:
			action_callFunction(player, contextptr);
			break;

		case ACTION_CALLMETHOD:
			action_callMethod(player, contextptr);
			break;

		case ACTION_RETURN:
			action_return(player, contextptr);
			break;

		case ACTION_NEW:
			action_new(contextptr);
			break;

		case ACTION_NEWMETHOD:
			action_newMethod(contextptr);
			break;

		case ACTION_VAR:
			action_var(context);
			break;

		case ACTION_VAREQUALS:
			action_varEquals(context);
			break;

		case ACTION_INITARRAY:
			action_initArray(context);
			break;

		case ACTION_INITOBJECT:
			action_initObject(context);
			break;

		case ACTION_TYPEOF:
			action_typeof(context);
			break;

		case ACTION_NEWADD:
			action_newAdd(context);
			break;

		case ACTION_NEWLESSTHAN:
			action_newLessThan(context);
			break;

		case ACTION_NEWEQUAL:
			action_newEqual(context);
			break;

		case ACTION_DUP:
			push(ddActionValue_copy(ddActionStack_top(&context->stack)));
			break;

		case ACTION_SWAP:
			action_swap(context);
			break;

		case ACTION_GETMEMBER:
			action_getMember(context);
			break;

		case ACTION_SETMEMBER:
			action_setMember(context);
			break;

		case ACTION_INCREMENT:
			action_increment(context);
			break;

		case ACTION_DECREMENT:
			action_decrement(context);
			break;

		case ACTION_MODULO:
			action_modulo(context);
			break;

		case ACTION_BITWISEAND:
			action_bitwiseAnd(context);
			break;

		case ACTION_BITWISEOR:
			action_bitwiseOr(context);
			break;

		case ACTION_BITWISEXOR:
			action_bitwiseXor(context);
			break;

		case ACTION_SHIFTLEFT:
			action_shiftLeft(context);
			break;

		case ACTION_SHIFTRIGHT:
			action_shiftRight(context);
			break;

		case ACTION_SHIFTRIGHT2:
			action_shiftRight2(context);
			break;

		case ACTION_DECLARENAMES:
			dd_warn("unexpected declarenames op at runtime");
			//action_declareNames(context);
			break;

		case ACTION_WITH:
			action_with(contextptr, op);
			break;

		case ACTION_DEFINEFUNCTION:
			// XXX - if op->arg is null, it's an anonymous function..?
			ddActionHash_setValue(context->object->properties, strdup(op->arg), op->val);
			break;

		case ACTION_ENUMERATE:
			action_enumerate(context);
			break;

		case ACTION_ENUMERATE2:
			action_enumerate2(context);
			break;
		
		case ACTION_SETREGISTER:
			action_setRegister(context, op);
			break;

		case ACTION_MAKENUMBER:
		{
			ddActionValue val = pop();

			if ( val.type == VALUE_INTEGER )
				push(val);
			else
				push(dd_newActionValue_double(ddActionValue_getDoubleValue(val)));
			
			ddActionValue_release(val);
			break;
		}
		case ACTION_MAKESTRING:
		{
			char* str = ddActionValue_getStringValue_release(context, pop());
			push(dd_newActionValue_string_noCopy(str));
			break;
		}
		case ACTION_STRICTEQ:
			action_strictEq(context);
			break;

		case ACTION_GREATERTHAN:
			action_newGreaterThan(context);
			break;

		// recoder ops:

		case ACTION_GETNAMEDVAR:
			action_getNamedVariable(context, op->arg);
			break;

		case ACTION_SETNAMEDVAR:
			action_setNamedVariable(context, strdup(op->arg), ddActionValue_copy(op->val));
			break;

		case ACTION_GETNAMEDMEMBER:
			action_getNamedMember(context, op->arg);
			break;

		case ACTION_SETNAMEDMEMBER:
			action_setNamedMember(context, strdup(op->arg), ddActionValue_copy(op->val));
			break;

		case ACTION_DELETENAMEDVAR:
			action_deleteNamed(context, op->arg);
			break;

		case ACTION_NAMEDVAR:
			action_namedVarEquals(context, strdup(op->arg), ddUndefValue);
			break;

		case ACTION_NAMEDVAREQ:
			action_namedVarEquals(context, strdup(op->arg), ddActionValue_copy(op->val));
			break;

		case ACTION_NEWNAMED:
			action_namedNew(contextptr, strdup(op->arg));
			break;

		case ACTION_EQUALS_INT:
			action_equal_double(context, op->val.data.intValue);
			break;

		case ACTION_EQUALS_DOUBLE:
			action_equal_double(context, op->val.data.doubleValue);
			break;

		case ACTION_EQUALS_STRING:
			action_equal_string(context, op->val.data.stringValue);
			break;

		case ACTION_LESSTHAN_INT:
			action_lessThan_double(context, op->val.data.intValue);
			break;

		case ACTION_LESSTHAN_DOUBLE:
			action_lessThan_double(context, op->val.data.doubleValue);
			break;

		case ACTION_LESSTHAN_STRING:
			action_lessThan_string(context, op->val.data.stringValue);
			break;

		case ACTION_GREATERTHAN_INT:
			action_greaterThan_double(context, op->val.data.intValue);
			break;

		case ACTION_GREATERTHAN_DOUBLE:
			action_greaterThan_double(context, op->val.data.doubleValue);
			break;

		case ACTION_GREATERTHAN_STRING:
			action_greaterThan_string(context, op->val.data.stringValue);
			break;

		case ACTION_ADD_INT:
			action_add_int(context, op->val.data.intValue);
			break;

		case ACTION_ADD_DOUBLE:
			action_add_double(context, op->val.data.doubleValue);
			break;

		case ACTION_ADD_STRING:
			action_add_string(context, op->val.data.stringValue);
			break;

		case ACTION_SUBTRACT_INT:
			action_subtract_int(context, op->val.data.intValue);
			break;

		case ACTION_SUBTRACT_DOUBLE:
			action_subtract_double(context, op->val.data.doubleValue);
			break;

		case ACTION_MULTIPLY_INT:
			action_multiply_int(context, op->val.data.intValue);
			break;

		case ACTION_MULTIPLY_DOUBLE:
			action_multiply_double(context, op->val.data.doubleValue);
			break;

		case ACTION_DIVIDE_INT:
			action_divide_int(context, op->val.data.intValue);
			break;

		case ACTION_DIVIDE_DOUBLE:
			action_divide_double(context, op->val.data.doubleValue);
			break;

		case ACTION_MODULO_INT:
			action_modulo_int(context, op->val.data.intValue);
			break;

		case ACTION_SHIFTLEFT_INT:
			action_shiftLeft_int(context, op->val.data.intValue);
			break;

		case ACTION_SHIFTRIGHT_INT:
			action_shiftRight_int(context, op->val.data.intValue);
			break;

		case ACTION_CALLNAMEDFUNCTION:
			action_callNamedFunction(player, contextptr, op->arg);
			break;

		case ACTION_CALLNAMEDFUNCTIONNARGS:
			action_callNamedFunctionNargs(player, contextptr, op->arg, op->val.data.intValue);
			break;
		
		case ACTION_CALLNAMEDMETHOD:
			action_callNamedMethod(player, contextptr, op->arg);
			break;
		
		case ACTION_CALLNAMEDFRAME:
			action_callNamedFrame(player, context, op->arg);
			break;
		
		case ACTION_PUSH_VALUE:
			push(ddActionValue_copy(op->val));
			break;

		case ACTION_PUSH_REGISTER:
			push(ddActionValue_copy(player->registers[op->data]));
			break;

		case ACTION_BRANCHIFFALSE:
			action_branchIfFalse(player, context, op->val.data.intValue);
			break;
		
		case ACTION_INSTANCEOF:
			dd_warn("action ACTION_INSTANCEOF not implemented"); break;
		case ACTION_STRINGGREATER:
			dd_warn("action ACTION_STRINGGREATER not implemented"); break;
		case ACTION_DEFINEFUNCTION2:
			dd_warn("action ACTION_DEFINEFUNCTION2 not implemented"); break;
		case ACTION_EXTENDS:
			dd_warn("action ACTION_EXTENDS not implemented"); break;
		case ACTION_CAST:
			dd_warn("action ACTION_CAST not implemented"); break;
		case ACTION_IMPLEMENTS:
			dd_warn("action ACTION_IMPLEMENTS not implemented"); break;
		case ACTION_TRY:
			dd_warn("action ACTION_TRY not implemented"); break;
		case ACTION_THROW:
			dd_warn("action ACTION_THROW not implemented"); break;
		
		default:
			dd_warn("unknown byte action: %02x", op->op);
	}
}
