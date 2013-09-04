/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <math.h>
#include <string.h>

#include "movieclip_class.h"
#include "object_class.h"
#include "../actiontypes.h"
#include "../function.h"
#include "../../render/rect.h"

ddActionClass* ddActionMovieClipClass = NULL;

void dd_destroyActionMovieClip(ddActionObject* object)
{
	ddActionMovieClip* clip = (ddActionMovieClip*)object;
	
	if ( clip->name != NULL )
		dd_free(clip->name);
}

#ifdef DD_INCLUDE_DEBUGGER
/*
 properties:
 alpha
 currentframe
 droptarget
 focusrect
 framesloaded
 height
 highquality
 name
 quality
 rotation
 soundbuftime
 target
 totalframes
 url
 visible
 width
 x
 xmouse
 xscale
 y
 ymouse
 yscale

 _level0, etc.
 */
#endif

void
ddActionMovieClip_setProperty(ddActionContext* context, ddActionObject* object,
							  char* name, ddActionValue value)
{
	ddActionValue val = ddActionHash_findValue(object->properties, name);
	ddDrawClip* clip = ((ddActionMovieClip*)object)->movieClip;

	if ( val.type == VALUE_TEXTFIELD )
	{
		ddDrawTextField_setText(val.data.textFieldValue,
						  ddActionValue_getStringValue_release(context, value));

		dd_free(name);
	}
	else if ( name != NULL && clip != NULL && clip->clip != NULL && name[0] == '_' )
	{
		/* XXX- can do faster lookup w/ binary search */

		if ( strcasecmp(name + 1, "x") == 0 )
		{
			ddDrawClip_setX(clip, FIXED_D(ddActionValue_getDoubleValue(value)));
			dd_free(name);
		}
		else if ( strcasecmp(name + 1, "y") == 0 )
		{
			ddDrawClip_setY(clip, FIXED_D(ddActionValue_getDoubleValue(value)));
			dd_free(name);
		}	
		else if ( strcasecmp(name + 1, "rotation") == 0 )
		{
			ddDrawClip_setRotation(clip, M_PI * ddActionValue_getDoubleValue(value) / 180);
			dd_free(name);
		}	
		else if ( strcasecmp(name + 1, "xscale") == 0 )
		{
			ddDrawClip_setXScale(clip, ddActionValue_getDoubleValue(value) / 100);
			dd_free(name);
		}	
		else if ( strcasecmp(name + 1, "yscale") == 0 )
		{
			ddDrawClip_setYScale(clip, ddActionValue_getDoubleValue(value) / 100);
			dd_free(name);
		}	
		else if ( strcasecmp(name + 1, "width") == 0 )
		{
			ddDrawClip_setWidth(clip, FIXED_D(ddActionValue_getDoubleValue(value)));
			dd_free(name);
		}	
		else if ( strcasecmp(name + 1, "height") == 0 )
		{
			ddDrawClip_setHeight(clip, FIXED_D(ddActionValue_getDoubleValue(value)));
			dd_free(name);
		}	
		else if ( strcasecmp(name + 1, "alpha") == 0 )
		{
			ddDrawClip_setAlpha(clip, ddActionValue_getDoubleValue(value) / 100);
			dd_free(name);
		}	
		else if ( strcasecmp(name + 1, "visible") == 0 )
		{
			ddDrawClip_setVisible(clip, ddActionValue_getBooleanValue(value));
			dd_free(name);
		}
		
		/* XXX - etc */

		else
			ddActionObject_setProperty_default(context, object, name, value);
	}
	else
		ddActionObject_setProperty_default(context, object, name, value);

	/*
	if ( strncasecmp(name, "on", 2) == 0 )
	{
		ddActionFunction* function = ddActionValue_getFunctionValue(value);
		ddAction* action;
		
		if ( function == NULL )
			return;
		
		action = ddActionFunction_getBytecodeAction(function);
		
		if ( action == NULL )
			return;
		
		if ( strcasecmp(name, "onMouseMove") == 0 )
			ddPlayer_registerMouseMoveListener(context->player, clip, action);

		else if ( strcasecmp(name, "onMouseUp") == 0 )
			ddPlayer_registerMouseUpListener(context->player, clip, action);

		else if ( strcasecmp(name, "onMouseDown") == 0 )
			ddPlayer_registerMouseDownListener(context->player, clip, action);

		else if ( strcasecmp(name, "onKeyUp") == 0 )
			ddPlayer_registerKeyUpListener(context->player, clip, action);
		
		else if ( strcasecmp(name, "onKeyDown") == 0 )
			ddPlayer_registerMouseMoveListener(context->player, clip, action);
		
		else if ( strcasecmp(name, "onEnterFrame") == 0 )
			ddPlayer_registerEnterFrameListener(context->player, clip, action);
	}
	 */
}


