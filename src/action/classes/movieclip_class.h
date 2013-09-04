/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_CLASSES_MOVIECLIP_H_INCLUDED
#define DD_ACTION_CLASSES_MOVIECLIP_H_INCLUDED

/*
 methods:

 attachMovie
 duplicateMovieClip
 getBounds
 getBytesLoaded
 getBytesTotal
 getURL
 globalToLocal
 gotoAndPlay
 gotoAndStop
 hitTest
 loadMovie
 loadVariables
 localToGlobal
 nextFrame
 play
 prevFrame
 removeMovieClip
 startDrag
 stop
 stopDrag
 swapDepths
 unloadMovie

 // SWF6
 clip.createEmptyMovieClip(name, depth) - creates empty subclip
 clip.createTextField(instanceName, depth, x, y, width, height)
 getDepth
 setMask(maskClip)

 // drawing
 beginFill
 beginGradientFill
 clear
 curveTo(x1, y1, x2, y2)
 endFill
 lineStyle(thickness[, color[, alpha]])
 lineTo(x, y)
 moveTo(x, y)
 */

typedef struct _ddActionMovieClip ddActionMovieClip;

#include "../../dd.h"
#include "../object.h"
#include "../class.h"
#include "../../player/drawclip.h"
#include "../../player/drawtextfield.h"

struct _ddActionMovieClip
{
	ddActionObject parent;

	ddDrawClip* movieClip;
	char* name;
	ddActionMovieClip* rootClip;
};


typedef enum
{
	PROPERTY_X              = 0,
	PROPERTY_Y              = 1,
	PROPERTY_XSCALE         = 2,
	PROPERTY_YSCALE         = 3,
	PROPERTY_CURRENTFRAME   = 4,
	PROPERTY_TOTALFRAMES    = 5,
	PROPERTY_ALPHA          = 6,
	PROPERTY_VISIBLE        = 7,
	PROPERTY_WIDTH          = 8,
	PROPERTY_HEIGHT         = 9,
	PROPERTY_ROTATION       = 10,
	PROPERTY_TARGET         = 11,
	PROPERTY_FRAMESLOADED   = 12,
	PROPERTY_NAME           = 13,
	PROPERTY_DROPTARGET     = 14,
	PROPERTY_URL            = 15,
	PROPERTY_HIGHQUALITY    = 16,
	PROPERTY_FOCUSRECT      = 17,
	PROPERTY_SOUNDBUFTIME   = 18,
	PROPERTY_QUALITY        = 19,
	PROPERTY_XMOUSE         = 20,
	PROPERTY_YMOUSE         = 21
} ddClipProperty;


extern ddActionClass* ddActionMovieClipClass;


ddActionMovieClip*
dd_newActionMovieClip(ddPlayer* player, ddDrawClip* clip);

void
ddActionMovieClip_releaseMovieClip(ddActionMovieClip* clip);

void
ddActionMovieClip_setProperty(ddActionContext* context, ddActionObject* object,
							  char* name, ddActionValue value);

ddActionValue
ddActionMovieClip_getProperty(ddActionContext* context, ddActionObject* object, const char* name);

ddActionValue
ddActionMovieClip_getPropertyNum(ddPlayer* player, ddActionMovieClip* clip, ddClipProperty num);

void
ddActionMovieClip_setPropertyNum(ddActionMovieClip* clip,
				 ddClipProperty num, ddActionValue value);

void
ddActionMovieClip_setName(ddActionMovieClip* clip, char* name);

char*
ddActionMovieClip_getName(ddActionMovieClip* clip);

void
ddActionMovieClip_addChild(ddActionMovieClip* parent, ddActionMovieClip* child);

void
ddActionMovieClip_removeChild(ddActionMovieClip* parent, ddActionMovieClip* child);

void
ddActionMovieClip_addTextField(ddActionMovieClip* clip, ddDrawTextField* field);

void
ddActionMovieClip_removeTextField(ddActionMovieClip* clip, ddDrawTextField* field);

ddActionFunction*
dd_initMovieClipClass();

void
ddActionMovieClip_gotoFrameLabel(ddActionContext* context,
								 ddActionMovieClip* clip, char* label, boolean play);

ddDrawClip*
ddActionMovieClip_getDrawClip(ddActionMovieClip* clip);

/* methods */

ddActionValue
ddActionMovieClip_gotoAndPlay(ddActionObject* object,
							  ddActionContext* context, int nargs);

ddActionValue
ddActionMovieClip_gotoAndStop(ddActionObject* object,
							  ddActionContext* context, int nargs);

ddActionValue
ddActionMovieClip_stop(ddActionObject* object,
					   ddActionContext* context, int nargs);

ddActionValue
ddActionMovieClip_play(ddActionObject* object,
					   ddActionContext* context, int nargs);

ddActionValue
ddActionMovieClip_prevFrame(ddActionObject* object,
							ddActionContext* context, int nargs);

ddActionValue
ddActionMovieClip_nextFrame(ddActionObject* object,
							ddActionContext* context, int nargs);


#endif /* DD_ACTION_CLASSES_MOVIECLIP_H_INCLUDED */
