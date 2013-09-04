/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "flirt.h"

struct ringBuffer
{
	ddAudioSample* buffer;
	int bufferLength;
	
	int readOffset;
	int writeOffset;
	int loopOffset;
	int surplus;
};


struct ringBuffer*
newRingBuffer(int size);

int
ringBuffer_samplesRemaining(struct ringBuffer* ring);

void
ringBuffer_loop(struct ringBuffer* ring);

void
ringBuffer_setSurplus(struct ringBuffer* ring, int surplus);

void
ringBuffer_advanceWriteOffset(struct ringBuffer* ring, int count);

ddAudioSample*
ringBuffer_getWriteBuffer(struct ringBuffer* ring);

void
ringBuffer_writeSamplesToStream(struct ringBuffer* ring, unsigned char* stream, int samples);