ddActionValue
ddActionMovieClip_getPropertyNum(ddPlayer* player, ddActionMovieClip* clip, ddClipProperty num)
{
	ddDrawClip* mc = clip->movieClip;
	
	if ( mc == NULL )
		return ddUndefValue;

	switch ( num )
	{
		case PROPERTY_X:
			return dd_newActionValue_double(DOUBLE_F(ddDrawClip_getX(mc)));

		case PROPERTY_Y:
			return dd_newActionValue_double(DOUBLE_F(ddDrawClip_getY(mc)));

		case PROPERTY_XSCALE:
			return dd_newActionValue_double(100 * ddDrawClip_getXScale(mc));

		case PROPERTY_YSCALE:
			return dd_newActionValue_double(100 * ddDrawClip_getYScale(mc));

		case PROPERTY_CURRENTFRAME:
			return dd_newActionValue_int(ddDrawClip_getCurrentFrame(mc));

		case PROPERTY_TOTALFRAMES:
			return dd_newActionValue_int(ddDrawClip_getTotalFrames(mc));

		case PROPERTY_ALPHA:
			return dd_newActionValue_double(100 * ddDrawClip_getAlpha(mc));

		case PROPERTY_VISIBLE:
			return dd_newActionValue_boolean(ddDrawClip_isVisible(mc));

		case PROPERTY_WIDTH:
			return dd_newActionValue_double(ddDrawClip_getWidth(mc));

		case PROPERTY_HEIGHT:
			return dd_newActionValue_double(ddDrawClip_getWidth(mc));

		case PROPERTY_ROTATION:
			return dd_newActionValue_double(180 * ddDrawClip_getRotation(mc) / M_PI);

		case PROPERTY_FRAMESLOADED:
			return dd_newActionValue_int(ddDrawClip_getFramesLoaded(mc));

		case PROPERTY_NAME:
			return dd_newActionValue_string(clip->name);

		case PROPERTY_XMOUSE:
			return dd_newActionValue_double(DOUBLE_F(ddDrawClip_getXMouse(player, mc)));

		case PROPERTY_YMOUSE:
			return dd_newActionValue_double(DOUBLE_F(ddDrawClip_getYMouse(player, mc)));

		case PROPERTY_TARGET:
		case PROPERTY_DROPTARGET:
		case PROPERTY_URL:
		case PROPERTY_HIGHQUALITY:
		case PROPERTY_FOCUSRECT:
		case PROPERTY_SOUNDBUFTIME:
		case PROPERTY_QUALITY:
			dd_warn("Unimplemented getProperty type: %i", num);
			return ddNullValue;

		default:
			dd_warn("Unknown getProperty type: %i", num);
			return ddNullValue;
	}
}


ddActionValue
ddActionMovieClip_getProperty(ddActionContext* context, ddActionObject* object, const char* name)
{
	ddActionValue val = ddActionHash_findValue(object->properties, name);
	ddDrawClip* mc = ((ddActionMovieClip*)object)->movieClip;

	if ( val.type == VALUE_TEXTFIELD )
		return dd_newActionValue_string(ddDrawTextField_getText(val.data.textFieldValue));

	else if ( val.type != VALUE_UNDEF )
		return val;
	
	else if ( name[0] == '.' && name[1] == '.' && name[2] == '\0' )
		return dd_newActionValue_object((ddActionObject*)mc->parentClip->actionClip);

	if ( name[0] == '_' && context != NULL )
	{
		ddPlayer* player = context->player;

		if ( mc != NULL )
		{
			if ( strcasecmp(name+1, "x") == 0 )
				return dd_newActionValue_double(DOUBLE_F(ddDrawClip_getX(mc)));
			
			else if ( strcasecmp(name+1, "y") == 0 )
				return dd_newActionValue_double(DOUBLE_F(ddDrawClip_getY(mc)));
			
			else if ( strcasecmp(name+1, "xscale") == 0 )
				return dd_newActionValue_double(100*ddDrawClip_getXScale(mc));
			
			else if ( strcasecmp(name+1, "yscale") == 0 )
				return dd_newActionValue_double(100*ddDrawClip_getYScale(mc));
			
			else if ( strcasecmp(name+1, "currentframe") == 0 )
				return dd_newActionValue_int(ddDrawClip_getCurrentFrame(mc));
			
			else if ( strcasecmp(name+1, "totalframes") == 0 )
				return dd_newActionValue_int(ddDrawClip_getTotalFrames(mc));
			
			else if ( strcasecmp(name+1, "alpha") == 0 )
				return dd_newActionValue_double(100 * ddDrawClip_getAlpha(mc));
			
			else if ( strcasecmp(name+1, "visible") == 0 )
				return dd_newActionValue_boolean(ddDrawClip_isVisible(mc));
			
			else if ( strcasecmp(name+1, "width") == 0 )
				return dd_newActionValue_double(ddDrawClip_getWidth(mc));
			
			else if ( strcasecmp(name+1, "height") == 0 )
				return dd_newActionValue_double(ddDrawClip_getWidth(mc));
			
			else if ( strcasecmp(name+1, "rotation") == 0 )
				return dd_newActionValue_double(180 * ddDrawClip_getRotation(mc) / M_PI);
			
			else if ( strcasecmp(name+1, "framesloaded") == 0 )
				return dd_newActionValue_int(ddDrawClip_getFramesLoaded(mc));
		
			else if ( strcasecmp(name+1, "xmouse") == 0 )
				return dd_newActionValue_double(DOUBLE_F(ddDrawClip_getXMouse(player, mc)));
			
			else if ( strcasecmp(name+1, "ymouse") == 0 )
				return dd_newActionValue_double(DOUBLE_F(ddDrawClip_getYMouse(player, mc)));
			
			else if ( strcasecmp(name+1, "parent" ) == 0 )
				return dd_newActionValue_object((ddActionObject*)mc->parentClip->actionClip);
			
			// XXX - these two (and other levels) should probably be available everywhere
			
			else if ( strcasecmp(name+1, "root") == 0 )
				return dd_newActionValue_object((ddActionObject*)mc->actionClip->rootClip);
			
			else if ( strcasecmp(name+1, "level0") == 0 )
				return dd_newActionValue_object((ddActionObject*)mc->actionClip->rootClip);
		}
		else if ( strcasecmp(name+1, "name") == 0 )
			return dd_newActionValue_string(((ddActionMovieClip*)object)->name);

		// _target: slash-path of this clip
		// _url: url of file that created this clip
		
		/* XXX - etc */

		else
			return ddActionObject_getProperty_default(context, object, name);
	}

	return ddActionObject_getProperty_default(context, object, name);
}


