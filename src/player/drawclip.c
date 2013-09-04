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
#include <stdio.h>

#include "drawclip.h"
#include "../action/function.h"

static void
ddDrawClip_getUpdateList(ddDrawable* d, ddUpdateList* list, ddMatrix matrix)
{
	ddDrawClip* clip = (ddDrawClip*)d;

	ddDrawClip_recalcMatrices(clip);

	clip->flags &= ~DRAWCLIP_STEPPED;

	if ( (clip->flags & DRAWCLIP_ISVISIBLE) && clip->displayList != NULL )
	{
		ddMatrix m = ddMatrix_multiply(matrix, d->matrix);
		
		m = ddMatrix_scale(m, FIXED_D(clip->xScale), FIXED_D(clip->yScale));
		m = ddMatrix_rotate(m, clip->rotation);

		ddDisplayList_getUpdateList(clip->displayList, list, m);
		d->bounds = clip->displayList->bounds;
	}
	else
		d->bounds = ddInvalidRect;
}


static void
ddDrawClip_drawInImage(ddDrawable* d, ddImage* image, ddUpdateList* list,
					   ddMatrix matrix, ddCXform cXform, ddRect clipRect)
{
	ddDrawClip* clip = (ddDrawClip*)d;

	if ( (clip->flags & DRAWCLIP_ISVISIBLE) && clip->displayList != NULL )
	{
		//if ( !ddUpdateList_intersectsRect(list, d->bounds) )
		//	return;

		ddMatrix m = ddMatrix_multiply(matrix, d->matrix);
		ddCXform c = ddCXform_compose(cXform, d->cXform);
		
		m = ddMatrix_scale(m, FIXED_D(clip->xScale), FIXED_D(clip->yScale));
		m = ddMatrix_rotate(m, clip->rotation);

		ddDisplayList_drawInImage(clip->displayList, image, list, m, c, clipRect);
	}
}


void
dd_destroyDrawClip(ddDrawable* d)
{
	ddDrawClip* clip = (ddDrawClip*)d;

	if ( clip->displayList != NULL )
		dd_destroyDisplayList(clip->displayList);
	
	if ( clip->actionClip != NULL )
	{
		if ( clip->parentClip != NULL )
			ddActionMovieClip_removeChild(clip->parentClip->actionClip, clip->actionClip);

		ddActionMovieClip_releaseMovieClip(clip->actionClip);
		ddActionObject_release((ddActionObject*)clip->actionClip);
	}

	if ( clip->mp3Decoder != NULL )
	{
		dd_destroyMP3Decoder(clip->mp3Decoder);
		clip->mp3Decoder = NULL;
	}
	
	dd_free(d);
}


void
ddDrawClip_init(ddDrawClip* clip)
{
	ddDrawable_init((ddDrawable*)clip);
	
	clip->parent.type = MOVIECLIP_CHAR;
	clip->parent.destroy = dd_destroyDrawClip;
	clip->parent.getUpdateList = ddDrawClip_getUpdateList;
	clip->parent.drawInImage = ddDrawClip_drawInImage;
	
	clip->displayList = dd_newDisplayList();
	
	clip->xScale = 1.0;
	clip->yScale = 1.0;
	clip->rotation = 0;
	
	clip->xform = ddEmptyMatrix;
	clip->inverse = ddEmptyMatrix;
	
	clip->clip = NULL;
	clip->parentClip = NULL;
	clip->currentFrame = 0;
	clip->flags = 0;
	clip->onLoad = NULL;
	
	clip->soundTransform = dd_newSoundTransform(1.0, 0, 0, 1.0);
	clip->mp3Decoder = NULL;
}


ddDrawClip*
dd_newDrawClip(ddPlayer* player, ddMovieClip* clip)
{
	ddDrawClip* c = dd_malloc(sizeof(ddDrawClip));
	ddDrawClip_init(c);
	
	c->clip = clip;
	c->flags = DRAWCLIP_ISPLAYING | DRAWCLIP_ISVISIBLE | DRAWCLIP_FOLLOWTIMELINE;
	c->actionClip = dd_newActionMovieClip(player, c);
	
	ddPlayer_addActiveClip(player, c);

	return c;
}


ddDrawClip*
dd_newDrawClip_empty(ddPlayer* player)
{
	ddDrawClip* c = dd_malloc(sizeof(ddDrawClip));
	ddDrawClip_init(c);

	c->flags = DRAWCLIP_ISVISIBLE;
	c->actionClip = dd_newActionMovieClip(player, c);

	ddPlayer_addActiveClip(player, c);

	return c;
}


