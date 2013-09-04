/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_FORMAT_READ_H_INCLUDED
#define DD_FORMAT_READ_H_INCLUDED

#include <stdio.h>
#include "../dd.h"

typedef struct _ddReader ddReader;

#include "../player/movieclip.h"
#include "../render/rect.h"
#include "../render/matrix.h"
#include "../render/cxform.h"

struct _dictionary
{
	char** dictionary;
	int dictSize;
};

struct _ddReader
{
	unsigned long long fileOffset;
	int buffer;
	int bufbits; /* # of bits in buffer */
	void *data;
	int length;
	float frameRate;
	int filesize;
	int version;
	
	int x;
	int y;

	int (*read)(ddReader* r, void* buf, int bytes);
	boolean (*isEOF)(ddReader *r);
	void (*setOffset)(ddReader *r, unsigned long long offset);

	ddMovieClip* playclip;
	struct _dictionary* dictionaryList;
	int nDictionaries;
	
	ddColor background;
};

ddReader*
dd_newReader(int (*read)(ddReader* r, void* buf, int bytes),
			 boolean (*isEOF)(ddReader* r),
			 void (*setOffset)(ddReader* r, unsigned long long offset),
			 void *data, int foo);

ddReader*
dd_newFileReader(FILE *f);

ddReader*
dd_newBufferReader(char* buffer, int length);

void
dd_destroyReader(ddReader* r);

void
ddReader_setBuffer(ddReader* r, char* buffer, int length);

void
ddReader_addDictionary(ddReader* r, char** dictionary, int size);

//void ddReader_setPlayclip(ddMovieClip* player);

ddMovieClip*
ddReader_getPlayclip(ddReader* r);

ddColor
ddReader_getBackgroundColor(ddReader* r);

float
ddReader_getFrameRate(ddReader* r);

void
byteAlign(ddReader* r);

int
readBits(ddReader* r, int number);

int
readSBits(ddReader* r, int number);

int
readUInt8(ddReader* r);

int
readSInt8(ddReader* r);

int
readUInt16(ddReader* r);

int
readSInt16(ddReader* r);

unsigned long
readUInt32(ddReader* r);

long
readSInt32(ddReader* r);

char*
readString(ddReader* r);

double
readDouble(ddReader* r);

unsigned char*
readBlock(ddReader* r, int length);

void
readBlockIntoBuffer(ddReader* r, int length, unsigned char* buf);

void
skipBytes(ddReader* r, int num);

ddRect
readRect(ddReader* r);

ddColor
readRGB(ddReader* r);

ddColor
readRGBA(ddReader* r);

ddMatrix
readMatrix(ddReader* r);

ddCXform
readCXform(ddReader* r, int hasAlpha);

int
ddReader_getOffset(ddReader* r);

void
ddReader_setOffset(ddReader* r, unsigned long long offset);

boolean
ddReader_isEOF(ddReader* r);

#endif /* DD_FORMAT_READ_H_INCLUDED */