void
ddActionMovieClip_setPropertyNum(ddActionMovieClip* clip, ddClipProperty num, ddActionValue value)
{
	ddDrawClip* mc = clip->movieClip;

	if ( mc == NULL )
		return;
	
	switch ( num )
	{
		case PROPERTY_X:
			ddDrawClip_setX(mc, FIXED_D(ddActionValue_getDoubleValue(value)));
			break;
			
		case PROPERTY_Y:
			ddDrawClip_setY(mc, FIXED_D(ddActionValue_getDoubleValue(value)));
			break;
			
		case PROPERTY_XSCALE:
			ddDrawClip_setXScale(mc, ddActionValue_getDoubleValue(value)/100);
			break;
			
		case PROPERTY_YSCALE:
			ddDrawClip_setYScale(mc, ddActionValue_getDoubleValue(value)/100);
			break;
			
		case PROPERTY_ALPHA:
			ddDrawClip_setAlpha(mc, ddActionValue_getDoubleValue(value)/100);
			break;
			
		case PROPERTY_VISIBLE:
			ddDrawClip_setVisible(mc, ddActionValue_getBooleanValue(value));
			break;
			
		case PROPERTY_WIDTH:
			ddDrawClip_setWidth(mc, FIXED_D(ddActionValue_getDoubleValue(value)));
			break;
			
		case PROPERTY_HEIGHT:
			ddDrawClip_setHeight(mc, FIXED_D(ddActionValue_getDoubleValue(value)));
			break;
			
		case PROPERTY_ROTATION:
			ddDrawClip_setRotation(mc, M_PI * ddActionValue_getDoubleValue(value) / 180);
			break;

		case PROPERTY_HIGHQUALITY:
			dd_warn("Unimplemented setProperty: PROPERTY_HIGHQUALITY");
			break;
		case PROPERTY_SOUNDBUFTIME:
			dd_warn("Unimplemented setProperty: PROPERTY_SOUNDBUFTIME");
			break;
		case PROPERTY_QUALITY:
			dd_warn("Unimplemented setProperty: PROPERTY_QUALITY");
			break;
		case PROPERTY_CURRENTFRAME:
			dd_warn("Unimplemented setProperty: PROPERTY_CURRENTFRAME");
			break;
			
		case PROPERTY_FRAMESLOADED:
		case PROPERTY_NAME:
		case PROPERTY_TOTALFRAMES:
		case PROPERTY_TARGET:
		case PROPERTY_DROPTARGET:
		case PROPERTY_URL:
		case PROPERTY_XMOUSE:
		case PROPERTY_YMOUSE:
		case PROPERTY_FOCUSRECT:
			/* read-only */
			dd_warn("Setting a read-only property: %i", num);
			break;

		default:
			dd_warn("Unknown getProperty type: %i", num);
			break;
	}
}


ddActionMovieClip*
dd_newActionMovieClip(ddPlayer* player, ddDrawClip* clip)
{
	ddActionMovieClip* actionclip = dd_malloc(sizeof(ddActionMovieClip));
	ddActionObject_init((ddActionObject*)actionclip);

	ddActionObject_setClass((ddActionObject*)actionclip, ddActionMovieClipClass);
	
	if ( clip == NULL )
		dd_warn("dd_newActionMovieClip: clip == NULL");
	
	actionclip->movieClip = clip;
	actionclip->name = NULL;

	if ( player->playclip != NULL )
		actionclip->rootClip = player->playclip->actionClip;
	else
		actionclip->rootClip = actionclip;
	
	/* XXX - ifdef DEBUG here? */
	if ( ddActionMovieClipClass == NULL )
		dd_error("dd_newActionMovieClip called before dd_initMovieClipObject!");

	return actionclip;
}


