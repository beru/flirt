/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_BUTTON_H_INCLUDED
#define DD_PLAYER_BUTTON_H_INCLUDED

typedef struct _ddButton ddButton;

#include "character.h"
#include "timeline.h"
#include "sound.h"
#include "../action/action.h"

/* action flags */
#define DDBUTTON_KEYPRESS           0xfe00
#define DDBUTTON_OVERDOWNTOIDLE     0x100
#define DDBUTTON_IDLETOOVERDOWN     0x080
#define DDBUTTON_OUTDOWNTOIDLE      0x040
#define DDBUTTON_OUTDOWNTOOVERDOWN  0x020
#define DDBUTTON_OVERDOWNTOOUTDOWN  0x010
#define DDBUTTON_OVERDOWNTOOVERUP   0x008
#define DDBUTTON_OVERUPTOOVERDOWN   0x004
#define DDBUTTON_OVERUPTOIDLE       0x002
#define DDBUTTON_IDLETOOVERUP       0x001

/* place flags */
#define DDBUTTON_HITFLAG  (1<<3)
#define DDBUTTON_DOWNFLAG (1<<2)
#define DDBUTTON_OVERFLAG (1<<1)
#define DDBUTTON_UPFLAG   (1<<0)

#define DDBUTTON_HITFRAME  3
#define DDBUTTON_DOWNFRAME 2
#define DDBUTTON_OVERFRAME 1
#define DDBUTTON_UPFRAME   0

struct _ddButtonAction
{
	ddAction* action;
	int flags;
};
typedef struct _ddButtonAction ddButtonAction;

#define DDBUTTON_TRACKASMENU (1<<0)

#define DDBUTTONSOUND_OVERUPTOIDLE 0
#define DDBUTTONSOUND_IDLETOOVERUP 1
#define DDBUTTONSOUND_OVERUPTOOVERDOWN 2
#define DDBUTTONSOUND_OVERDOWNTOOVERUP 3

struct _ddButton
{
	ddCharacter parent;

	ddTimeline* timeline;
	int flags; /* track as menu */

	int nActions;
	ddButtonAction* actions;

	ddSoundInstance* sounds[4];
};


ddButton*
dd_newButton();

void
ddButton_addCharacter(ddButton* button, ddCharacter* character, int layer,
						   ddMatrix matrix, ddCXform cXform, int flags);

void
ddButton_addAction(ddButton* button, ddAction* action, int flags);

void
ddButton_addSoundInstance(ddButton* button, ddSoundInstance* sound, int flags);

#endif /* DD_PLAYER_BUTTON_H_INCLUDED */
