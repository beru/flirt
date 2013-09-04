/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_DRAWBUTTON_H_INCLUDED
#define DD_PLAYER_DRAWBUTTON_H_INCLUDED

#include "../dd.h"

typedef struct _ddDrawButton ddDrawButton;

#include "button.h"
#include "timeline.h"
#include "drawable.h"
#include "drawclip.h"
#include "displaylist.h"
#include "../render/shapeinstance.h"

struct _ddDrawButton
{
	ddDrawable parent;

	ddButton* button;

	int buttonDown;
	int frame;

	ddDrawClip* parentClip; /* button actions execute in the parent's context */
	ddDisplayList* displayList;
	ddDisplayList* hitList;
};


ddDrawButton*
dd_newDrawButton(ddPlayer* player, ddButton* button);

ddDrawable*
ddDrawButton_addDisplayItem(ddPlayer* player, ddDrawButton* b, ddCharacter* c,
							int level, int maskdepth);

void
ddDrawButton_setParentClip(ddDrawButton* b, ddDrawClip* clip);

void
ddDrawButton_unlink(ddPlayer* player, ddDrawButton* b, ddUpdateList* list);

void
ddDrawButton_gotoUpState(ddPlayer* player, ddDrawButton* b);

void
ddDrawButton_gotoDownState(ddPlayer* player, ddDrawButton* b);

void
ddDrawButton_gotoOverState(ddPlayer* player, ddDrawButton* b);

int
ddDrawButton_hitTest(ddDrawButton* b, fixed x, fixed y);

void
ddDrawButton_outUpToOverUp(ddPlayer* player, ddDrawButton* b);

void
ddDrawButton_overDownToOverUp(ddPlayer* player, ddDrawButton* b);

void
ddDrawButton_outDownToOutUp(ddPlayer* player, ddDrawButton* b);

void
ddDrawButton_overUpToOverDown(ddPlayer* player, ddDrawButton* b);

void
ddDrawButton_overUpToOutUp(ddPlayer* player, ddDrawButton* b);

void
ddDrawButton_overDownToOutDown(ddPlayer* player, ddDrawButton* b);

void
ddDrawButton_outDownToOverDown(ddPlayer* player, ddDrawButton* b);

#endif /* DD_PLAYER_DRAWBUTTON_H_INCLUDED */