void
ddActionMovieClip_releaseMovieClip(ddActionMovieClip* clip)
{
	clip->movieClip = NULL;
}


ddDrawClip*
ddActionMovieClip_getDrawClip(ddActionMovieClip* clip)
{
	return clip->movieClip;
}


ddActionMovieClip*
ddActionMovieClip_duplicate(ddPlayer* player, ddActionMovieClip* clip)
{
	ddDrawClip* newclip;
	
	if ( clip->movieClip == NULL ) 
		return NULL;
	
	newclip = ddDrawClip_duplicate(player, clip->movieClip);
	
	return newclip->actionClip;
}


void
ddActionMovieClip_setName(ddActionMovieClip* clip, char* name)
{
	clip->name = name;
}


char*
ddActionMovieClip_getName(ddActionMovieClip* clip)
{
	return clip->name;
}


void
ddActionMovieClip_addChild(ddActionMovieClip* parent, ddActionMovieClip* child)
{
	child->rootClip = parent->rootClip;

	dd_log("%8p (\"%s\") adding child %8p (\"%s\")", parent, parent->name, child, child->name);

	ddActionHash_addValue(((ddActionObject*)parent)->properties, strdup(child->name),
					   dd_newActionValue_object((ddActionObject*)child));
}


void
ddActionMovieClip_removeChild(ddActionMovieClip* parent, ddActionMovieClip* child)
{
	ddActionHash_removeValue(((ddActionObject*)parent)->properties, child->name);
}


void
ddActionMovieClip_addTextField(ddActionMovieClip* clip, ddDrawTextField* field)
{
	ddActionValue val;

	val.type = VALUE_TEXTFIELD;
	val.data.textFieldValue = field;

	ddActionHash_addValue(((ddActionObject*)clip)->properties,
					   strdup(field->textfield->varname), val);

	if ( clip->movieClip != NULL )
		ddDrawTextField_setParentClip(field, clip->movieClip);
}


void
ddActionMovieClip_removeTextField(ddActionMovieClip* clip, ddDrawTextField* field)
{
	ddActionHash_removeValue(((ddActionObject*)clip)->properties,
					   field->textfield->varname);
}


void
ddActionMovieClip_gotoFrameLabel(ddActionContext* context,
								 ddActionMovieClip* clip, char* label, boolean play)
{
	char* p = strchr(label, ':');

	if ( p != NULL )
	{
		ddActionObject* object = context->object;
		ddActionValue val;
		
		*p = '\0';

		context->object = (ddActionObject*)clip;
		val = getSlashPath(context, label);
		clip = (ddActionMovieClip*)ddActionValue_getObjectValue(val); // XXX
		ddActionValue_release(val);
		context->object = object;

		if ( clip == NULL )
			return; // XXX
		
		label = p + 1;
	}
	
	if ( clip->movieClip == NULL )
		return;
	
	ddDrawClip_gotoFrameLabel(context->player, clip->movieClip, label);

	if ( play )
		ddDrawClip_play(context->player, clip->movieClip);
	else
		ddDrawClip_stop(context->player, clip->movieClip);
}


ddActionValue
ddActionMovieClip_gotoAndPlay(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionMovieClip* clip = (ddActionMovieClip*)object;
	ddActionValue val = ddActionStack_pop(&context->stack);
	int frame = ddActionValue_getIntValue(val);

	ddActionValue_release(val);

	if ( !ddActionObject_isKindOfClass(object, ddActionMovieClipClass) )
		return ddUndefValue;
	
	if ( clip->movieClip != NULL )
	{
		ddDrawClip_gotoFrame(context->player, clip->movieClip, frame - 1);
		ddDrawClip_play(context->player, clip->movieClip);
	}

	return ddNullValue;
}


ddActionValue
ddActionMovieClip_gotoAndStop(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionMovieClip* clip = (ddActionMovieClip*)object;
	ddActionValue val = ddActionStack_pop(&context->stack);
	int frame = ddActionValue_getIntValue(val);

	ddActionValue_release(val);

	if ( !ddActionObject_isKindOfClass(object, ddActionMovieClipClass) )
		return ddUndefValue;
	
	if ( clip->movieClip != NULL )
	{
		ddDrawClip_gotoFrame(context->player, clip->movieClip, frame - 1);
		ddDrawClip_stop(context->player, clip->movieClip);
	}

	return ddNullValue;
}


