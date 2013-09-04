/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "sound.h"

void
dd_destroySound(ddCharacter* c)
{
	ddSound* sound = (ddSound*)c;
	
	if ( sound->data != NULL )
		dd_free(sound->data);
	
	dd_free(sound);
}

ddSound*
dd_newSound(ddSoundFormat format, int sampleRate, int bitsPerSample, boolean stereo, int sampleCount, int size, unsigned char* data, int delay)
{
	ddSound* sound = dd_malloc(sizeof(ddSound));
	
	sound->parent.type = SOUND_CHAR;
	sound->parent.instantiate = NULL;
	sound->parent.destroy = dd_destroySound;
	
	sound->format = format;
	sound->sampleRate = sampleRate;
	sound->bitsPerSample = bitsPerSample;
	sound->stereo = stereo;
	sound->sampleCount = sampleCount;
	sound->size = size;
	sound->data = data;
	sound->delay = delay;
	
	return sound;
}


ddSoundStreamBlock*
dd_newSoundStreamBlock(ddSoundFormat format, int sampleRate, int bitsPerSample, boolean stereo, int sampleCount, int size, unsigned char* data, int delay)
{
	ddSoundStreamBlock* block = dd_malloc(sizeof(ddSoundStreamBlock));
	
	block->size = size;
	block->data = data;
	block->format = format;
	block->sampleRate = sampleRate;
	block->bitsPerSample = bitsPerSample;
	block->stereo = stereo;
	block->sampleCount = sampleCount;
	block->delay = delay;
	
	return block;
}

void
dd_destroySoundStreamBlock(ddSoundStreamBlock* block)
{
	if ( block->data != NULL )
		dd_free(block->data);
	
	dd_free(block);
}


void
ddSoundStreamBlock_renderMP3IntoBuffer(ddSoundStreamBlock* block, ddDrawClip* clip, ddSoundTransform xform, ddAudioSample* buffer, int length, int* outDelay)
{
	ddMP3Decoder* decoder = ddDrawClip_getMP3Decoder(clip);
	unsigned char* data = block->data;
	unsigned char* end = block->data + block->size;
	ddAudioSample* bufend = buffer + length;
	
	while ( data < end && buffer < bufend )
	{
		if ( !ddMP3Decoder_decodeFrame(decoder, xform, &data, end - data, block->delay, &buffer, length) )
			break;
	}
	
	*outDelay = buffer - bufend;
}


boolean
renderUncompressedDataIntoBuffer(unsigned char* indata, int* ioSize, int sampleRate, boolean stereo, int bitsPerSample, ddSoundTransform xform, ddAudioSample* buffer, int length)
{
	int size = *ioSize;
	unsigned char* data = indata;
	unsigned char* dataend = data + size;
	ddAudioSample* bufend = buffer + length;
	
	int left;
	int right;
	
	left = (unsigned)*data++;
	left += (SInt16)(*data++ << 8);
	
	if ( stereo )
	{
		right = (unsigned)*data++;
		right += (SInt16)(*data++ << 8);
	}
	else
		right = left;
	
	while ( data < dataend && buffer < bufend )
	{
		int nextleft;
		int nextright;
		int j;
		
		if ( bitsPerSample == 16 )
		{
			nextleft = (unsigned)*data++;
			nextleft += (SInt16)(*data++ << 8);
			
			if ( stereo )
			{
				nextright = (unsigned)*data++;
				nextright += (SInt16)(*data++ << 8);
			}
			else
				nextright = nextleft;
		}
		else
		{
			nextleft = (SInt16)(*data++ << 8);
			
			if ( stereo )
				nextright = (SInt16)(*data++ << 8);
			else
				nextright = nextleft;
		}
		
		if ( sampleRate == 5500 )
		{
			for ( j = 0; j < 8; ++j )
			{
				SInt16 l = left + j * (nextleft - left) / 8;
				SInt16 r = right + j * (nextright - right) / 8;
			
				buffer->left += l * xform.leftToLeft + r * xform.rightToLeft;
				buffer->right += l * xform.leftToRight + r * xform.rightToRight;
				++buffer;
			}
		}
		else if ( sampleRate == 11000 )
		{
			for ( j = 0; j < 4; ++j )
			{
				SInt16 l = left + j * (nextleft - left) / 4;
				SInt16 r = right + j * (nextright - right) / 4;
				
				buffer->left += l * xform.leftToLeft + r * xform.rightToLeft;
				buffer->right += l * xform.leftToRight + r * xform.rightToRight;
				++buffer;
			}
		}
		else if ( sampleRate == 22000 )
		{
			buffer->left += left * xform.leftToLeft + right * xform.rightToLeft;
			buffer->right += left * xform.leftToRight + right * xform.rightToRight;

			left = (nextleft + left) / 2;
			right = (nextright + right) / 2;
			
			++buffer;
			
			buffer->left += left * xform.leftToLeft + right * xform.rightToLeft;
			buffer->right += left * xform.leftToRight + right * xform.rightToRight;
		}
		else if ( sampleRate == 44000 )
		{
			buffer->left += left * xform.leftToLeft + right * xform.rightToLeft;
			buffer->right += left * xform.leftToRight + right * xform.rightToRight;
			++buffer;
		}

		left = nextleft;
		right = nextright;
	}
	
	*ioSize = data - indata;
	
	// return DD_TRUE if we played the whole thing
	return (data < dataend);
}


