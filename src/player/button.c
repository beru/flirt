/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "button.h"
#include "drawbutton.h"
#include "character.h"

static void
dd_destroyButton(ddCharacter* c)
{
	int i;

	ddButton* button = (ddButton*)c;

	for ( i = 0; i < button->nActions; ++i )
		dd_destroyAction(button->actions[i].action);

	if ( button->actions != NULL )
		dd_free(button->actions);

	dd_destroyTimeline(button->timeline);

	dd_free(c);
}


static ddDrawable*
ddButton_instantiate(ddPlayer* player, ddCharacter* c)
{
	return (ddDrawable*)dd_newDrawButton(player, (ddButton*)c);
}


ddButton*
dd_newButton()
{
	ddButton* button = dd_malloc(sizeof(ddButton));
	button->parent.type = BUTTON_CHAR;
	button->parent.instantiate = ddButton_instantiate;
	button->parent.destroy = dd_destroyButton;

	button->timeline = dd_newButtonTimeline();

	button->nActions = 0;
	button->actions = NULL;

	button->sounds[0] = NULL;
	button->sounds[1] = NULL;
	button->sounds[2] = NULL;
	button->sounds[3] = NULL;

	return button;
}


void
ddButton_addCharacter(ddButton* button, ddCharacter* character, int depth,
					  ddMatrix matrix, ddCXform cXform, int flags)
{
	if ( flags & DDBUTTON_HITFLAG )
	{
		ddTimeline_placeButtonCharacter(button->timeline, character,
								  DDBUTTON_HITFRAME, depth, matrix, cXform);
	}

	if ( flags & DDBUTTON_UPFLAG )
	{
		ddTimeline_placeButtonCharacter(button->timeline, character,
								  DDBUTTON_UPFRAME, depth, matrix, cXform);
	}

	if ( flags & DDBUTTON_DOWNFLAG )
	{
		ddTimeline_placeButtonCharacter(button->timeline, character,
								  DDBUTTON_DOWNFRAME, depth, matrix, cXform);
	}

	if ( flags & DDBUTTON_OVERFLAG )
	{
		ddTimeline_placeButtonCharacter(button->timeline, character,
								  DDBUTTON_OVERFRAME, depth, matrix, cXform);
	}
}


#define BUTTONACTION_INCREMENT 8

void
ddButton_addAction(ddButton* button, ddAction* action, int flags)
{
	if ( button->nActions % BUTTONACTION_INCREMENT == 0 )
	{
		button->actions = dd_realloc(button->actions,
							   (button->nActions + BUTTONACTION_INCREMENT) *
							   sizeof(struct _ddButtonAction));
	}

	button->actions[button->nActions].action = action;
	button->actions[button->nActions].flags = flags;

	++button->nActions;
}


void
ddButton_addSoundInstance(ddButton* button, ddSoundInstance* sound, int flags)
{
	if ( flags == DDBUTTON_OVERUPTOIDLE )
		button->sounds[DDBUTTONSOUND_OVERUPTOIDLE] = sound;
	else if ( flags == DDBUTTON_IDLETOOVERUP )
		button->sounds[DDBUTTONSOUND_IDLETOOVERUP] = sound;
	else if ( flags == DDBUTTON_OVERUPTOOVERDOWN ) 
		button->sounds[DDBUTTONSOUND_OVERUPTOOVERDOWN] = sound;
	else if ( flags == DDBUTTON_OVERDOWNTOOVERUP )
		button->sounds[DDBUTTONSOUND_OVERDOWNTOOVERUP] = sound;
	else
		; // something
}