ddActionValue
ddActionMovieClip_nextFrame(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionMovieClip* clip = (ddActionMovieClip*)object;

	if ( !ddActionObject_isKindOfClass(object, ddActionMovieClipClass) )
		return ddUndefValue;
	
	if ( clip->movieClip != NULL )
		ddDrawClip_nextFrame(context->player, clip->movieClip);

	return ddNullValue;
}


ddActionValue
ddActionMovieClip_prevFrame(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionMovieClip* clip = (ddActionMovieClip*)object;

	if ( !ddActionObject_isKindOfClass(object, ddActionMovieClipClass) )
		return ddUndefValue;
	
	if ( clip->movieClip != NULL )
		ddDrawClip_prevFrame(context->player, clip->movieClip);

	return ddNullValue;
}


ddActionValue
ddActionMovieClip_play(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionMovieClip* clip = (ddActionMovieClip*)object;

	if ( !ddActionObject_isKindOfClass(object, ddActionMovieClipClass) )
		return ddUndefValue;
	
	if ( clip->movieClip != NULL )
		ddDrawClip_play(context->player, clip->movieClip);

	return ddNullValue;
}


ddActionValue
ddActionMovieClip_stop(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionMovieClip* clip = (ddActionMovieClip*)object;

	if ( !ddActionObject_isKindOfClass(object, ddActionMovieClipClass) )
		return ddUndefValue;
	
	if ( clip->movieClip != NULL )
		ddDrawClip_stop(context->player, clip->movieClip);

	return ddNullValue;
}


static ddActionValue
ddActionMovieClip_getBounds(ddActionObject* object, ddActionContext* context, int nargs)
{
	// XXX - one arg: parent coordinate space (clip)
	
	ddActionMovieClip* clip = (ddActionMovieClip*)object;
	ddActionObject* bounds;
	ddRect rect;

	if ( !ddActionObject_isKindOfClass(object, ddActionMovieClipClass) )
		return ddUndefValue;
	
	if ( clip->movieClip == NULL )
		return ddNullValue;

	rect = clip->movieClip->parent.bounds;

	if ( !RECT_VALID(rect) )
		return ddNullValue;

	bounds = dd_newActionObject();

	ddActionHash_addValue(bounds->properties, strdup("xMin"),
					   dd_newActionValue_double(DOUBLE_F(rect.left)));

	ddActionHash_addValue(bounds->properties, strdup("xMax"),
					   dd_newActionValue_double(DOUBLE_F(rect.right)));

	ddActionHash_addValue(bounds->properties, strdup("yMin"),
					   dd_newActionValue_double(DOUBLE_F(rect.top)));

	ddActionHash_addValue(bounds->properties, strdup("yMax"),
					   dd_newActionValue_double(DOUBLE_F(rect.bottom)));

	return dd_newActionValue_object(bounds);
}


static ddActionValue
ddActionMovieClip_getBytesLoaded(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionMovieClip* clip = (ddActionMovieClip*)object;

	if ( !ddActionObject_isKindOfClass(object, ddActionMovieClipClass) )
		return ddUndefValue;
	
	if ( clip->movieClip == NULL )
		return ddNullValue;

	return dd_newActionValue_int(ddDrawClip_getBytesLoaded(clip->movieClip));
}


static ddActionValue
ddActionMovieClip_getBytesTotal(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionMovieClip* clip = (ddActionMovieClip*)object;

	if ( !ddActionObject_isKindOfClass(object, ddActionMovieClipClass) )
		return ddUndefValue;
	
	if ( clip->movieClip == NULL )
		return ddNullValue;

	return dd_newActionValue_int(ddDrawClip_getBytesTotal(clip->movieClip));
}


static ddActionValue
ddActionMovieClip_attachMovie(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionMovieClip* clip = (ddActionMovieClip*)object;
	
	ddActionValue idval;
	ddActionValue nameval;
	ddActionValue depthval;
	ddActionValue initval = ddNullValue;

	ddCharacter* c;
	ddDrawable* d;
	ddActionFunction* ctor;
	
	if ( nargs < 3 )
	{
		while ( nargs-- > 0 )
			ddActionValue_release(ddActionStack_pop(&context->stack));
		
		return ddNullValue;
	}
	
	idval = ddActionStack_pop(&context->stack);
	nameval = ddActionStack_pop(&context->stack);
	depthval = ddActionStack_pop(&context->stack);
	
	if ( nargs > 3 )
		initval = ddActionStack_pop(&context->stack);
	
	while ( nargs-- > 4 )
		ddActionValue_release(ddActionStack_pop(&context->stack));

	if ( clip->movieClip != NULL &&
		 ddMovieClip_findLibrarySymbol(clip->rootClip->movieClip->clip,
									   ddActionValue_getStringValue(context, idval), &c, &ctor) )
	{
		d = ddDrawClip_addDisplayItem(context->player, clip->movieClip, c,
								ddActionValue_getIntValue(depthval),
								ddActionValue_getStringValue(context, nameval), -1);

		if ( ctor != NULL )
		{
			ddActionMovieClip* newclip = ((ddDrawClip*)d)->actionClip;
			ddActionObject* clipobj = (ddActionObject*)newclip;
			ddActionObject* initobject = ddActionValue_getObjectValue(initval);
			ddActionContext* tmpcontext;
			ddActionValue val;
			
			ddActionObject_setPrototype(clipobj, ctor->prototype);
			
			if ( initobject != NULL )
				ddActionObject_copyProperties(clipobj, initobject);
			
			// invoke ctor on newclip w/ no args
			tmpcontext = dd_newActionContext(context->player, clipobj);

			ddActionFunction_call(ctor, clipobj, &tmpcontext, NULL, 0, DD_TRUE);
			
			if ( ctor->type == FUNCTION_BYTECODE )
				val = ddActionContext_execute(context->player, &tmpcontext);
			else
				val = ddActionContext_popValue(tmpcontext);
			
			ddActionValue_release(val);
			dd_destroyActionContext(tmpcontext);
		}
	}

	if ( initval.type != VALUE_NULL )
		ddActionValue_release(initval);
	
	ddActionValue_release(depthval);
	ddActionValue_release(nameval);
	ddActionValue_release(idval);

	return ddNullValue;
}


