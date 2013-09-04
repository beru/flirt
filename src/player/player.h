/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_PLAYER_H_INCLUDED
#define DD_PLAYER_PLAYER_H_INCLUDED

typedef struct _ddPlayer ddPlayer;

#include "../render/matrix.h"
#include "../render/updatelist.h"
#include "../render/rect.h"
#include "../render/image.h"
#include "../format/read.h"
#include "../format/readthread.h"
#include "../action/value.h"
#include "../action/event.h"
#include "../action/object.h"
#include "../action/context.h"
#include "../action/classes/key_class.h"
#include "../sound/mixer.h"

#include "drawbutton.h"
#include "drawclip.h"
#include "movieclip.h"

typedef enum
{
	DD_ARROW_CURSOR = 0,
	DD_BUTTON_CURSOR,
	DD_TEXTFIELD_CURSOR
} ddCursorType;

struct _ddListEntry
{
	struct _ddListEntry* next;
	void* item;
};

struct _ddList
{
	struct _ddListEntry* head;
	struct _ddListEntry* end;
};

typedef struct _ddList ddList;
typedef struct _ddListEntry ddListEntry;

struct _ddPlayer
{
	ddMovieClip* player;
	ddDrawClip* playclip;
	ddSoundMixer* mixer;
	
	ddRect bounds;
	ddMatrix matrix;
	
	ddUpdateList* updateList;

	ddImage* image;
	ddReader* reader;

	int width;
	int height;
	
	float frameRate;

	ddColor backgroundColor;

	// dragging
	ddDrawClip* dragMovie;
	ddRect dragConstraint;

	UInt16 keycode;

	// current mouse position
	fixed xmouse;
	fixed ymouse;

	// buttons in the movie
	ddDrawButton* activeButton;

	int buttonDown; // is the active button down?
	int overButton; // if it's down, is it still over the button?

	// action globals
	ddActionValue registers[256];

	void (*traceFunction)(const char* message, void* userdata);
	void* traceUserData;

	long startTime;

	ddActionContext* actionContext;
	ddActionObject* globals;
	ddActionKeyObject* keyObject;

	int nLibrarySymbols;
	struct _ddMovieClipLibrarySymbol *librarySymbols;

	// list of currently playing clips
	ddList activeClips;

	// list of actions to run for the current frame
	ddList frameActions;

	// list of clips to be destroyed after all have stepped
	ddList unloadClips;
	
	// event handlers
	ddEventHandlerSet eventHandlers;
};


void
dd_destroyPlayer(ddPlayer* player);

ddPlayer*
dd_newPlayer_file(FILE *f, ddImage* image);

int
ddPlayer_readMovie(ddPlayer* player);

void
ddPlayer_setImage(ddPlayer* player, ddImage* image);

void
ddPlayer_step(ddPlayer* player);

const ddUpdateList*
ddPlayer_getUpdateList(ddPlayer* player);

boolean
ddPlayer_renderFrameSoundIntoBuffer(ddPlayer* player, ddAudioSample* buffer, int* ioSize);

float
ddPlayer_getFrameRate(ddPlayer* player);

ddColor
ddPlayer_getBackgroundColor(ddPlayer* player);

void
ddPlayer_startDrag(ddPlayer* player, ddDrawClip* clip, int lock, ddRect constrain);

void
ddPlayer_stopDrag(ddPlayer* player);

void
ddPlayer_addActiveClip(ddPlayer* player, ddDrawClip* clip);

void
ddPlayer_removeActiveClip(ddPlayer* player, ddDrawClip* clip);

void
ddPlayer_removeActiveButton(ddPlayer* player, ddDrawButton* button);

void
ddPlayer_addUnloadClip(ddPlayer* player, ddDrawClip* clip);

void
ddPlayer_registerClass(ddPlayer* player, char* name, ddActionFunction* ctor);

ddCursorType
ddPlayer_doMouseMove(ddPlayer* player, int x, int y);

void
ddPlayer_doMouseDown(ddPlayer* player, int x, int y);

void
ddPlayer_doMouseUp(ddPlayer* player, int x, int y);

void
ddPlayer_doKeyDown(ddPlayer* player, UInt16 code);

void
ddPlayer_doKeyUp(ddPlayer* player, UInt16 code);

void
ddPlayer_registerMouseMoveListener(ddPlayer* player, ddDrawClip* clip, ddAction* action);

void
ddPlayer_registerMouseDownListener(ddPlayer* player, ddDrawClip* clip, ddAction* action);

void
ddPlayer_registerMouseUpListener(ddPlayer* player, ddDrawClip* clip, ddAction* action);

void
ddPlayer_registerKeyDownListener(ddPlayer* player, ddDrawClip* clip, ddAction* action);

void
ddPlayer_registerKeyUpListener(ddPlayer* player, ddDrawClip* clip, ddAction* action);

void
ddPlayer_registerEnterFrameListener(ddPlayer* player, ddDrawClip* clip, ddAction* action);

void
ddPlayer_removeClipEventHandlers(ddPlayer* player, ddDrawClip* clip);

ddActionMovieClip*
ddPlayer_getRootClip(ddPlayer* player);

void
ddPlayer_addFrameAction(ddPlayer* player, ddAction* action, ddActionObject* object);

#ifdef DD_INCLUDE_DEBUGGER
void
ddPlayer_stepFlashUpdates(ddPlayer* player);

void
ddPlayer_drawUpdates(ddPlayer* player);

typedef enum
{
	STEP_FRAME,
	STEP_OVER,
	STEP_INTO,
	STEP_FINISH
} ddStepMode;

boolean
ddPlayer_executeFrameActions(ddPlayer* player, ddStepMode mode);

void
ddPlayer_updateDisplay(ddPlayer* player, ddRect** outRects, int* outCount);

int
ddPlayer_actionStackDepth(ddPlayer* player);

ddActionContext*
ddPlayer_actionContextAtDepth(ddPlayer* player, int depth);
#endif

// here for dependency problems

void
ddDrawable_unlink(ddPlayer* player, ddDrawable* d, ddUpdateList* update);

#endif /* DD_PLAYER_PLAYER_H_INCLUDED */
