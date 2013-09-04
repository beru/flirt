/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>
#include <ctype.h>

#include "movieclip.h"
#include "drawclip.h"

void
dd_destroyMovieClip(ddCharacter* c)
{
	ddMovieClip* clip = (ddMovieClip*)c;
	int i;

	if ( clip->flags & MOVIECLIP_FREECHARLIST )
		dd_destroyCharacterList(clip->charList);

	if ( clip->timeline != NULL )
		dd_destroyTimeline(clip->timeline);

	for ( i = 0; i < clip->nLabels; ++i )
		dd_free(clip->labels[i].label);

	if ( clip->labels != NULL )
		dd_free(clip->labels);

	dd_free(clip);
}


ddDrawable*
ddMovieClip_instantiate(ddPlayer* player, ddCharacter* c)
{
	ddMovieClip* clip = (ddMovieClip*)c;
	
	if ( clip->initAction != NULL )
	{
		ddAction_execute(player, clip->initAction, (ddActionObject*)player->playclip->actionClip);
		clip->initAction = NULL;
	}

	return (ddDrawable*)dd_newDrawClip(player, clip);
}


ddMovieClip*
dd_newMovieClip(ddCharacterList* charList)
{
	ddMovieClip* clip = dd_malloc(sizeof(ddMovieClip));

	clip->parent.type = MOVIECLIP_CHAR;
	clip->parent.instantiate = ddMovieClip_instantiate;
	clip->parent.destroy = dd_destroyMovieClip;
	clip->framesLoaded = 0;
	clip->framesTotal = 0;
	clip->bytesLoaded = 0;
	clip->bytesTotal = 0;
	clip->width = 0;
	clip->height = 0;

	clip->flags = 0;
	clip->nLabels = 0;
	clip->labels = NULL;

	clip->timeline = dd_newTimeline();

	clip->librarySymbols = NULL;
	clip->nLibrarySymbols = 0;

	if ( charList == NULL )
	{
		clip->charList = dd_newCharacterList();
		clip->flags |= MOVIECLIP_FREECHARLIST;
	}
	else
		clip->charList = charList;

	clip->initAction = NULL;

	return clip;
}


ddDrawClip*
ddMovieClip_makeRootClip(ddPlayer* player, ddMovieClip* clip, int level)
{
	char* name = dd_malloc(16);

	ddDrawClip* d = dd_newDrawClip(player, clip);

	snprintf(name, 15, "_level%i", level);

	d->actionClip->rootClip = d->actionClip;
	d->parentClip = NULL;

	ddActionMovieClip_setName(d->actionClip, name);

	ddActionHash_setValue(((ddActionObject*)d->actionClip)->properties, strdup("$version"), dd_newActionValue_string("MAC 6,0,21,0"));
	
	return d;
}


void
ddMovieClip_addFrameLabel(ddMovieClip* clip, char* label)
{
	clip->labels = dd_realloc(clip->labels,
							(clip->nLabels + 1) * sizeof(struct _ddMovieClipFrameLabel));

	clip->labels[clip->nLabels].frame = clip->framesLoaded;
	clip->labels[clip->nLabels].label = label;

	++clip->nLabels;
}


int
ddMovieClip_findFrameLabel(ddMovieClip* clip, const char* label)
{
	int i;
	boolean isNum = DD_TRUE;
	
	// XXX - kinda goofy:

	for ( i = 0; label[i] != '\0'; ++i )
	{
		if ( !isdigit(label[0]) )
		{
			isNum = DD_FALSE;
			break;
		}
	}

	if ( isNum )
		return atoi(label);

	for ( i = 0; i < clip->nLabels; ++i )
	{
		if ( strcmp(label, clip->labels[i].label) == 0 )
			return clip->labels[i].frame;
	}

	return -1;
}


void
ddMovieClip_addLibrarySymbol(ddMovieClip* player, int id, char* name)
{
	player->librarySymbols = dd_realloc(player->librarySymbols,
									 (player->nLibrarySymbols + 1) *
									 sizeof(struct _ddMovieClipLibrarySymbol));

	player->librarySymbols[player->nLibrarySymbols].charID = id;
	player->librarySymbols[player->nLibrarySymbols].name = name;
	player->librarySymbols[player->nLibrarySymbols].ctor = NULL;
	
	++player->nLibrarySymbols;
}