static ddActionValue
ddActionMovieClip_duplicateMovieClip(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionMovieClip* clip = (ddActionMovieClip*)object;

	//if ( nargs != 2 )
	//	dd_warn("duplicateMovieClip expects at least 2 arguments");
	
	ddActionValue nameval = ddActionStack_pop(&context->stack);
	ddActionValue depthval = ddActionStack_pop(&context->stack);

	int depth = ddActionValue_getIntValue(depthval);
	char* name = ddActionValue_getStringValue(context, nameval);
	
	if ( clip->movieClip != NULL )
		ddDrawClip_duplicateClip(context->player, clip->movieClip, clip->movieClip->parentClip, depth, name);

	dd_free(name);
	
	ddActionValue_release(depthval);
	ddActionValue_release(nameval);
	
	return ddNullValue;
}


static ddActionValue
ddActionMovieClip_removeMovieClip(ddActionObject* object, ddActionContext* context, int nargs)
{
	dd_warn("removeMovieClip not implemented");
	/* no args */
	return ddNullValue;
}


static ddActionValue
ddActionMovieClip_unloadMovie(ddActionObject* object, ddActionContext* context, int nargs)
{
	dd_warn("unloadMovie not implemented");
	/* no args */
	return ddNullValue;
}


ddActionValue
ddActionMovieClip_startDrag(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionMovieClip* clip = (ddActionMovieClip*)object;
	ddRect constrain = ddInvalidRect;
	int lock = DD_FALSE;

	/* one arg: lock center */
	/* five args: lock center, constraints- left, top, right, bottom */

	if ( nargs >= 1 )
	{
		ddActionValue val = ddActionStack_pop(&context->stack);
		lock = ddActionValue_getBooleanValue(val);
		ddActionValue_release(val);
		--nargs;
	}

	if ( nargs >= 4 )
	{
		ddActionValue leftval = ddActionStack_pop(&context->stack);
		ddActionValue topval = ddActionStack_pop(&context->stack);
		ddActionValue rightval = ddActionStack_pop(&context->stack);
		ddActionValue bottomval = ddActionStack_pop(&context->stack);

		constrain = dd_makeRect(FIXED_D(ddActionValue_getDoubleValue(leftval)),
						 FIXED_D(ddActionValue_getDoubleValue(topval)),
						 FIXED_D(ddActionValue_getDoubleValue(rightval)),
						 FIXED_D(ddActionValue_getDoubleValue(bottomval)));
		nargs -= 4;
	}

	while ( nargs-- > 0 )
		ddActionValue_release(ddActionStack_pop(&context->stack));

	if ( clip->movieClip != NULL )
		ddPlayer_startDrag(context->player, clip->movieClip, lock, constrain);

	return ddNullValue;
}


static ddActionValue
ddActionMovieClip_stopDrag(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddPlayer_stopDrag(context->player);
	return ddNullValue;
}


