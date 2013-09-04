/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "readsound.h"

void
readMP3Headers(ddReader* r, int length);


void
readDefineSound(ddMovieClip* p, ddReader* r, int length)
{
	int end = ddReader_getOffset(r) + length;
	int characterID = readUInt16(r);
	int flags = readUInt8(r);
	
	ddSoundFormat format;
	int sampleCount;
	unsigned char* data;
	int sampleRate;
	int bitsPerSample;
	boolean stereo;
	int delay;
	int size;
	ddSound* sound;
	
	switch ( (flags & 0xf0) >> 4 )
	{
		case 0: format = DDSOUND_UNCOMPRESSED; break;
		case 1: format = DDSOUND_ADPCM; break;
		case 2: format = DDSOUND_MP3; break;
		case 3: format = DDSOUND_UNCOMPRESSED_LITTLEENDIAN; break;
		case 6: format = DDSOUND_NELLYMOSER; break;
		default: dd_warn("Unknown sound format: %i", (flags & 0xf0) >> 4);
	}
	
	sampleCount = readUInt32(r);
	
	if ( format == DDSOUND_MP3 )
		delay = readSInt16(r);
	
	size = end - ddReader_getOffset(r);
	
	data = readBlock(r, size);
	
	switch ( (flags & 0x0c) >> 2 )
	{
		case 0: sampleRate = 5500; break;
		case 1: sampleRate = 11000; break;
		case 2: sampleRate = 22000; break;
		case 3: sampleRate = 44000; break;
	}
	
	bitsPerSample = ((flags & 0x02) != 0) ? 16 : 8;
	stereo = ((flags & 0x01) != 0) ? DD_TRUE : DD_FALSE;
	
	sound = dd_newSound(format, sampleRate, bitsPerSample, stereo, sampleCount, size, data, delay);

	ddMovieClip_addCharacter(p, characterID, (ddCharacter*)sound);
}


/* streamhead type 2 allows uncompressed data, and 8-bit if uncompressed */

void
readSoundStreamHead(ddMovieClip* p, ddReader* r, int length)
{
	int flags;
	ddSoundFormat format;
	int sampleRate;
	int bitsPerSample;
	boolean stereo;
	int samplesPerFrame;
	int delay = 0;
	
	readUInt8(r); // recommended playback settings - ignore
	
	flags = readUInt8(r);
	
	switch ( (flags & 0xf0) >> 4 )
	{
		case 0: format = DDSOUND_UNCOMPRESSED; break;
		case 1: format = DDSOUND_ADPCM; break;
		case 2: format = DDSOUND_MP3; break;
		case 3: format = DDSOUND_UNCOMPRESSED_LITTLEENDIAN; break;
		case 6: format = DDSOUND_NELLYMOSER; break;
		default: dd_warn("Unknown sound format: %i", (flags & 0xf0) >> 4);
	}
	
	switch ( (flags & 0x0c) >> 2 )
	{
		case 0: sampleRate = 5500; break;
		case 1: sampleRate = 11000; break;
		case 2: sampleRate = 22000; break;
		case 3: sampleRate = 44000; break;
	}
	
	bitsPerSample = ((flags & 0x02) != 0) ? 16 : 8;
	stereo = ((flags & 0x01) != 0) ? DD_TRUE : DD_FALSE;
	
	samplesPerFrame = readUInt16(r);
	
	if ( format == DDSOUND_MP3 )
		delay = readSInt16(r);
	
	ddMovieClip_setSoundStreamFormat(p, format, sampleRate, bitsPerSample, stereo, samplesPerFrame, delay);
}


void
readSoundStreamBlock(ddMovieClip* p, ddReader* r, int length)
{
	int sampleCount;
	int delay = 0;
	int size = length;
	unsigned char* data;

	if ( ddMovieClip_getSoundStreamFormat(p) == DDSOUND_MP3 )
	{
		sampleCount = readUInt16(r);
		delay = readUInt16(r);
		size -= 4;
	}
	else
		sampleCount = size;
	
	data = readBlock(r, size);
	
	ddMovieClip_addSoundStreamBlock(p, sampleCount, delay, size, data);
}


