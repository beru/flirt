/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_SOUND_H_INCLUDED
#define DD_PLAYER_SOUND_H_INCLUDED

typedef struct _ddSound ddSound;
typedef struct _ddSoundStreamBlock ddSoundStreamBlock;
typedef struct _ddSoundInstance ddSoundInstance;
typedef struct _ddSoundTransform ddSoundTransform;

struct _ddSoundTransform
{
	float leftToLeft;
	float leftToRight;
	float rightToLeft;
	float rightToRight;
};

#include "../dd.h"

typedef enum
{
	DDSOUND_UNCOMPRESSED,
	DDSOUND_ADPCM,
	DDSOUND_MP3,
	DDSOUND_UNCOMPRESSED_LITTLEENDIAN,
	DDSOUND_NELLYMOSER
} ddSoundFormat;

#include "../sound/mp3decoder.h"
#include "character.h"
#include "drawclip.h"

static inline ddSoundTransform
dd_newSoundTransform(float ltol, float ltor, float rtol, float rtor)
{
	ddSoundTransform xform = { ltol, ltor, rtol, rtor };
	return xform;
}


struct _ddSound
{
	ddCharacter parent;
	
	ddSound* next;
	
	int size;
	unsigned char* data;
	
	ddSoundFormat format;
	int sampleRate;
	int bitsPerSample;
	boolean stereo;
	int sampleCount;
	int delay;
};

ddSound*
dd_newSound(ddSoundFormat format, int sampleRate, int bitsPerSample, boolean stereo, int sampleCount, int size, unsigned char* data, int delay);

void
dd_destroySound(ddCharacter* sound);


struct _ddSoundStreamBlock
{
	int size;
	unsigned char* data;
	
	ddSoundFormat format;
	int sampleRate;
	int bitsPerSample;
	boolean stereo;
	int sampleCount;
	int delay;
};

ddSoundStreamBlock*
dd_newSoundStreamBlock(ddSoundFormat format, int sampleRate, int bitsPerSample, boolean stereo, int sampleCount, int size, unsigned char* data, int delay);

void
dd_destroySoundStreamBlock(ddSoundStreamBlock* block);

void
ddSoundStreamBlock_renderIntoBuffer(ddSoundStreamBlock* block, ddDrawClip* clip, ddAudioSample* buffer, int length, int* outDelay);

struct ddSoundEnvelopePoint
{
	int position;
	float leftLevel;
	float rightLevel;
};

struct _ddSoundInstance
{
	ddSoundInstance* next;
	
	int characterid;
	ddSound* sound;
	
	boolean stop;
	boolean noMultiples;
	
	int inPoint;
	int outPoint;
	int loopCount;
	
	int envelopeCount;
	struct ddSoundEnvelopePoint* points;
};

ddSoundInstance*
dd_newSoundInstance(int character);

void
ddSoundInstance_setStopFlag(ddSoundInstance* sound, boolean flag);

void
ddSoundInstance_setNoMultipleFlag(ddSoundInstance* sound, boolean flag);

void
ddSoundInstance_setInPoint(ddSoundInstance* sound, int position);

void
ddSoundInstance_setOutPoint(ddSoundInstance* sound, int position);

void
ddSoundInstance_setLoopCount(ddSoundInstance* sound, int position);

void
ddSoundInstance_addEnvelopePoint(ddSoundInstance* sound, int position, float left, float right);

boolean
ddSoundInstance_renderIntoBuffer(ddSoundInstance* instance, ddDrawClip* clip, int* offset, int* loopCount, ddAudioSample* buffer, int length, ddMP3Decoder** decoder, int* delay);

#endif /* DD_PLAYER_SOUND_H_INCLUDED */