static ddActionValue
ddActionMovieClip_hitTest(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionMovieClip* clip = (ddActionMovieClip*)object;

	ddActionValue xval, yval;
	fixed x, y;
	int flag;

	if ( !ddActionObject_isKindOfClass(object, ddActionMovieClipClass) )
		return ddUndefValue;
	
	if ( nargs == 0 )
		return ddFalseValue;

	if ( nargs == 1 )
	{
		ddActionValue val = ddActionStack_pop(&context->stack);
		ddActionObject* obj = ddActionValue_getObjectValue(val);
		ddActionMovieClip* objclip = (ddActionMovieClip*)obj;
		
		ddActionValue_release(val);

		if ( !ddActionObject_isKindOfClass(obj, ddActionMovieClipClass) )
			return ddUndefValue;
		
		if ( obj == NULL || clip->movieClip != NULL )
			return ddFalseValue;

		if ( ddRect_intersectsRect(clip->movieClip->parent.bounds, objclip->movieClip->parent.bounds) )
			return ddTrueValue;
		else
			return ddFalseValue;
	}

	xval = ddActionStack_pop(&context->stack);
	yval = ddActionStack_pop(&context->stack);

	if ( nargs == 3 )
	{
		ddActionValue flagval = ddActionStack_pop(&context->stack);
		flag = ddActionValue_getBooleanValue(flagval);
		ddActionValue_release(flagval);
	}
	else
		flag = DD_FALSE;

	while ( nargs-- > 3 )
		ddActionValue_release(ddActionStack_pop(&context->stack));

	/* 3 args- x, y, shapeFlag (true: use shape, false: just bbox) */

	x = FIXED_D(ddActionValue_getDoubleValue(xval));
	y = FIXED_D(ddActionValue_getDoubleValue(yval));

	ddActionValue_release(xval);
	ddActionValue_release(yval);

	if ( clip->movieClip == NULL )
		return ddFalseValue;
	
	ddDrawClip_localToGlobal(clip->movieClip, &x, &y);

	if ( flag )
		return ddDrawClip_hitTest(clip->movieClip, x, y) ? ddTrueValue: ddFalseValue;
	else
		return ddRect_containsPoint(clip->movieClip->parent.bounds, x, y) ? ddTrueValue: ddFalseValue;
}


static ddActionValue
ddActionMovieClip_globalToLocal(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);
	ddActionObject* obj = ddActionValue_getObjectValue(val);
	ddActionMovieClip* clip = (ddActionMovieClip*)object;

	ddActionValue xval, yval;
	fixed x, y;

	/* XXX - guess it's possible this could result in obj being freed.. */
	ddActionValue_release(val);

	if ( !ddActionObject_isKindOfClass(object, ddActionMovieClipClass) )
		return ddUndefValue;

	if ( obj == NULL || clip->movieClip == NULL )
		return ddNullValue;

	xval = ddActionHash_findValue(obj->properties, "x");
	yval = ddActionHash_findValue(obj->properties, "y");

	if ( xval.type == VALUE_NULL || yval.type == VALUE_NULL )
		return ddNullValue;

	x = FIXED_D(ddActionValue_getDoubleValue(xval));
	y = FIXED_D(ddActionValue_getDoubleValue(yval));

	ddDrawClip_globalToLocal(clip->movieClip, &x, &y);

	ddActionHash_setValue(obj->properties, strdup("x"), dd_newActionValue_double(DOUBLE_F(x)));
	ddActionHash_setValue(obj->properties, strdup("y"), dd_newActionValue_double(DOUBLE_F(y)));

	return ddNullValue;
}


static ddActionValue
ddActionMovieClip_localToGlobal(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionStack_pop(&context->stack);
	ddActionObject* obj = ddActionValue_getObjectValue(val);
	ddActionMovieClip* clip = (ddActionMovieClip*)object;
	
	ddActionValue xval, yval;
	fixed x, y;

	/* XXX - guess it's possible this could result in obj being freed.. */
	ddActionValue_release(val);

	if ( !ddActionObject_isKindOfClass(object, ddActionMovieClipClass) )
		return ddUndefValue;
	
	if ( obj == NULL || clip->movieClip == NULL )
		return ddNullValue;

	xval = ddActionHash_findValue(obj->properties, "x");
	yval = ddActionHash_findValue(obj->properties, "y");

	if ( xval.type == VALUE_NULL || yval.type == VALUE_NULL )
		return ddNullValue;

	x = FIXED_D(ddActionValue_getDoubleValue(xval));
	y = FIXED_D(ddActionValue_getDoubleValue(yval));

	ddDrawClip_localToGlobal(clip->movieClip, &x, &y);

	ddActionHash_setValue(obj->properties, strdup("x"), dd_newActionValue_double(DOUBLE_F(x)));
	ddActionHash_setValue(obj->properties, strdup("y"), dd_newActionValue_double(DOUBLE_F(y)));

	return ddNullValue;
}


static char*
ddActionMovieClip_toString_recurse(ddActionMovieClip* c, char* str)
{
	if ( c->movieClip->parentClip != NULL )
		str = ddActionMovieClip_toString_recurse(c->movieClip->parentClip->actionClip, str);

	return str + sprintf(str, "%s.", c->name);
}


static ddActionValue
ddActionMovieClip_toString(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionMovieClip* clip = (ddActionMovieClip*)object;
	char* name;
	char* p;

	if ( !ddActionObject_isKindOfClass(object, ddActionMovieClipClass) )
		return ddUndefValue;
	
	name = dd_malloc(256); // XXX
	name[0] = '\0';

	p = ddActionMovieClip_toString_recurse(clip, name);

	*--p = '\0';

	return dd_newActionValue_string_noCopy(name);
}


