/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>
#include <math.h>

#include "../function.h"

ddActionObject* mathObject = NULL;

static ddActionValue
Math_abs(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);
	double dval = ddActionValue_getDoubleValue(val);

	ddActionValue_release(val);

	return dd_newActionValue_double((dval < 0) ? -dval : dval);
}


static ddActionValue
Math_acos(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);
	double dval = ddActionValue_getDoubleValue(val);

	ddActionValue_release(val);

	return dd_newActionValue_double(acos(dval));
}


static ddActionValue
Math_asin(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);
	double dval = ddActionValue_getDoubleValue(val);

	ddActionValue_release(val);

	return dd_newActionValue_double(asin(dval));
}


static ddActionValue
Math_atan(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);
	double dval = ddActionValue_getDoubleValue(val);

	ddActionValue_release(val);

	return dd_newActionValue_double(atan(dval));
}


static ddActionValue
Math_atan2(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue vala = ddActionStack_pop(&context->stack);
	ddActionValue valb = ddActionStack_pop(&context->stack);

	double dvala = ddActionValue_getDoubleValue(vala);
	double dvalb = ddActionValue_getDoubleValue(valb);

	ddActionValue_release(valb);
	ddActionValue_release(vala);

	return dd_newActionValue_double(atan2(dvala, dvalb));
}


static ddActionValue
Math_ceil(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);
	double dval = ddActionValue_getDoubleValue(val);

	ddActionValue_release(val);

	return dd_newActionValue_double(ceil(dval));
}


static ddActionValue
Math_cos(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);
	double dval = ddActionValue_getDoubleValue(val);

	ddActionValue_release(val);

	return dd_newActionValue_double(cos(dval));
}


static ddActionValue
Math_exp(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);
	double dval = ddActionValue_getDoubleValue(val);

	ddActionValue_release(val);

	return dd_newActionValue_double(exp(dval));
}


static ddActionValue
Math_floor(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);
	double dval = ddActionValue_getDoubleValue(val);

	ddActionValue_release(val);

	return dd_newActionValue_double(floor(dval));
}


static ddActionValue
Math_log(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);
	double dval = ddActionValue_getDoubleValue(val);

	ddActionValue_release(val);

	return dd_newActionValue_double(log(dval));
}


static ddActionValue
Math_max(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val;
	double maxval;

	if ( nargs == 0 )
		return ddNullValue;

	val = ddActionStack_pop(&context->stack);
	maxval = ddActionValue_getDoubleValue(val);
	ddActionValue_release(val);

	while ( --nargs > 0 )
	{
		ddActionValue nextval = ddActionStack_pop(&context->stack);
		double dval = ddActionValue_getDoubleValue(nextval);

		if ( dval > maxval )
			maxval = dval;

		ddActionValue_release(nextval);
	}

	return dd_newActionValue_double(maxval);
}


static ddActionValue
Math_min(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val;
	double minval;

	if ( nargs == 0 )
		return ddNullValue;

	val = ddActionStack_pop(&context->stack);
	minval = ddActionValue_getDoubleValue(val);
	ddActionValue_release(val);

	while ( --nargs > 0 )
	{
		ddActionValue nextval = ddActionStack_pop(&context->stack);
		double dval = ddActionValue_getDoubleValue(nextval);

		if ( dval < minval )
			minval = dval;

		ddActionValue_release(nextval);
	}

	return dd_newActionValue_double(minval);
}


static ddActionValue
Math_pow(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue vala = ddActionStack_pop(&context->stack);
	ddActionValue valb = ddActionStack_pop(&context->stack);

	double dvala = ddActionValue_getDoubleValue(vala);
	double dvalb = ddActionValue_getDoubleValue(valb);

	ddActionValue_release(vala);
	ddActionValue_release(valb);

	return dd_newActionValue_double(pow(dvala, dvalb));
}


static ddActionValue
Math_random(ddActionObject* object, ddActionContext* context, int nargs)
{
	/* cygwin seems to return rand() > RAND_MAX..? */
	return dd_newActionValue_double((double)(rand() % RAND_MAX) / RAND_MAX);
}


