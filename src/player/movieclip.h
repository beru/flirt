/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_MOVIECLIP_H_INCLUDED
#define DD_PLAYER_MOVIECLIP_H_INCLUDED

typedef struct _ddMovieClip ddMovieClip;

#include "character.h"
#include "timeline.h"
#include "charlist.h"
#include "drawclip.h"
#include "sound.h"

#define MOVIECLIP_FREECHARLIST	(1<<1)

struct _ddMovieClipFrameLabel
{
	int frame;
	char* label;
};

struct _ddMovieClipLibrarySymbol
{
	int charID;
	char* name;
	ddActionFunction* ctor;
};

struct _ddMovieClip
{
	ddCharacter parent;

	int framesLoaded;
	int framesTotal;
	int bytesLoaded;
	int bytesTotal;
	fixed width;
	fixed height;

	int flags;

	int nLabels;
	struct _ddMovieClipFrameLabel *labels;

	int nLibrarySymbols;
	struct _ddMovieClipLibrarySymbol *librarySymbols;

	ddTimeline* timeline;
	ddCharacterList* charList;

	ddAction* initAction;
	
	struct
	{
		ddSoundFormat format;
		int sampleRate;
		int bitsPerSample;
		boolean stereo;
		int samplesPerFrame;
		int delay;
	} stream;
};


ddMovieClip*
dd_newMovieClip(ddCharacterList* charList);

void
dd_destroyMovieClip(ddCharacter* c);

ddDrawClip*
ddMovieClip_makeRootClip(ddPlayer* player, ddMovieClip* clip, int level);

void
ddMovieClip_addFrameLabel(ddMovieClip* clip, char* label);

int
ddMovieClip_findFrameLabel(ddMovieClip* clip, const char* label);

void
ddMovieClip_addLibrarySymbol(ddMovieClip* clip, int id, char* name);

void
ddMovieClip_registerClass(ddMovieClip* clip, char* name, ddActionFunction* ctor);

boolean
ddMovieClip_findLibrarySymbol(ddMovieClip* player, const char* name, ddCharacter** outChar, ddActionFunction** outCtor);

/* prepares movieclip for next frame's placeobject/removeobject blocks-
not to be confused with ddDrawClip_nextFrame which moves the clip
instance's playhead */

void
ddMovieClip_nextFrame(ddMovieClip* clip);

int
ddMovieClip_getBytesLoaded(ddMovieClip* clip);

int
ddMovieClip_getBytesTotal(ddMovieClip* clip);

int
ddMovieClip_getFramesLoaded(ddMovieClip* clip);

void
ddMovieClip_addCharacter(ddMovieClip* clip, int id, ddCharacter* c);

ddCharacter*
ddMovieClip_getCharacter(ddMovieClip* clip, int id);

ddCharacterList*
ddMovieClip_getCharList(ddMovieClip* clip);

void
ddMovieClip_addInitAction(ddMovieClip* clip, ddAction* action);

void
ddMovieClip_setWidth(ddMovieClip* clip, fixed width);

void
ddMovieClip_setHeight(ddMovieClip* clip, fixed height);

fixed
ddMovieClip_getWidth(ddMovieClip* clip);

fixed
ddMovieClip_getHeight(ddMovieClip* clip);

void
ddMovieClip_addSoundInstance(ddMovieClip* clip, ddSoundInstance* sound);

void
ddMovieClip_setSoundStreamFormat(ddMovieClip* clip, ddSoundFormat format, int sampleRate, int bitsPerSample, boolean stereo, int samplesPerFrame, int initialDelay);

ddSoundFormat
ddMovieClip_getSoundStreamFormat(ddMovieClip* clip);

void
ddMovieClip_addSoundStreamBlock(ddMovieClip* clip, int sampleCount, int delay, int size, unsigned char* data);

#endif /* DD_PLAYER_MOVIECLIP_H_INCLUDED */