void
ddDrawClip_renderInImage(ddDrawClip* clip, ddImage* image, ddColor background)
{
	ddUpdateList* update = dd_newUpdateList();
	ddRect bounds = dd_makeRect(0, FIXED_I(image->width), 0, FIXED_I(image->height));

	ddUpdateList_includeRect(update, bounds);

	ddDrawable_getUpdateList((ddDrawable*)clip, update, ddIdentityMatrix);

#ifndef DD_RENDER_TOPDOWN
	ddImage_fillRect(image, bounds, background);
#endif

	ddDrawable_drawInImage((ddDrawable*)clip, image, update, ddIdentityMatrix, ddEmptyCXform, bounds);

#ifdef DD_RENDER_TOPDOWN
	ddImage_blendRect(image, bounds, background);
#endif

	dd_destroyUpdateList(update);
}


ddDrawClip*
ddDrawClip_duplicate(ddPlayer* player, ddDrawClip* clip)
{
	return dd_newDrawClip(player, clip->clip);
}


void
ddDrawClip_unlink(ddPlayer* player, ddDrawClip* clip, ddUpdateList* list)
{
	dd_log("%p unlinking", clip);

	if ( (clip->flags & DRAWCLIP_ISPLAYING) != 0 )
		ddPlayer_removeActiveClip(player, clip);

	ddPlayer_removeClipEventHandlers(player, clip);

	ddDisplayList_unlink(player, clip->displayList, list);
}


ddMovieClip*
ddDrawClip_getMovieClip(ddDrawClip* clip)
{
	return clip->clip;
}


void
ddDrawClip_advancePlayhead(ddPlayer* player, ddDrawClip* clip)
{
	if ( (clip->flags & DRAWCLIP_ISPLAYING) == 0 )
		return;

	if ( (clip->flags & DRAWCLIP_LOADED) != 0 )
		++clip->currentFrame;

	// if clip has > 1 frame, loop to beginning after we step off the end

	if ( clip->clip->framesTotal == 1 )
		clip->flags |= DRAWCLIP_STOPPED;

	if ( clip->currentFrame >= clip->clip->framesTotal )
		clip->currentFrame = 0;
}


void
ddDrawClip_step(ddPlayer* player, ddDrawClip* clip)
{
	if ( clip->flags & DRAWCLIP_STEPPED )
		return;
	
	clip->flags |= DRAWCLIP_STEPPED;
	
	if ( (clip->flags & DRAWCLIP_LOADED) == 0 )
	{
		if ( clip->onLoad != NULL )
			ddPlayer_addFrameAction(player, clip->onLoad, (ddActionObject*)clip->actionClip);
		
		// XXX - if 'onLoad' property is a function, call it
		// (how could it be set before the clip's loaded?)
		
		clip->flags |= DRAWCLIP_LOADED;
	}

	ddTimeline_updateClipDisplay(player, clip->clip->timeline, clip);
	ddTimeline_playFrameSounds(player, clip->clip->timeline, clip);
	ddTimeline_scheduleActions(player, clip->clip->timeline, clip, clip->currentFrame);
}


void
ddDrawClip_recalcMatrices(ddDrawClip* clip)
{
	ddMatrix m;
	
	if ( clip->parentClip != NULL )
		m = ddMatrix_multiply(clip->parentClip->xform, clip->parent.matrix);
	else
		m = clip->parent.matrix;
	
	m = ddMatrix_rotate(m, clip->rotation);
	m = ddMatrix_scale(m, FIXED_D(clip->xScale), FIXED_D(clip->yScale));
	
	clip->xform = m;
	clip->inverse = ddMatrix_invert(m);
}


int
ddDrawClip_hitTest(ddDrawClip* clip, fixed x, fixed y)
{
	return ddDisplayList_hitTest(clip->displayList, x, y);
}


ddDrawButton*
ddDrawClip_findButton(ddDrawClip* clip, fixed x, fixed y)
{
	ddDrawable* d = clip->displayList->drawablesTail;
	
	while ( d != NULL )
	{
		if ( d->type == MOVIECLIP_CHAR )
		{
			ddDrawButton* b = ddDrawClip_findButton((ddDrawClip*)d, x, y);
			
			if ( b != NULL )
				return b;
		}
		else if ( d->type == BUTTON_CHAR && ddDrawButton_hitTest((ddDrawButton*)d, x, y) )
			return (ddDrawButton*)d;
		
		d = d->last;
	}
	
	return NULL;
}


void
ddDrawClip_setOnLoadAction(ddDrawClip* clip, ddAction* action)
{
	clip->onLoad = action;
}