static ddActionValue
Math_round(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);
	double dval = ddActionValue_getDoubleValue(val);

	ddActionValue_release(val);

	return dd_newActionValue_double(floor(dval + 0.5));
}


static ddActionValue
Math_sin(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);
	double dval = ddActionValue_getDoubleValue(val);

	ddActionValue_release(val);

	return dd_newActionValue_double(sin(dval));
}


static ddActionValue
Math_sqrt(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);
	double dval = ddActionValue_getDoubleValue(val);

	ddActionValue_release(val);

	return dd_newActionValue_double(sqrt(dval));
}


static ddActionValue
Math_tan(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);
	double dval = ddActionValue_getDoubleValue(val);

	ddActionValue_release(val);

	return dd_newActionValue_double(tan(dval));
}


#define addFunction(name, nargs) \
ddActionHash_addValue_flags(math->properties, #name, \
							dd_newActionValue_function(dd_newActionNativeFunction(Math_##name, nargs)), \
							HASH_READONLY | HASH_PERMANENT | HASH_DONTENUM);

#define addConstant(name, val) \
ddActionHash_addValue_flags(math->properties, #name, \
							dd_newActionValue_double(val), \
							HASH_READONLY | HASH_PERMANENT | HASH_DONTENUM);

ddActionObject*
dd_initMathObject()
{
	mathObject = dd_newActionObject();

	ddActionObject_addNativeMethod(mathObject, strdup("abs"), Math_abs, 1);
	ddActionObject_addNativeMethod(mathObject, strdup("acos"), Math_acos, 1);
	ddActionObject_addNativeMethod(mathObject, strdup("asin"), Math_asin, 1);
	ddActionObject_addNativeMethod(mathObject, strdup("atan"), Math_atan, 1);
	ddActionObject_addNativeMethod(mathObject, strdup("atan2"), Math_atan2, 2);
	ddActionObject_addNativeMethod(mathObject, strdup("ceil"), Math_ceil, 1);
	ddActionObject_addNativeMethod(mathObject, strdup("cos"), Math_cos, 1);
	ddActionObject_addNativeMethod(mathObject, strdup("exp"), Math_exp, 1);
	ddActionObject_addNativeMethod(mathObject, strdup("floor"), Math_floor, 1);
	ddActionObject_addNativeMethod(mathObject, strdup("log"), Math_log, 1);
	ddActionObject_addNativeMethod(mathObject, strdup("pow"), Math_pow, 1);
	ddActionObject_addNativeMethod(mathObject, strdup("random"), Math_random, 0);
	ddActionObject_addNativeMethod(mathObject, strdup("round"), Math_round, 1);
	ddActionObject_addNativeMethod(mathObject, strdup("sin"), Math_sin, 1);
	ddActionObject_addNativeMethod(mathObject, strdup("sqrt"), Math_sqrt, 1);
	ddActionObject_addNativeMethod(mathObject, strdup("tan"), Math_tan, 1);
	ddActionObject_addNativeMethod(mathObject, strdup("max"), Math_max, FUNCTION_VARARGS);
	ddActionObject_addNativeMethod(mathObject, strdup("min"), Math_min, FUNCTION_VARARGS);

	ddActionHash_setValue(mathObject->properties, strdup("E"), dd_newActionValue_double(M_E));
	ddActionHash_setValue(mathObject->properties, strdup("LN10"), dd_newActionValue_double(M_LN10));
	ddActionHash_setValue(mathObject->properties, strdup("LN2"), dd_newActionValue_double(M_LN2));
	ddActionHash_setValue(mathObject->properties, strdup("LOG10E"), dd_newActionValue_double(M_LOG10E));
	ddActionHash_setValue(mathObject->properties, strdup("LOG2E"), dd_newActionValue_double(M_LOG2E));
	ddActionHash_setValue(mathObject->properties, strdup("PI"), dd_newActionValue_double(M_PI));
	ddActionHash_setValue(mathObject->properties, strdup("SQRT1_2"), dd_newActionValue_double(M_SQRT1_2));
	ddActionHash_setValue(mathObject->properties, strdup("SQRT2"), dd_newActionValue_double(M_SQRT2));

	return mathObject;
}
