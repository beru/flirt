/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>

#include "read.h"

ddReader*
dd_newReader(int (*read)(ddReader* r, void* buf, int bytes),
			 boolean (*isEOF)(ddReader* r),
			 void (*setOffset)(ddReader* r, unsigned long long offset),
			 void* data, int foo)
{
	ddReader* r = dd_malloc(sizeof(struct _ddReader));

	r->read = read;
	r->isEOF = isEOF;
	r->setOffset = setOffset;
	r->data = data;
	r->playclip = dd_newMovieClip(NULL);
	r->fileOffset = 0;
	r->length = foo;
	r->buffer = 0;
	
	r->dictionaryList = NULL;
	r->nDictionaries = 0;

	return r;
}


void
dd_destroyReader(ddReader* r)
{
	int i;
	int j;
	
	if ( r->playclip != NULL )
		dd_destroyMovieClip((ddCharacter*)r->playclip);

	if ( r->dictionaryList != NULL )
	{
		for ( i = 0; i < r->nDictionaries; ++i )
		{
			struct _dictionary dict = r->dictionaryList[i];

			for ( j = 0; j < dict.dictSize; ++j )
				dd_free(dict.dictionary[j]);
		}

		dd_free(r->dictionaryList);
	}

	dd_free(r);
}


int
fileRead(ddReader* r, void* buf, int bytes)
{
	r->fileOffset += bytes;
	return fread(buf, 1, bytes, (FILE*)r->data);
}


boolean
fileIsEOF(ddReader* r)
{
	return feof((FILE*)r->data);
}


void
fileSetOffset(ddReader* r, unsigned long long offset)
{
	r->fileOffset = offset;
	fseek((FILE*)r->data, offset, SEEK_SET);
}


ddReader*
dd_newFileReader(FILE *f)
{
	return dd_newReader(fileRead, fileIsEOF, fileSetOffset, (void *)f, -1);
}


int
bufferRead(ddReader* r, void* buf, int bytes)
{
	if ( r->fileOffset + bytes > r->length )
		bytes = r->length - r->fileOffset;
	
	memcpy(buf, r->data + r->fileOffset, bytes);
	r->fileOffset += bytes;
	
	return bytes;
}


boolean
bufferIsEOF(ddReader* r)
{
	return r->fileOffset >= r->length;
}


void
bufferSetOffset(ddReader* r, unsigned long long offset)
{
	r->fileOffset = offset;
}


ddReader*
dd_newBufferReader(char* buffer, int length)
{
	return dd_newReader(bufferRead, bufferIsEOF, bufferSetOffset, (void*)buffer, length);
}


void
ddReader_setBuffer(ddReader* r, char* buffer, int length)
{
	r->read = bufferRead;
	r->isEOF = bufferIsEOF;
	r->data = buffer;
	r->length = length;
	r->fileOffset = 0;
	r->setOffset = bufferSetOffset;
}


int
ddReader_getOffset(ddReader* r)
{
	return r->fileOffset;
}


void
ddReader_setOffset(ddReader* r, unsigned long long offset)
{
	r->setOffset(r, offset);
}


boolean
ddReader_isEOF(ddReader* r)
{
	return r->isEOF(r);
}


float
ddReader_getFrameRate(ddReader* r)
{
	return r->frameRate;
}


ddMovieClip*
ddReader_getPlayclip(ddReader* r)
{
	return r->playclip;
}


void
ddReader_addDictionary(ddReader* r, char** dictionary, int size)
{
	r->dictionaryList = dd_realloc(r->dictionaryList, (r->nDictionaries + 1) * sizeof(struct _dictionary));

	r->dictionaryList[r->nDictionaries].dictionary = dictionary;
	r->dictionaryList[r->nDictionaries].dictSize = size;
	++r->nDictionaries;
}


int
getChar(ddReader* r)
{
	unsigned char c;
	
	if ( r->read(r, &c, 1) == 1 )
		return c;
	else
		return -1;
}


void
byteAlign(ddReader* r)
{
	if ( r->bufbits > 0 )
	{
		r->bufbits = 0;
		r->buffer = 0;
	}
}


int
readBits(ddReader* r, int number)
{
	int ret = r->buffer;

	if ( number == r->bufbits )
	{
		r->bufbits = 0;
		r->buffer = 0;
		return ret;
	}

	if ( number > r->bufbits )
	{
		number -= r->bufbits;

		while ( number > 8 )
		{
			ret <<= 8;
			ret += getChar(r);
			number -= 8;
		}

		r->buffer = getChar(r);

		if ( number > 0 )
		{
			ret <<= number;
			r->bufbits = 8 - number;
			ret += r->buffer >> (8 - number);
			r->buffer &= (1 << (r->bufbits)) - 1;
		}

		return ret;
	}

	ret = r->buffer >> (r->bufbits - number);
	r->bufbits -= number;
	r->buffer &= (1 << (r->bufbits)) - 1;

	return ret;
}


int
readSBits(ddReader* r, int number)
{
	int num = readBits(r, number);

	if ( num & (1 << (number - 1)) )
		return num - (1 << number);
	else
		return num;
}


int
readUInt8(ddReader* r)
{
	r->bufbits = 0;
	return getChar(r);
}


int
readSInt8(ddReader* r)
{
	r->bufbits = 0;
	return (signed char)getChar(r);
}


int
readSInt16(ddReader* r)
{
	return readUInt8(r) + (readSInt8(r) << 8);
}


int
readUInt16(ddReader* r)
{
	return readUInt8(r) + (readUInt8(r) << 8);
}


long
readSInt32(ddReader* r)
{
	return (long)readUInt8(r) + (readUInt8(r) << 8) +
		(readUInt8(r) << 16) + (readUInt8(r) << 24);
}