void
ddDrawClip_scheduleClipActions(ddPlayer* player, ddDrawClip* clip, const char* name)
{
	ddActionObject* clipobj = (ddActionObject*)clip->actionClip;
	ddActionFunction* func = ddActionObject_getFunctionNamed(clipobj, name);
	ddAction* action = ddActionFunction_getBytecodeAction(func);

	ddDrawable* child;

	if ( action != NULL )
		ddPlayer_addFrameAction(player, action, clipobj);
	
	child = clip->displayList->drawablesHead;
	
	while ( child != NULL )
	{
		if ( child->type == MOVIECLIP_CHAR )
			ddDrawClip_scheduleClipActions(player, (ddDrawClip*)child, name);
		
		child = child->next;
	}
}


void
ddDrawClip_swapDepths(ddDrawClip* clip, int depth)
{
	if ( clip->parentClip == NULL )
		dd_warn("swapDepths called on clip w/out parent!");
	else
		ddDisplayList_swapDepths(clip->parentClip->displayList, ddDrawable_getDepth((ddDrawable*)clip), depth);
}


void
ddDrawClip_gotoFrame(ddPlayer* player, ddDrawClip* clip, int frame)
{
	if ( frame == clip->currentFrame )
		return;
	
	if ( frame >= clip->clip->framesLoaded || frame < 0 )
		frame = 0;

	clip->currentFrame = frame;

	ddTimeline_updateClipDisplay(player, clip->clip->timeline, clip);
	ddTimeline_playFrameSounds(player, clip->clip->timeline, clip);
	ddTimeline_scheduleActions(player, clip->clip->timeline, clip, frame);
}


void
ddDrawClip_nextFrame(ddPlayer* player, ddDrawClip* clip)
{
	ddDrawClip_gotoFrame(player, clip, clip->currentFrame + 1);
	ddDrawClip_stop(player, clip);
}


void
ddDrawClip_prevFrame(ddPlayer* player, ddDrawClip* clip)
{
	ddDrawClip_gotoFrame(player, clip, clip->currentFrame - 1);
	ddDrawClip_stop(player, clip);
}


void
ddDrawClip_play(ddPlayer* player, ddDrawClip* clip)
{
	if ( clip->flags & DRAWCLIP_ISPLAYING )
	{
		clip->flags &= ~DRAWCLIP_STOPPED;
		return;
	}

	clip->flags |= DRAWCLIP_ISPLAYING;
	clip->flags &= ~DRAWCLIP_STOPPED;

	// XXX - ???
	clip->flags &= ~DRAWCLIP_STEPPED;
	
	ddPlayer_addActiveClip(player, clip);
}


void
ddDrawClip_stop(ddPlayer* player, ddDrawClip* clip)
{
	if ( (clip->flags & DRAWCLIP_ISPLAYING) == 0 )
		return;

	clip->flags |= DRAWCLIP_STOPPED;
	//clip->flags &= ~DRAWCLIP_ISPLAYING;
}


void
ddDrawClip_stopSounds(ddDrawClip* clip)
{
	dd_warn("Not implemented");
}


void
ddDrawClip_toggleQuality(ddDrawClip* clip)
{
	dd_warn("Not implemented");
}


void
ddDrawClip_gotoFrameLabel(ddPlayer* player, ddDrawClip* clip, char* label)
{
	ddDrawClip_gotoFrame(player, clip, ddMovieClip_findFrameLabel(clip->clip, label));
}


void
ddDrawClip_setMaskShape(ddDrawClip* clip, ddDrawable* mask)
{
	ddDisplayList_setMaskDrawable(clip->displayList, mask);
}


ddDrawable*
ddDrawClip_addDisplayItem(ddPlayer* player, ddDrawClip* clip, ddCharacter* c, int level, const char* name, int maskLevel)
{
	ddDrawable* d;
	char* clipname;

	if ( c == NULL )
		return NULL;

	d = ddDisplayList_addCharacter(player, clip->displayList, c, level, maskLevel);

	if ( d != NULL )
	{
		if ( clip->parent.maskShape != NULL )
			ddDrawable_setMaskDrawable(d, clip->parent.maskShape);
		
		if ( c->type == MOVIECLIP_CHAR )
		{
			ddDrawClip* newclip = (ddDrawClip*)d;

			dd_log("%p adding clip %p", clip, d);

			if ( name == NULL )
			{
				clipname = dd_malloc(16); /* "instancexxxxx" */
				snprintf(clipname, 16, "instance%i", level);
			}
			else
				clipname = strdup(name);

			newclip->parentClip = clip;

			ddActionMovieClip_setName(newclip->actionClip, clipname);
			ddActionMovieClip_addChild(clip->actionClip, newclip->actionClip);
		}
		else if ( c->type == BUTTON_CHAR )
		{
			dd_log("%p adding button %p", clip, d);

			ddDrawButton_setParentClip((ddDrawButton*)d, clip);
			//ddPlayer_addActiveButton(player, (ddDrawButton*)d);
		}
		else if ( c->type == TEXTFIELD_CHAR )
			ddActionMovieClip_addTextField(clip->actionClip, (ddDrawTextField*)d);
	}

	return d;
}