#define SOUNDINFO_STOP			(1<<5)
#define SOUNDINFO_NOMULTIPLE	(1<<4)
#define SOUNDINFO_HASENVELOPE	(1<<3)
#define SOUNDINFO_HASLOOPS		(1<<2)
#define SOUNDINFO_HASOUTPOINT	(1<<1)
#define SOUNDINFO_HASINPOINT	(1<<0)

ddSoundInstance*
readSoundInfo(ddReader* r, int characterid)
{
	int flags = readUInt8(r);
	ddSoundInstance* sound;
	
	if ( (flags & SOUNDINFO_STOP) != 0 )
		return NULL;
	
	sound = dd_newSoundInstance(characterid);
	
	ddSoundInstance_setNoMultipleFlag(sound, (flags & SOUNDINFO_NOMULTIPLE) ? DD_TRUE : DD_FALSE);

	if ( (flags & SOUNDINFO_HASINPOINT) != 0 )
		ddSoundInstance_setInPoint(sound, readUInt32(r));
	
	if ( (flags & SOUNDINFO_HASOUTPOINT) != 0 )
		ddSoundInstance_setOutPoint(sound, readUInt32(r));
	
	if ( (flags & SOUNDINFO_HASLOOPS) != 0 )
		ddSoundInstance_setLoopCount(sound, readUInt16(r));
	
	if ( (flags & SOUNDINFO_HASENVELOPE) != 0 )
	{
		int count = readUInt8(r);
		int i;
		
		for ( i = 0; i < count; ++i )
		{
			int position = readUInt32(r);
			float leftLevel = (float)readUInt16(r) / 0xffff;
			float rightLevel = (float)readUInt16(r) / 0xffff;
			
			ddSoundInstance_addEnvelopePoint(sound, position, leftLevel, rightLevel);
		}
	}
	
	return sound;
}

void
readStartSound(ddMovieClip* p, ddReader* r, int length)
{
	int soundID = readUInt16(r);

	ddMovieClip_addSoundInstance(p, readSoundInfo(r, soundID));
}