void
ddSoundStreamBlock_renderUncompressedIntoBuffer(ddSoundStreamBlock* block, ddSoundTransform xform, ddAudioSample* buffer, int length)
{
	int count = block->sampleCount;
	
	renderUncompressedDataIntoBuffer(block->data, &count, block->sampleRate, block->stereo, block->bitsPerSample, xform, buffer, length);
}


void
ddSoundStreamBlock_renderIntoBuffer(ddSoundStreamBlock* block, ddDrawClip* clip, ddAudioSample* buffer, int length, int* outDelay)
{
	ddSoundTransform xform = ddDrawClip_getSoundTransform(clip);
	
	switch ( block->format )
	{
		case DDSOUND_UNCOMPRESSED:
		case DDSOUND_UNCOMPRESSED_LITTLEENDIAN:
			ddSoundStreamBlock_renderUncompressedIntoBuffer(block, xform, buffer, length);
			break;
		case DDSOUND_ADPCM:
			dd_warn("ADPCM not supported");
			break;
		case DDSOUND_MP3:
			ddSoundStreamBlock_renderMP3IntoBuffer(block, clip, xform, buffer, length, outDelay);
			break;
		case DDSOUND_NELLYMOSER:
			dd_warn("Nelly-Moser not supported");
			break;
	}
}


ddSoundInstance*
dd_newSoundInstance(int character)
{
	ddSoundInstance* sound = dd_malloc(sizeof(ddSoundInstance));
	
	sound->next = NULL;
	sound->characterid = character;
	
	sound->stop = DD_FALSE;
	sound->noMultiples = DD_FALSE;
	sound->loopCount = 1;
	
	return sound;
}


void
dd_destroySoundInstance(ddSoundInstance* sound)
{
	if ( sound->points != NULL )
		dd_free(sound->points);
	
	dd_free(sound);
}


void
ddSoundInstance_setStopFlag(ddSoundInstance* sound, boolean flag)
{
	sound->stop = flag;
}


void
ddSoundInstance_setNoMultipleFlag(ddSoundInstance* sound, boolean flag)
{
	sound->noMultiples = flag;
}


void
ddSoundInstance_setInPoint(ddSoundInstance* sound, int position)
{
	sound->inPoint = position;
}


void
ddSoundInstance_setOutPoint(ddSoundInstance* sound, int position)
{
	sound->outPoint = position;
}


void
ddSoundInstance_setLoopCount(ddSoundInstance* sound, int count)
{
	sound->loopCount = count;
}


