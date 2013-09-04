/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "mixer.h"
#include "../sound/mp3decoder.h"

ddSoundMixer*
dd_newSoundMixer()
{
	ddSoundMixer* mixer = dd_malloc(sizeof(ddSoundMixer));

	mixer->sounds = NULL;
	mixer->streams = NULL;
	mixer->playing = DD_FALSE;
	
	return mixer;
}

void
dd_destroySoundMixer(ddSoundMixer* mixer)
{
	ddPlayingSound* sound = mixer->sounds;
	
	ddSoundMixer_clearStreamBlocks(mixer);
	
	while ( sound != NULL )
	{
		ddPlayingSound* next = sound->next;

		if ( sound->decoder != NULL )
			dd_destroyMP3Decoder(sound->decoder);
		
		dd_free(sound);
		sound = next;
	}

	dd_free(mixer);
}


void
ddSoundMixer_playSoundStreamBlock(ddSoundMixer* mixer, ddDrawClip* clip, ddSoundStreamBlock* block)
{
	struct streamingSound* stream = dd_malloc(sizeof(struct streamingSound));
	
	stream->block = block;
	stream->next = mixer->streams;
	stream->clip = clip;
	
	mixer->streams = stream;
}


ddPlayingSound*
ddSoundMixer_playSoundInstance(ddSoundMixer* mixer, ddDrawClip* clip, ddSoundInstance* sound)
{
	ddPlayingSound* item;
	
	if ( sound->noMultiples )
	{
		ddPlayingSound* item = mixer->sounds;

		while ( item != NULL )
		{
			// XXX - is it by sound or sound instance?
			
			if ( item->sound->characterid == sound->characterid )
				return NULL;
		}
	}
	
	item = dd_malloc(sizeof(ddPlayingSound));

	item->sound = sound;
	item->clip = clip;
	item->xform = ddDrawClip_getSoundTransform(clip);
	item->offset = sound->inPoint;
	item->loopCount = 0;
	item->decoder = NULL;
	
	item->next = mixer->sounds;
	mixer->sounds = item;
	
	return item;
}


void
ddSoundMixer_stopSoundInstance(ddSoundMixer* mixer, ddSoundInstance* sound)
{
	ddPlayingSound** listptr = &mixer->sounds;
	ddPlayingSound* item = *listptr;
	
	while ( item != NULL )
	{
		if ( item->sound->characterid == sound->characterid )
		{
			dd_free(item);
			*listptr = item->next;
		}
		else
			listptr = &item->next;
		
		item = item->next;
	}
}


boolean
ddSoundMixer_renderFrameIntoBuffer(ddSoundMixer* mixer, ddAudioSample* buffer, int* ioSize)
{
	int length = *ioSize;
	int writelen = length;
	struct streamingSound* block;
	ddPlayingSound* sound = mixer->sounds;
	ddPlayingSound** soundptr = &mixer->sounds;
	
	while ( sound != NULL )
	{
		ddPlayingSound* next = sound->next;
		
		if ( !ddSoundInstance_renderIntoBuffer(sound->sound, sound->clip, &sound->offset, &sound->loopCount, buffer, length, &sound->decoder, &sound->delay) )
		{
			// remove from list

			if ( sound->decoder != NULL )
				dd_destroyMP3Decoder(sound->decoder);
			
			*soundptr = next;
			dd_free(sound);
		}
		else
			soundptr = &sound->next;
		
		if ( length + sound->delay > writelen )
			writelen = length + sound->delay;
		
		sound = next;
	}
	
	block = mixer->streams;
	
	while ( block != NULL )
	{
		struct streamingSound* next = block->next;
		int delay = 0;
		
		ddSoundStreamBlock_renderIntoBuffer(block->block, block->clip, buffer, length, &delay);
		
		if ( length + delay > writelen )
			writelen = length + delay;
		
		dd_free(block);
		block = next;
	}
	
	mixer->streams = NULL;

	*ioSize = writelen;
	
	return DD_TRUE;
}


void
ddSoundMixer_clearStreamBlocks(ddSoundMixer* mixer)
{
	struct streamingSound* block = mixer->streams;
	
	while ( block != NULL )
	{
		struct streamingSound* next = block->next;
		
		dd_free(block);
		
		block = next;
	}
	
	mixer->streams = NULL;
}