/*
#define MP3_FRAME_SYNC       0xFFE00000

#define MP3_VERSION          0x00180000
#define MP3_VERSION_25       0x00000000
#define MP3_VERSION_RESERVED 0x00080000
#define MP3_VERSION_2        0x00100000
#define MP3_VERSION_1        0x00180000

#define MP3_LAYER            0x00060000
#define MP3_LAYER_RESERVED   0x00000000
#define MP3_LAYER_3          0x00020000
#define MP3_LAYER_2          0x00040000
#define MP3_LAYER_1          0x00060000

#define MP3_PROTECT          0x00010000 // 16-bit CRC after header

#define MP3_BITRATE          0x0000F000
#define MP3_BITRATE_SHIFT    12

int mp1l1_bitrate_table[] = { 0,   32,   64,  96, 128, 160, 192, 224,
	256, 288, 320, 352, 382, 416, 448 };

int mp1l2_bitrate_table[] = { 0,   32,   48,  56,  64,  80,  96, 112,
	128, 160, 192, 224, 256, 320, 384 };

int mp1l3_bitrate_table[] = { 0,    32,  40,  48,  56,  64,  80,  96,
	112, 128, 160, 192, 224, 256, 320 };

int mp2l1_bitrate_table[] = { 0,    32,  48,  56,  64,  80,  96, 112,
	128, 144, 160, 176, 192, 224, 256 };

int mp2l23_bitrate_table[] = { 0,    8,  16,  24,  32,  40,  48,  56,
	64,  80,  96, 112, 128, 144, 160 };

#define MP3_SAMPLERATE       0x00000C00
#define MP3_SAMPLERATE_SHIFT 10

int mp1_samplerate_table[] = { 44100, 48000, 32000 };
int mp2_samplerate_table[] = { 22050, 24000, 16000 };
int mp25_samplerate_table[] = { 11025, 12000, 8000 };

#define MP3_PADDING          0x00000200 // if set, add an extra slot - 4 bytes
for layer 1, 1 byte for 2+3

#define MP3_CHANNEL          0x000000C0
#define MP3_CHANNEL_STEREO   0x00000000
#define MP3_CHANNEL_JOINT    0x00000040
#define MP3_CHANNEL_DUAL     0x00000080
#define MP3_CHANNEL_MONO     0x000000C0

// rest of the header info doesn't affect frame size..

void
silentSkipBytes(ddReader* r, int length)
{
	for ( ; length > 0; --length )
		readUInt8(r);
}


void
readMP3Headers(ddReader* r, int length)
{
	unsigned long flags;
	int frameLen;
	int frameNum = 0;
	int bitrate = 0;
	int bitrate_idx;
	int samplerate;
	int samplerate_idx;
	int version = 0;
	int layer = 0;
	int channels = 0;
	int padding = 0;

	while ( length > 0 )
	{
		++frameNum;

		// get 4-byte header, bigendian
		flags = readUInt8(r) << 24;
		flags += readUInt8(r) << 16;
		flags += readUInt8(r) << 8;
		flags += readUInt8(r);

		if ( (flags & MP3_FRAME_SYNC) != MP3_FRAME_SYNC )
			dd_warn("bad sync on MP3 block!");

		bitrate_idx = (flags & MP3_BITRATE) >> MP3_BITRATE_SHIFT;
		samplerate_idx = (flags & MP3_SAMPLERATE) >> MP3_SAMPLERATE_SHIFT;

		channels = ((flags & MP3_CHANNEL) == MP3_CHANNEL_MONO) ? 1 : 2;

		switch ( flags & MP3_VERSION )
		{
			case MP3_VERSION_1:  version = 1; break;
			case MP3_VERSION_2:  version = 2; break;
			case MP3_VERSION_25: version = 25; break;
			default: dd_warn("unknown MP3 version!");
		}
		
		switch ( flags & MP3_LAYER )
		{
			case MP3_LAYER_1: layer = 1; break;
			case MP3_LAYER_2: layer = 2; break;
			case MP3_LAYER_3: layer = 3; break;
			default: dd_warn("unknown MP3 layer!");
		}

		if ( version == 1 )
		{
			samplerate = mp1_samplerate_table[samplerate_idx];

			if ( layer == 1 )
				bitrate = mp1l1_bitrate_table[bitrate_idx];

			else if ( layer == 2 )
				bitrate = mp1l2_bitrate_table[bitrate_idx];

			else if ( layer == 3 )
				bitrate = mp1l3_bitrate_table[bitrate_idx];
		}
		else
		{
			if ( version == 2 )
				samplerate = mp2_samplerate_table[samplerate_idx];
			else
				samplerate = mp25_samplerate_table[samplerate_idx];

			if ( layer == 1 )
				bitrate = mp2l1_bitrate_table[bitrate_idx];
			else
				bitrate = mp2l23_bitrate_table[bitrate_idx];
		}

		padding = (flags & MP3_PADDING) ? 1 : 0;

		if ( layer == 1 )
			padding <<= 2;

		if ( version == 1 )
			frameLen = 144 * bitrate * 1000 / samplerate + padding;
		else
			frameLen = 72 * channels * bitrate * 1000 / samplerate + padding;

		printf("frame %i: MP%i layer %i, %i Hz, %ikbps, %s, length=%i, protect %s",
		 frameNum, version, layer, samplerate, bitrate,
		 (channels==2) ? "stereo" : "mono", frameLen,
		 (flags&MP3_PROTECT) ? "on" : "off");
		
		if ( length < frameLen - 4 )
			silentSkipBytes(r, length);
		else
			silentSkipBytes(r, frameLen - 4);

		length -= frameLen;
	}

	if ( length > 0 )
		skipBytes(r, length);
}
*/