void
ddSoundInstance_addEnvelopePoint(ddSoundInstance* sound, int position, float left, float right)
{
	int n = sound->envelopeCount++;
	
	sound->points = dd_realloc(sound->points, sound->envelopeCount * sizeof(struct ddSoundEnvelopePoint));
	
	sound->points[n].position = position;
	sound->points[n].leftLevel = left;
	sound->points[n].rightLevel = right;
}


boolean
ddSoundInstance_renderMP3IntoBuffer(ddSoundInstance* instance, ddSoundTransform xform, int* ioOffset, int* ioLoopCount, ddAudioSample* buffer, int length, ddMP3Decoder** decoderPtr, int* delay)
{
	ddSound* sound = instance->sound;
	int offset = *ioOffset;
	int loopCount = *ioLoopCount;
	ddMP3Decoder* decoder = *decoderPtr;
	
	unsigned char* data = sound->data + offset;
	unsigned char* enddata = sound->data + sound->size;
	
	ddAudioSample* startbuffer = buffer;
	ddAudioSample* endbuffer = buffer + length;
	
	buffer += *delay;

	if ( decoder == NULL )
	{
		decoder = dd_newMP3Decoder();
		*decoderPtr = decoder;
	}
	
	while ( loopCount < instance->loopCount )
	{
		int size = sound->size - offset;

		while ( data < enddata && buffer < endbuffer )
		{
			if ( !ddMP3Decoder_decodeFrame(decoder, xform, &data, size, sound->delay, &buffer, length) )
				break;
		}

		if ( data < enddata )
		{
			offset = data - sound->data;
			break;
		}
		
		offset = 0;
		++loopCount;
	}
	
	*ioLoopCount = loopCount;
	*ioOffset = offset;
	
	*delay = (buffer - startbuffer) - length;
	
	return (loopCount < instance->loopCount);
}


boolean
ddSoundInstance_renderUncompressedIntoBuffer(ddSoundInstance* instance, ddSoundTransform xform, int* ioOffset, int* ioLoopCount, ddAudioSample* buffer, int length)
{
	ddSound* sound = instance->sound;
	int offset = *ioOffset;
	int loopCount = *ioLoopCount;

	while ( loopCount < instance->loopCount )
	{
		unsigned char* data = sound->data + offset;
		int size = sound->size - offset;

		// XXX - check boundary condition when sound fits exactly
		
		if ( renderUncompressedDataIntoBuffer(data, &size, sound->sampleRate, sound->stereo, sound->bitsPerSample, xform, buffer, length) )
		{
			offset += size;
			break;
		}
		
		offset = 0;
		++loopCount;
	}

	*ioLoopCount = loopCount;
	*ioOffset = offset;

	return (loopCount < instance->loopCount);
}


boolean
ddSoundInstance_renderIntoBuffer(ddSoundInstance* instance, ddDrawClip* clip, int* offset, int* loopCount, ddAudioSample* buffer, int length, ddMP3Decoder** decoder, int* delay)
{
	ddSoundTransform xform = ddDrawClip_getSoundTransform(clip);
	
	if ( instance->sound == NULL )
		return DD_FALSE;

	switch ( instance->sound->format )
	{
		case DDSOUND_UNCOMPRESSED:
		case DDSOUND_UNCOMPRESSED_LITTLEENDIAN:
			return ddSoundInstance_renderUncompressedIntoBuffer(instance, xform, offset, loopCount, buffer, length);
		case DDSOUND_ADPCM:
			dd_warn("ADPCM not supported");
			return DD_FALSE;
		case DDSOUND_MP3:
			return ddSoundInstance_renderMP3IntoBuffer(instance, xform, offset, loopCount, buffer, length, decoder, delay);
		case DDSOUND_NELLYMOSER:
			dd_warn("Nelly-Moser not supported");
			return DD_FALSE;
	}
	
	return DD_FALSE;
}
