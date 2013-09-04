/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_DRAWCLIP_H_INCLUDED
#define DD_PLAYER_DRAWCLIP_H_INCLUDED

#include "../dd.h"

typedef struct _ddDrawClip ddDrawClip;

#include "../player/player.h"
#include "drawable.h"
#include "drawbutton.h"
#include "movieclip.h"
#include "displaylist.h"
#include "../action/action.h"
#include "../action/event.h"
#include "../action/classes/movieclip_class.h"
#include "sound.h"
#include "../sound/mp3decoder.h"

/* drawclip is a (drawable) instance of a movie clip */

#define DRAWCLIP_ISPLAYING	(1<<0)
#define DRAWCLIP_STOPPED	(1<<1)
#define DRAWCLIP_ISVISIBLE	(1<<2)

/* when we set a position property (x,y,scale,rotation) with actionscript,
the drawclip no longer responds to the timeline script */

#define DRAWCLIP_FOLLOWTIMELINE	(1<<3)

#define DRAWCLIP_LOADED		(1<<4)
#define DRAWCLIP_STEPPED	(1<<5)
#define DRAWCLIP_UNLOAD		(1<<6)

struct _ddDrawClip
{
	ddDrawable parent;
	
	ddMovieClip* clip;

	int currentFrame;
	int flags;
	
	float xScale;
	float yScale;
	float rotation;
	
	ddMatrix xform;   /* save the matrix and inverse of the last draw */
	ddMatrix inverse; /* so we can swap between coordinate systems */

	ddDisplayList* displayList;
	ddDrawClip* parentClip;
	ddActionMovieClip* actionClip;

	ddAction* onLoad;
	
	ddSoundTransform soundTransform;
	ddMP3Decoder* mp3Decoder;
};


ddDrawClip*
dd_newDrawClip(ddPlayer* player, ddMovieClip* clip);

void
dd_destroyDrawClip(ddDrawable* clip);

ddDrawClip*
ddDrawClip_duplicate(ddPlayer* player, ddDrawClip* clip);

void
ddDrawClip_unlink(ddPlayer* player, ddDrawClip* clip, ddUpdateList* update);

ddMovieClip*
ddDrawClip_getMovieClip(ddDrawClip* clip);

void
ddDrawClip_renderInImage(ddDrawClip* clip, ddImage* image, ddColor background);

/* prepare the next frame for drawing, do actions, etc. */

void
ddDrawClip_step(ddPlayer* player, ddDrawClip* clip);

void
ddDrawClip_advancePlayhead(ddPlayer* player, ddDrawClip* clip);

ddDrawable*
ddDrawClip_addDisplayItem(ddPlayer* player, ddDrawClip* clip, ddCharacter* c,
						  int level, const char* name, int maskLevel);

void
ddDrawClip_swapDepths(ddDrawClip* clip, int depth);

void
ddDrawClip_setMaskShape(ddDrawClip* clip, ddDrawable* mask);

void
ddDrawClip_recalcMatrices(ddDrawClip* clip);

void
ddDrawClip_removeFromParent(ddDrawClip* clip);

int
ddDrawClip_hitTest(ddDrawClip* clip, fixed x, fixed y);

ddDrawButton*
ddDrawClip_findButton(ddDrawClip* clip, fixed x, fixed y);

void
ddDrawClip_gotoFrame(ddPlayer* player, ddDrawClip* clip, int frame);

void
ddDrawClip_nextFrame(ddPlayer* player, ddDrawClip* clip);

void
ddDrawClip_prevFrame(ddPlayer* player, ddDrawClip* clip);

void
ddDrawClip_play(ddPlayer* player, ddDrawClip* clip);

void
ddDrawClip_stop(ddPlayer* player, ddDrawClip* clip);

void
ddDrawClip_stopSounds(ddDrawClip* clip);

void
ddDrawClip_toggleQuality(ddDrawClip* clip);

