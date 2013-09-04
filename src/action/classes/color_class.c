/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>

#include "color_class.h"
#include "movieclip_class.h"
#include "object_class.h"
#include "../function.h"

ddActionClass* ddActionColorClass = NULL;

static void
Color_destroy(ddActionObject* object)
{
	ddActionColor* color = (ddActionColor*)object;
	
	ddActionObject_release((ddActionObject*)color->clip);
	color->clip = NULL;
}


static ddActionValue
Color_constructor(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionColor* color;
	ddActionValue val = ddActionStack_pop(&context->stack);
	ddActionObject* obj;
	
	if ( val.type == VALUE_STRING )
	{
		char* name = ddActionValue_getStringValue_release(context, val);
		val = ddActionObject_getProperty(context, context->object, name);
	}

	obj = ddActionValue_getObjectValue(val);
	ddActionValue_release(val);
	
	if ( obj == NULL || !ddActionObject_isKindOfClass(obj, ddActionMovieClipClass) )
		return ddNullValue;

	color = dd_malloc(sizeof(ddActionColor));

	ddActionObject_init((ddActionObject*)color);
	ddActionObject_setClass((ddActionObject*)color, ddActionColorClass);
	
	color->clip = (ddActionMovieClip*)ddActionObject_retain(obj);

	return dd_newActionValue_object((ddActionObject*)color);
}


static ddActionValue
Color_getRGB(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionColor* color = (ddActionColor*)object;
	ddDrawClip* drawclip;
	ddCXform xform;
	
	if ( !ddActionObject_isKindOfClass(object, ddActionColorClass) )
		return ddUndefValue;
	
	drawclip = color->clip->movieClip;

	if ( drawclip == NULL )
		return ddUndefValue;
	
	xform = ddDrawable_getCXform((ddDrawable*)drawclip);
	
	/* XXX - doesn't deal with negative values properly! */

	return dd_newActionValue_int((xform.rAdd << 16) | (xform.gAdd << 8) | xform.bAdd);
}


static ddActionValue
Color_setRGB(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionColor* color = (ddActionColor*)object;
	ddDrawClip* drawclip;
	ddCXform xform = { 0, 0, 0, 0, 0, 0, 0, 0 };
	
	ddActionValue val;
	int num;
	
	if ( !ddActionObject_isKindOfClass(object, ddActionColorClass) )
		return ddUndefValue;

	drawclip = color->clip->movieClip;

	if ( drawclip == NULL )
		return ddNullValue;
	
	val = ddActionStack_pop(&context->stack);
	num = ddActionValue_getIntValue(val);
	ddActionValue_release(val);
	
	xform.aMult = 0x100;
	xform.rAdd = (num & 0xff0000) >> 16;
	xform.gAdd = (num & 0x00ff00) >> 8;
	xform.bAdd = (num & 0x0000ff);

	ddDrawable_setCXform((ddDrawable*)drawclip, xform, DD_FALSE);

	return ddNullValue;
}


static ddActionValue
Color_getTransform(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionColor* color = (ddActionColor*)object;
	ddDrawClip* drawclip;
	ddCXform xform;
	ddActionObject* obj;
	
	if ( !ddActionObject_isKindOfClass(object, ddActionColorClass) )
		return ddUndefValue;
	
	drawclip = color->clip->movieClip;

	if ( drawclip == NULL )
		return ddUndefValue;
		
	xform = ddDrawable_getCXform((ddDrawable*)drawclip);
	obj = dd_newActionObject();
	
	ddActionHash_addValue(obj->properties, strdup("aa"), dd_newActionValue_int(100 * xform.aMult / 256));
	ddActionHash_addValue(obj->properties, strdup("ra"), dd_newActionValue_int(100 * xform.rMult / 256));
	ddActionHash_addValue(obj->properties, strdup("ga"), dd_newActionValue_int(100 * xform.gMult / 256));
	ddActionHash_addValue(obj->properties, strdup("ba"), dd_newActionValue_int(100 * xform.bMult / 256));
	ddActionHash_addValue(obj->properties, strdup("ab"), dd_newActionValue_int(100 * xform.aAdd / 256));
	ddActionHash_addValue(obj->properties, strdup("rb"), dd_newActionValue_int(100 * xform.rAdd / 256));
	ddActionHash_addValue(obj->properties, strdup("gb"), dd_newActionValue_int(100 * xform.gAdd / 256));
	ddActionHash_addValue(obj->properties, strdup("bb"), dd_newActionValue_int(100 * xform.bAdd / 256));

	return dd_newActionValue_object(obj);
}

static ddActionValue
Color_setTransform(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionColor* color = (ddActionColor*)object;

	ddCXform xform;
	ddActionValue val;
	ddActionObject* obj;

	if ( !ddActionObject_isKindOfClass(object, ddActionColorClass) )
		return ddUndefValue;
	
	xform = ddDrawable_getCXform((ddDrawable*)color->clip->movieClip);
	val = ddActionStack_pop(&context->stack);
	obj = ddActionValue_getObjectValue(val);
	
	if ( obj == NULL )
		return ddNullValue;
	
	xform.aMult = 256 * ddActionValue_getIntValue(ddActionHash_findValue(obj->properties, "aa")) / 100;
	xform.rMult = 256 * ddActionValue_getIntValue(ddActionHash_findValue(obj->properties, "ra")) / 100;
	xform.gMult = 256 * ddActionValue_getIntValue(ddActionHash_findValue(obj->properties, "ga")) / 100;
	xform.bMult = 256 * ddActionValue_getIntValue(ddActionHash_findValue(obj->properties, "ba")) / 100;
	xform.aAdd = 256 * ddActionValue_getIntValue(ddActionHash_findValue(obj->properties, "ab")) / 100;
	xform.rAdd = 256 * ddActionValue_getIntValue(ddActionHash_findValue(obj->properties, "rb")) / 100;
	xform.gAdd = 256 * ddActionValue_getIntValue(ddActionHash_findValue(obj->properties, "gb")) / 100;
	xform.bAdd = 256 * ddActionValue_getIntValue(ddActionHash_findValue(obj->properties, "bb")) / 100;

	ddDrawable_setCXform((ddDrawable*)color->clip->movieClip, xform, DD_FALSE);

	return ddNullValue;
}


ddActionFunction*
dd_initColorClass()
{
	ddActionFunction* ctor = dd_newActionNativeFunction(Color_constructor, 1);
	ddActionObject* classObject;
	
	ddActionColorClass = dd_newActionClass();
	classObject = (ddActionObject*)ddActionColorClass;

	ddActionObject_addNativeMethod(classObject, strdup("getRGB"), Color_getRGB, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setRGB"), Color_setRGB, 1);
	ddActionObject_addNativeMethod(classObject, strdup("getTransform"), Color_getTransform, 0);
	ddActionObject_addNativeMethod(classObject, strdup("setTransform"), Color_setTransform, 1);

	ddActionColorClass->destroy = Color_destroy;

	ddActionFunction_setPrototype(ctor, classObject);

	return ctor;
}