void
ddDrawClip_duplicateClip(ddPlayer* player, ddDrawClip* clip, ddDrawClip* parent, int depth, const char* name)
{
	ddDrawClip* newclip = (ddDrawClip*)ddDrawClip_addDisplayItem(player, parent, (ddCharacter*)clip->clip, depth, name, -1);
	
	newclip->onLoad = clip->onLoad;
	newclip->parent.matrix = clip->parent.matrix;
	newclip->parent.bounds = clip->parent.bounds;
	newclip->parent.cXform = clip->parent.cXform;
	
	ddDrawClip_step(player, newclip);
}


/* coordinate transform */

void
ddDrawClip_localToGlobal(ddDrawClip* clip, fixed *x, fixed *y)
{
	ddMatrix_apply(clip->xform, x, y);
}


void
ddDrawClip_globalToLocal(ddDrawClip* clip, fixed *x, fixed *y)
{
	ddMatrix_apply(clip->inverse, x, y);
}


/* property accessors */

int
ddDrawClip_getBytesLoaded(ddDrawClip* clip)
{
	return ddMovieClip_getBytesLoaded(clip->clip);
}


int
ddDrawClip_getBytesTotal(ddDrawClip* clip)
{
	return ddMovieClip_getBytesTotal(clip->clip);
}


int
ddDrawClip_getFramesLoaded(ddDrawClip* clip)
{
	return ddMovieClip_getFramesLoaded(clip->clip);
}


int
ddDrawClip_getCurrentFrame(ddDrawClip* clip)
{
	return clip->currentFrame;
}


int
ddDrawClip_getTotalFrames(ddDrawClip* clip)
{
	return clip->clip->framesTotal;
}


fixed
ddDrawClip_getWidth(ddDrawClip* clip)
{
	return ddRect_getWidth(clip->parent.bounds);
}


void
ddDrawClip_setWidth(ddDrawClip* clip, fixed width)
{
	fixed clipwidth = ddRect_getWidth(clip->parent.bounds);

	if ( clipwidth == 0 )
		return;

	ddDrawClip_setXScale(clip, (double)width/clipwidth);
}


fixed
ddDrawClip_getHeight(ddDrawClip* clip)
{
	return ddRect_getHeight(clip->parent.bounds);
}


void
ddDrawClip_setHeight(ddDrawClip* clip, fixed height)
{
	fixed clipheight = ddRect_getHeight(clip->parent.bounds);

	if ( clipheight == 0 )
		return;

	ddDrawClip_setYScale(clip, (double)height/clipheight);
}


fixed
ddDrawClip_getX(ddDrawClip* clip)
{
	return clip->parent.matrix.dx;
}


void
ddDrawClip_setX(ddDrawClip* clip, fixed x)
{
	clip->flags &= ~DRAWCLIP_FOLLOWTIMELINE;
	clip->parent.matrix.dx = x;
	ddDrawClip_recalcMatrices(clip);
}


fixed
ddDrawClip_getY(ddDrawClip* clip)
{
	return clip->parent.matrix.dy;
}


void
ddDrawClip_setY(ddDrawClip* clip, fixed y)
{
	clip->flags &= ~DRAWCLIP_FOLLOWTIMELINE;
	clip->parent.matrix.dy = y;
	ddDrawClip_recalcMatrices(clip);
}


void
ddDrawClip_setXY(ddDrawClip* clip, fixed x, fixed y)
{
	clip->flags &= ~DRAWCLIP_FOLLOWTIMELINE;
	clip->parent.matrix.dx = x;
	clip->parent.matrix.dy = y;
	ddDrawClip_recalcMatrices(clip);
}


void
ddDrawClip_displaceXY(ddDrawClip* clip, fixed dx, fixed dy)
{
	clip->flags &= ~DRAWCLIP_FOLLOWTIMELINE;
	clip->parent.matrix.dx += dx;
	clip->parent.matrix.dy += dy;
	ddDrawClip_recalcMatrices(clip);
}