void
ddMovieClip_registerClass(ddMovieClip* player, char* name, ddActionFunction* ctor)
{
	int i;

	for ( i = 0; i < player->nLibrarySymbols; ++i )
	{
		if ( strcasecmp(name, player->librarySymbols[i].name) == 0 )
		{
			player->librarySymbols[i].ctor = ctor;
			return;
		}
	}
}


boolean
ddMovieClip_findLibrarySymbol(ddMovieClip* player, const char* name, ddCharacter** outChar, ddActionFunction** outCtor)
{
	int i;

	for ( i = 0; i < player->nLibrarySymbols; ++i )
	{
		if ( strcasecmp(name, player->librarySymbols[i].name) == 0 )
		{
			*outChar = ddCharacterList_getCharacter(player->charList, player->librarySymbols[i].charID);
			*outCtor = player->librarySymbols[i].ctor;

			return DD_TRUE;
		}
	}

	return DD_FALSE;
}


void
ddMovieClip_nextFrame(ddMovieClip* clip)
{
	++clip->framesLoaded;
	/* clip->bytesLoaded = ddReader_getOffset(r); */

	if ( clip->framesLoaded < clip->framesTotal )
		ddTimeline_nextFrame(clip->timeline);
	//else
	//	++clip->timeline->nFrames;
}


int
ddMovieClip_getBytesLoaded(ddMovieClip* clip)
{
	return clip->bytesLoaded;
}


int
ddMovieClip_getBytesTotal(ddMovieClip* clip)
{
	return clip->bytesTotal;
}


int
ddMovieClip_getFramesLoaded(ddMovieClip* clip)
{
	return clip->framesLoaded;
}


void
ddMovieClip_addCharacter(ddMovieClip* clip, int id, ddCharacter* c)
{
	ddCharacterList_addCharacter(clip->charList, id, c);
}


ddCharacter*
ddMovieClip_getCharacter(ddMovieClip* clip, int id)
{
	return ddCharacterList_getCharacter(clip->charList, id);
}


ddCharacterList*
ddMovieClip_getCharList(ddMovieClip* clip)
{
	return clip->charList;
}


void
ddMovieClip_addInitAction(ddMovieClip* clip, ddAction* action)
{
	clip->initAction = action;
}


void
ddMovieClip_setWidth(ddMovieClip* clip, fixed width)
{
	clip->width = width;
}


void
ddMovieClip_setHeight(ddMovieClip* clip, fixed height)
{
	clip->height = height;
}


fixed
ddMovieClip_getWidth(ddMovieClip* clip)
{
	return clip->width;
}


fixed
ddMovieClip_getHeight(ddMovieClip* clip)
{
	return clip->height;
}


void
ddMovieClip_addSoundInstance(ddMovieClip* clip, ddSoundInstance* sound)
{
	ddTimeline_addSoundInstance(clip->timeline, sound);
}


void
ddMovieClip_setSoundStreamFormat(ddMovieClip* clip, ddSoundFormat format, int sampleRate, int bitsPerSample, boolean stereo, int samplesPerFrame, int initialDelay)
{
	clip->stream.format = format;
	clip->stream.sampleRate = sampleRate;
	clip->stream.bitsPerSample = bitsPerSample;
	clip->stream.stereo = stereo;
	clip->stream.samplesPerFrame = samplesPerFrame;
	clip->stream.delay = initialDelay;
}


ddSoundFormat
ddMovieClip_getSoundStreamFormat(ddMovieClip* clip)
{
	return clip->stream.format;
}


void
ddMovieClip_addSoundStreamBlock(ddMovieClip* clip, int sampleCount, int delay, int size, unsigned char* data)
{
	ddSoundStreamBlock* block =
		dd_newSoundStreamBlock(clip->stream.format,
							   clip->stream.sampleRate,
							   clip->stream.bitsPerSample,
							   clip->stream.stereo,
							   sampleCount, size, data, delay);

	ddTimeline_addSoundStreamBlock(clip->timeline, block);
}


#ifdef DD_INCLUDE_DEBUGGER

int
ddMovieClip_getFramesTotal(ddMovieClip* clip)
{
	return clip->timeline->nFrames; // clip->framesTotal;
}

ddTimelineFrame*
ddMovieClip_getFrame(ddMovieClip* clip, int i)
{
	return &clip->timeline->frames[i];
}

#endif
