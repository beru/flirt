/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:
 
 * Redistributions of source code must retain the above copyright notice
   and URL, this list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright
   notice and URL, this list of conditions and the following disclaimer
   in the documentation and/or other materials provided with the distribution.   
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "flirt.h"

// functions available when libflirt is compiled with DD_INCLUDE_DEBUGGER set

typedef struct _ddTimelineFrame ddTimelineFrame;
typedef struct _ddTimelineItem ddTimelineItem;
typedef struct _ddAction ddAction;
typedef struct _ddSoundStreamBlock ddSoundStreamBlock;
typedef struct _ddCharacter ddCharacter;
typedef struct _ddDisplayList ddDisplayList;
typedef struct _ddDrawable ddDrawable;
typedef struct _ddDrawButton ddDrawButton;

typedef enum
{
	SHAPE_CHAR,
	MORPH_CHAR,
	BITMAP_CHAR,
	BUTTON_CHAR,
	TEXT_CHAR,
	FONT_CHAR,
	MOVIECLIP_CHAR,
	TEXTFIELD_CHAR,
	SOUND_CHAR
} ddCharacterType;

typedef enum
{
	STEP_FRAME,
	STEP_OVER,
	STEP_INTO,
	STEP_FINISH
} ddStepMode;


// for debugging:

void
ddPlayer_stepFlashUpdates(ddPlayer* player);

void
ddPlayer_drawUpdates(ddPlayer* player);




ddMovieClip*
ddPlayer_getMovieClip(ddPlayer* player);

ddDrawClip*
ddPlayer_getPlayClip(ddPlayer* player);

boolean
ddPlayer_executeFrameActions(ddPlayer* player, ddStepMode mode);

int
ddPlayer_actionStackDepth(ddPlayer* player);

ddActionContext*
ddPlayer_actionContextAtDepth(ddPlayer* player, int depth);

ddAction*
ddActionContext_getAction(ddActionContext* context);

int
ddActionContext_getActionOffset(ddActionContext* context);

char*
ddActionContext_getLabel(ddActionContext* context);

int
ddMovieClip_getFramesTotal(ddMovieClip* clip);

ddTimelineFrame*
ddMovieClip_getFrame(ddMovieClip* clip, int i);

ddTimelineItem*
ddTimelineFrame_getItems(ddTimelineFrame* frame);

ddAction*
ddTimelineFrame_getActions(ddTimelineFrame* frame);

int
ddAction_getLength(ddAction* action);

char*
ddAction_opcodeAtOffset(ddAction* action, int length);

ddAction*
ddAction_functionAtOffset(ddAction* action, int length);

void
ddAction_setBreakpoint(ddAction* action, int line);

void
ddAction_clearBreakpoint(ddAction* action, int line);

void
ddAction_getBreakpoints(ddAction* action, int** outLines, int* outNBreaks);

ddActionObject*
ddPlayer_getGlobals(ddPlayer* player);

int
ddActionObject_getProperties(ddActionObject* object, char*** outNames, ddActionValue** outValues, int* outCount);

ddActionFunction*
ddActionValue_getFunctionValue(ddActionValue value);

ddAction*
ddActionFunction_getBytecodeAction(ddActionFunction* function);

ddSoundStreamBlock*
ddTimelineFrame_getSoundStreamBlock(ddTimelineFrame* frame);

ddTimelineItem*
ddTimelineItem_getNext(ddTimelineItem* item);

ddCharacter*
ddTimelineItem_getCharacter(ddTimelineItem* item);

const char*
ddTimelineItem_getName(ddTimelineItem* item);

ddCharacterType
ddCharacter_getType(ddCharacter* character);

void
ddDrawClip_getDisplayList(ddDrawClip* clip, ddDrawable*** outItems, int* outNItems);

ddActionObject*
ddDrawClip_getActionObject(ddDrawClip* clip);

void
ddDrawButton_getDisplayList(ddDrawClip* clip, ddDrawable*** outItems, int* outNItems);

ddCharacterType
ddDrawable_getType(ddDrawable* d);

ddRect
ddDrawable_getBounds(ddDrawable* d);