double
ddDrawClip_getXMouse(ddPlayer* player, ddDrawClip* clip)
{
	fixed x = FIXED_I(player->xmouse);
	fixed y = FIXED_I(player->ymouse);

	ddMatrix_apply(clip->inverse, &x, &y);
	return DOUBLE_F(x);
}


double
ddDrawClip_getYMouse(ddPlayer* player, ddDrawClip* clip)
{
	fixed x = FIXED_I(player->xmouse);
	fixed y = FIXED_I(player->ymouse);

	ddMatrix_apply(clip->inverse, &x, &y);
	return DOUBLE_F(y);
}


double
ddDrawClip_getRotation(ddDrawClip* clip)
{
	return clip->rotation + atan2(clip->parent.matrix.c, clip->parent.matrix.a);
}


void
ddDrawClip_setRotation(ddDrawClip* clip, double newrot)
{
	clip->rotation = newrot - atan2(clip->parent.matrix.c, clip->parent.matrix.a);
	clip->flags &= ~DRAWCLIP_FOLLOWTIMELINE;

	ddDrawClip_recalcMatrices(clip);
}


double
ddDrawClip_getXScale(ddDrawClip* clip)
{
	double a = DOUBLE_F(clip->parent.matrix.a);
	double c = DOUBLE_F(clip->parent.matrix.c);

	return hypot(a, c) * clip->xScale;
}


void
ddDrawClip_setXScale(ddDrawClip* clip, double xScale)
{
	double a = DOUBLE_F(clip->parent.matrix.a);
	double c = DOUBLE_F(clip->parent.matrix.c);
	
	clip->xScale = xScale / hypot(a, c);
	clip->flags &= ~DRAWCLIP_FOLLOWTIMELINE;

	ddDrawClip_recalcMatrices(clip);
}


double
ddDrawClip_getYScale(ddDrawClip* clip)
{
	double b = DOUBLE_F(clip->parent.matrix.b);
	double d = DOUBLE_F(clip->parent.matrix.d);

	return clip->yScale * hypot(b, d);
}


void
ddDrawClip_setYScale(ddDrawClip* clip, double yScale)
{
	double b = DOUBLE_F(clip->parent.matrix.b);
	double d = DOUBLE_F(clip->parent.matrix.d);
	
	clip->yScale = yScale / hypot(b, d);
	clip->flags &= ~DRAWCLIP_FOLLOWTIMELINE;

	ddDrawClip_recalcMatrices(clip);
}


double
ddDrawClip_getAlpha(ddDrawClip* clip)
{
	return clip->parent.cXform.aMult / 255.0;
}


void
ddDrawClip_setAlpha(ddDrawClip* clip, double alpha)
{
	clip->flags &= ~DRAWCLIP_FOLLOWTIMELINE;

	// XXX - set visible flag?
	clip->parent.cXform.aMult = (int)rint(255 * alpha);
}


boolean
ddDrawClip_isVisible(ddDrawClip* clip)
{
	// XXX - look at alpha too?
	return (clip->flags & DRAWCLIP_ISVISIBLE) != 0;
}


void
ddDrawClip_setVisible(ddDrawClip* clip, boolean visible)
{
	if ( visible )
		clip->flags |= DRAWCLIP_ISVISIBLE;
	else
		clip->flags &= ~DRAWCLIP_ISVISIBLE;
}


void
ddDrawClip_setSoundTransform(ddDrawClip* clip, ddSoundTransform xform)
{
	clip->soundTransform = xform;
}


ddSoundTransform
ddDrawClip_getSoundTransform(ddDrawClip* clip)
{
	return clip->soundTransform;
}


ddMP3Decoder*
ddDrawClip_getMP3Decoder(ddDrawClip* clip)
{
	if ( clip->mp3Decoder == NULL )
		clip->mp3Decoder = dd_newMP3Decoder();
	
	return clip->mp3Decoder;
}

#ifdef DD_INCLUDE_DEBUGGER

void
ddDrawClip_getDisplayList(ddDrawClip* clip, ddDrawable*** outItems, int* outNItems)
{
	ddDrawable* d = clip->displayList->drawablesHead;
	ddDrawable** list = NULL;
	int count = 0;
	
	while ( d != NULL )
	{
		list = dd_realloc(list, (count + 1) * sizeof(ddDrawable*));
		list[count] = d;
		++count;
		d = d->next;
	}
	
	*outItems = list;
	*outNItems = count;
}

ddActionObject*
ddDrawClip_getActionObject(ddDrawClip* clip)
{
	return (ddActionObject*)clip->actionClip;
}

#endif