unsigned long
readUInt32(ddReader* r)
{
	return (unsigned long)(readUInt8(r) + (readUInt8(r) << 8) +
						(readUInt8(r) << 16) + (readUInt8(r) << 24));
}


void
skipBytes(ddReader* r, int num)
{
	for( ; num > 0; --num )
		readUInt8(r);
}


double
readDouble(ddReader* r)
{
	char data[8];

#ifdef DD_LITTLE_ENDIAN
	data[4] = readUInt8(r);
	data[5] = readUInt8(r);
	data[6] = readUInt8(r);
	data[7] = readUInt8(r);
	data[0] = readUInt8(r);
	data[1] = readUInt8(r);
	data[2] = readUInt8(r);
	data[3] = readUInt8(r);
#else
	data[7] = readUInt8(r);
	data[6] = readUInt8(r);
	data[5] = readUInt8(r);
	data[4] = readUInt8(r);
	data[3] = readUInt8(r);
	data[2] = readUInt8(r);
	data[1] = readUInt8(r);
	data[0] = readUInt8(r);
#endif
	
	return *((double*)data);
}


unsigned char*
readBlock(ddReader* r, int length)
{
	unsigned char* buf = dd_malloc(length);

	r->read(r, buf, length);

	return buf;
}


void
readBlockIntoBuffer(ddReader* r, int length, unsigned char* buf)
{
	r->read(r, buf, length);
}


char*
readString(ddReader* r)
{
	int len = 0, buflen = 256;
	char c, *buf, *p;

	buf = (char*)malloc(sizeof(char) * 256);
	p = buf;

	while ( (c = (char)readUInt8(r)) != '\0' )
	{
		if ( len > buflen - 3 )
		{
			buf = (char* )realloc(buf, sizeof(char) * (buflen + 256));
			buflen += 256;
			p = buf+len;
		}

		switch ( c )
		{
			case '\n':
				*(p++) = '\\';	*(p++) = 'n';	++len;	break;
			case '\t':
				*(p++) = '\\';	*(p++) = 't';	++len;	break;
			case '\r':
				*(p++) = '\\';	*(p++) = 'r';	++len;	break;
			default:
				*(p++) = c;
		}

		++len;
	}

	*p = 0;

	return buf;
}


ddRect
readRect(ddReader* r)
{
	int nBits, xMin, xMax, yMin, yMax;

	byteAlign(r);

	nBits = readBits(r, 5);
	xMin = readSBits(r, nBits);
	xMax = readSBits(r, nBits);
	yMin = readSBits(r, nBits);
	yMax = readSBits(r, nBits);

	return dd_makeRect(FIXED_T(xMin), FIXED_T(xMax),
				   FIXED_T(yMin), FIXED_T(yMax));
}


ddMatrix
readMatrix(ddReader* r)
{
	int nBits, x, y;

	fixed xScale = FIXED_D(1.0), yScale = FIXED_D(1.0),
		rotate0 = FIXED_D(0), rotate1 = FIXED_D(0);

	byteAlign(r);

	/* XXX - severe loss of accuracy here! */

	if ( readBits(r, 1) ) /* has scale */
	{
		nBits = readBits(r, 5);
		xScale = FIXED_D((float)readSBits(r, nBits) / 0x10000);
		yScale = FIXED_D((float)readSBits(r, nBits) / 0x10000);
	}

	if ( readBits(r, 1) ) /* has rotate */
	{
		nBits = readBits(r, 5);
		rotate1 = FIXED_D((float)readSBits(r, nBits) / 0x10000);
		rotate0 = FIXED_D((float)readSBits(r, nBits) / 0x10000);
	}

	nBits = readBits(r, 5);
	x = readSBits(r, nBits);
	y = readSBits(r, nBits);

	return dd_makeMatrix(xScale, rotate0, rotate1, yScale,
					 FIXED_T(x), FIXED_T(y));
}


ddColor
readRGB(ddReader* r)
{
	unsigned char red = readUInt8(r);
	unsigned char green = readUInt8(r);
	unsigned char blue = readUInt8(r);

	return DDCOLOR(0xff, red, green, blue);
}


ddColor
readRGBA(ddReader* r)
{
	unsigned char red = readUInt8(r);
	unsigned char green = readUInt8(r);
	unsigned char blue = readUInt8(r);
	unsigned char alpha = readUInt8(r);

	return DDCOLOR(alpha, red, green, blue);
}


ddCXform
readCXform(ddReader* r, int hasAlpha)
{
	int hasAdd, hasMult, nBits;

	SInt16 rMult = 0x100;
	SInt16 gMult = 0x100;
	SInt16 bMult = 0x100;
	SInt16 aMult = 0x100;

	SInt16 rAdd = 0;
	SInt16 gAdd = 0;
	SInt16 bAdd = 0;
	SInt16 aAdd = 0;

	byteAlign(r);

	hasAdd = readBits(r, 1);
	hasMult = readBits(r, 1);
	nBits = readBits(r, 4);

	if ( hasMult )
	{
		rMult = readSBits(r, nBits);
		gMult = readSBits(r, nBits);
		bMult = readSBits(r, nBits);

		if ( hasAlpha )
			aMult = readSBits(r, nBits);
	}

	if ( hasAdd )
	{
		rAdd = readSBits(r, nBits);
		gAdd = readSBits(r, nBits);
		bAdd = readSBits(r, nBits);

		if ( hasAlpha )
			aAdd = readSBits(r, nBits);
	}

	return dd_makeCXform(rMult, gMult, bMult, aMult, rAdd, gAdd, bAdd, aAdd);
}