void
ddDrawClip_gotoFrameNumber(ddPlayer* player, ddDrawClip* clip, int frame);

void
ddDrawClip_gotoFrameLabel(ddPlayer* player, ddDrawClip* clip, char* label);

void
ddDrawClip_setEnterFrameAction(ddDrawClip* clip, ddAction* action);

void
ddDrawClip_setOnLoadAction(ddDrawClip* clip, ddAction* action);

void
ddDrawClip_scheduleClipActions(ddPlayer* player, ddDrawClip* clip, const char* name);

void
ddDrawClip_duplicateClip(ddPlayer* player, ddDrawClip* clip, ddDrawClip* parent, int depth, const char* name);

/* coordinate transform */

void
ddDrawClip_localToGlobal(ddDrawClip* clip, fixed *x, fixed *y);

void
ddDrawClip_globalToLocal(ddDrawClip* clip, fixed *x, fixed *y);


/* properties */

int
ddDrawClip_getBytesLoaded(ddDrawClip* clip);

int
ddDrawClip_getBytesTotal(ddDrawClip* clip);

int
ddDrawClip_getFramesLoaded(ddDrawClip* clip);

int
ddDrawClip_getCurrentFrame(ddDrawClip* clip);

int
ddDrawClip_getTotalFrames(ddDrawClip* clip);

fixed
ddDrawClip_getX(ddDrawClip* clip);

void
ddDrawClip_setX(ddDrawClip* clip, fixed x);

fixed
ddDrawClip_getY(ddDrawClip* clip);

void
ddDrawClip_setY(ddDrawClip* clip, fixed y);

void
ddDrawClip_setXY(ddDrawClip* clip, fixed x, fixed y);

void
ddDrawClip_displaceXY(ddDrawClip* clip, fixed dx, fixed dy);

double
ddDrawClip_getXMouse(ddPlayer* player, ddDrawClip* clip);

double
ddDrawClip_getYMouse(ddPlayer* player, ddDrawClip* clip);

fixed
ddDrawClip_getWidth(ddDrawClip* clip);

void
ddDrawClip_setWidth(ddDrawClip* clip, fixed width);

fixed
ddDrawClip_getHeight(ddDrawClip* clip);

void
ddDrawClip_setHeight(ddDrawClip* clip, fixed height);

double
ddDrawClip_getRotation(ddDrawClip* clip);

void
ddDrawClip_setRotation(ddDrawClip* clip, double rotation);

double
ddDrawClip_getXScale(ddDrawClip* clip);

void
ddDrawClip_setXScale(ddDrawClip* clip, double xScale);

double
ddDrawClip_getYScale(ddDrawClip* clip);

void
ddDrawClip_setYScale(ddDrawClip* clip, double yScale);

double
ddDrawClip_getAlpha(ddDrawClip* clip);

void
ddDrawClip_setAlpha(ddDrawClip* clip, double alpha);

int
ddDrawClip_isVisible(ddDrawClip* clip);

void
ddDrawClip_setVisible(ddDrawClip* clip, int visible);

void
ddDrawClip_setSoundTransform(ddDrawClip* clip, ddSoundTransform xform);

ddSoundTransform
ddDrawClip_getSoundTransform(ddDrawClip* clip);

ddMP3Decoder*
ddDrawClip_getMP3Decoder(ddDrawClip* clip);

static inline int
ddDrawClip_isFollowing(ddDrawClip* clip)
{
	return (clip->flags & DRAWCLIP_FOLLOWTIMELINE) != 0;
}

#ifdef DD_INCLUDE_DEBUGGER

void
ddDrawClip_getDisplayList(ddDrawClip* clip, ddDrawable*** outDrawables, int* outCount);

ddActionObject*
ddDrawClip_getActionObject(ddDrawClip* clip);

#endif

#endif /* DD_PLAYER_DRAWSHAPE_H_INCLUDED */
