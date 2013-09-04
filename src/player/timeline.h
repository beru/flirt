/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_TIMELINE_H_INCLUDED
#define DD_PLAYER_TIMELINE_H_INCLUDED

#include "../dd.h"

typedef struct _ddTimeline ddTimeline;
typedef struct _ddTimelineFrame ddTimelineFrame;
typedef struct _ddTimelineItem ddTimelineItem;

#include "character.h"
#include "displaylist.h"
#include "drawclip.h"
#include "drawbutton.h"
#include "sound.h"
#include "../render/matrix.h"
#include "../render/cxform.h"
#include "../action/action.h"

#define DDEVENTHANDLER_ONLOAD      0
#define DDEVENTHANDLER_ENTERFRAME  1
#define DDEVENTHANDLER_UNLOAD      2
#define DDEVENTHANDLER_MOUSEMOVE   3
#define DDEVENTHANDLER_MOUSEDOWN   4
#define DDEVENTHANDLER_MOUSEUP     5
#define DDEVENTHANDLER_KEYDOWN     6
#define DDEVENTHANDLER_KEYUP       7
#define DDEVENTHANDLER_DATA        8

#define DD_NUMEVENTHANDLERS 9

struct _ddTimelineItem
{
#ifdef DD_INCLUDE_DEBUGGER
	int count;
#endif
	
	ddTimelineItem* next;
	int level;
	int ratio;
	int maskdepth;
	char* name;
	ddCharacter* character;
	ddMatrix matrix;
	ddCXform cXform;
	ddAction* actions[DD_NUMEVENTHANDLERS];
};

struct _ddTimelineFrame
{
	ddTimelineItem* items;
	ddAction* actions;
	ddAction* actiontails;
	ddSoundStreamBlock* streamblock;
	ddSoundInstance* sounds;
};

struct _ddTimeline
{
	int nFrames;
	ddTimelineFrame* frames;
};


ddTimeline*
dd_newTimeline();

ddTimeline*
dd_newButtonTimeline();

void
dd_destroyTimeline(ddTimeline* t);

void
ddTimeline_placeCharacter(ddTimeline* timeline, ddCharacter* c, int depth,
						  ddMatrix matrix, ddCXform cXform, int ratio,
						  int maskdepth, char* name);

void
ddTimeline_placeButtonCharacter(ddTimeline* timeline, ddCharacter* c,
								int frame, int depth, ddMatrix matrix,
								ddCXform cXform);

void
ddTimeline_removeCharacter(ddTimeline* timeline, ddCharacter* c, int depth);

ddTimelineItem*
ddTimeline_findItem(ddTimeline* timeline, int depth);

ddTimelineItem*
ddTimeline_addItem(ddTimeline* timeline, int depth, int frame);

void
ddTimeline_removeItem(ddTimeline* timeline, int depth);

void
ddTimeline_addAction(ddTimeline* timeline, ddAction* action);

void
ddTimeline_scheduleActions(ddPlayer* player, ddTimeline* timeline, ddDrawClip* clip, int frameNum);

void
ddTimeline_addSoundInstance(ddTimeline* timeline, ddSoundInstance* sound);

void
ddTimeline_addSoundStreamBlock(ddTimeline* timeline, ddSoundStreamBlock* block);

ddSoundStreamBlock*
ddTimelineFrame_getSoundStreamBlock(ddTimelineFrame* frame);

void
ddTimeline_nextFrame(ddTimeline* timeline);

void
ddTimeline_updateClipDisplay(ddPlayer* player, ddTimeline* timeline, ddDrawClip* clip);

void
ddTimeline_updateButtonDisplay(ddPlayer* player, ddTimeline* timeline, ddDrawButton* button, int frame);

void
ddTimeline_playFrameSounds(ddPlayer* player, ddTimeline* timeline, ddDrawClip* clip);

#endif /* DD_PLAYER_TIMELINE_H_INCLUDED */
