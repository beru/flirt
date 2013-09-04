/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ringbuffer.h"

struct ringBuffer*
newRingBuffer(int size)
{
	struct ringBuffer* ring = malloc(sizeof(struct ringBuffer));
	
	ring->buffer = malloc(sizeof(ddAudioSample) * size);
	ring->bufferLength = size;
	ring->loopOffset = size;
	ring->readOffset = 0;
	ring->writeOffset = 0;
	ring->surplus = 0;
	
	return ring;
}


int
ringBuffer_samplesRemaining(struct ringBuffer* ring)
{
	return ring->bufferLength - ring->writeOffset;
}


void
ringBuffer_loop(struct ringBuffer* ring)
{
	if ( ring->readOffset < ring->surplus )
	{
		printf("Not enough free space to loop!");
		return;
	}
	
	memcpy(ring->buffer, ring->buffer + ring->writeOffset, ring->surplus);

	ring->loopOffset = ring->writeOffset;
	ring->writeOffset = 0;
}


void
ringBuffer_setSurplus(struct ringBuffer* ring, int surplus)
{
	ring->surplus = surplus;
}


ddAudioSample*
ringBuffer_getWriteBuffer(struct ringBuffer* ring)
{
	return ring->buffer + ring->writeOffset;
}


void
ringBuffer_advanceWriteOffset(struct ringBuffer* ring, int count)
{
	ring->writeOffset += count;
}


void
ringBuffer_writeSamplesToStream(struct ringBuffer* ring, unsigned char* stream, int samples)
{
	short* output = (short*)stream;
	
	if ( ring->readOffset > ring->writeOffset )
	{
		ddAudioSample* p = ring->buffer + ring->readOffset;
		ddAudioSample* end;
		
		if ( ring->readOffset + samples > ring->loopOffset )
			end = ring->buffer + ring->loopOffset;
		else
			end = ring->buffer + ring->readOffset + samples;
		
		while ( p < end )
		{
			*output++ = DD_SAMPLE_SINT16(p->left >> 8);
			*output++ = DD_SAMPLE_SINT16(p->right >> 8);
			++p;
		}
		
		if ( end < ring->buffer + samples )
		{
			ring->readOffset += samples;
			return;
		}
		
		samples -= ring->loopOffset - ring->readOffset;
		ring->readOffset = 0;
	}
	
	if ( ring->readOffset < ring->writeOffset )
	{
		ddAudioSample* p = ring->buffer + ring->readOffset;
		ddAudioSample* end;
		
		if ( ring->readOffset + samples > ring->writeOffset )
		{
			printf("audio underflow\n");
			samples = ring->writeOffset - ring->readOffset;
		}
		
		end = ring->buffer + ring->readOffset + samples;
		
		while ( p < end )
		{
			*output++ = DD_SAMPLE_SINT16(p->left >> 16);
			*output++ = DD_SAMPLE_SINT16(p->right >> 16);
			++p;
		}
		
		ring->readOffset += samples;
	}
}