static ddActionValue
ddActionMovieClip_swapDepths(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionMovieClip* clip = (ddActionMovieClip*)object;
	ddActionValue val = ddActionContext_popValue(context);
	int depth;

	if ( !ddActionObject_isKindOfClass(object, ddActionMovieClipClass) )
		return ddUndefValue;
	
	if ( val.type == VALUE_OBJECT && ddActionObject_isKindOfClass(val.data.objectValue, ddActionMovieClipClass) )
	{
		ddActionMovieClip* clip = (ddActionMovieClip*)val.data.objectValue;
		depth = ddDrawable_getDepth((ddDrawable*)clip->movieClip);
	}
	else
		depth = ddActionValue_getIntValue(val);
	
	ddDrawClip_swapDepths(clip->movieClip, depth);
	
	return ddNullValue;
}


static ddActionValue
ddActionMovieClip_createEmptyMovieClip(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue depthval = ddActionContext_popValue(context);
	ddActionValue nameval = ddActionContext_popValue(context);

	char* name = ddActionValue_getStringValue_release(context, nameval);
	int depth = ddActionValue_getIntValue(depthval);

	fprintf(stderr, "createEmptyMovieClip('%s', %i)\n", name, depth);

	ddActionValue_release(depthval);

	return ddNullValue;
}

/*
 static
 getURL
 loadMovie
 loadVariables
 */

static ddActionValue
MovieClip_constructor(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionObject* clipObject = (ddActionObject*)dd_newActionMovieClip(context->player, NULL);
	ddActionObject_setClass(clipObject, ddActionMovieClipClass);

	return dd_newActionValue_object(clipObject);
}


ddActionFunction*
dd_initMovieClipClass()
{
	ddActionFunction* ctor = dd_newActionNativeFunction(MovieClip_constructor, 0);
	
	ddActionObject* classObject;

	ddActionMovieClipClass = dd_newActionClass();
	classObject = (ddActionObject*)ddActionMovieClipClass;

	ddActionMovieClipClass->destroy = dd_destroyActionMovieClip;
	ddActionMovieClipClass->setProperty = ddActionMovieClip_setProperty;
	ddActionMovieClipClass->getProperty = ddActionMovieClip_getProperty;
	
	ddActionObject_addNativeMethod(classObject, strdup("gotoAndPlay"), ddActionMovieClip_gotoAndPlay, 1);
	ddActionObject_addNativeMethod(classObject, strdup("gotoAndStop"), ddActionMovieClip_gotoAndStop, 1);
	ddActionObject_addNativeMethod(classObject, strdup("nextFrame"), ddActionMovieClip_nextFrame, 0);
	ddActionObject_addNativeMethod(classObject, strdup("prevFrame"), ddActionMovieClip_prevFrame, 0);
	ddActionObject_addNativeMethod(classObject, strdup("play"), ddActionMovieClip_play, 0);
	ddActionObject_addNativeMethod(classObject, strdup("stop"), ddActionMovieClip_stop, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getBounds"), ddActionMovieClip_getBounds, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getBytesLoaded"), ddActionMovieClip_getBytesLoaded, 0);
	ddActionObject_addNativeMethod(classObject, strdup("getBytesTotal"), ddActionMovieClip_getBytesTotal, 0);
	ddActionObject_addNativeMethod(classObject, strdup("attachMovie"), ddActionMovieClip_attachMovie, FUNCTION_VARARGS);
	ddActionObject_addNativeMethod(classObject, strdup("startDrag"), ddActionMovieClip_startDrag, FUNCTION_VARARGS);
	ddActionObject_addNativeMethod(classObject, strdup("stopDrag"), ddActionMovieClip_stopDrag, 0);
	ddActionObject_addNativeMethod(classObject, strdup("globalToLocal"), ddActionMovieClip_globalToLocal, 1);
	ddActionObject_addNativeMethod(classObject, strdup("localToGlobal"), ddActionMovieClip_localToGlobal, 1);
	ddActionObject_addNativeMethod(classObject, strdup("duplicateMovieClip"), ddActionMovieClip_duplicateMovieClip, 2);
	ddActionObject_addNativeMethod(classObject, strdup("removeMovieClip"), ddActionMovieClip_removeMovieClip, 0);
	ddActionObject_addNativeMethod(classObject, strdup("unloadMovie"), ddActionMovieClip_unloadMovie, 2);
	ddActionObject_addNativeMethod(classObject, strdup("hitTest"), ddActionMovieClip_hitTest, FUNCTION_VARARGS);
	ddActionObject_addNativeMethod(classObject, strdup("toString"), ddActionMovieClip_toString, 0);
	ddActionObject_addNativeMethod(classObject, strdup("swapDepths"), ddActionMovieClip_swapDepths, 1);
	ddActionObject_addNativeMethod(classObject, strdup("createEmptyMovieClip"), ddActionMovieClip_createEmptyMovieClip, 1);
	
	ddActionFunction_setPrototype(ctor, classObject);

	return ctor;
}
