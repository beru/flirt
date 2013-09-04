/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>
#include <zlib.h>

#include "readthread.h"

#include "readshape.h"
#include "readplace.h"
#include "readbitmap.h"
#include "readjpeg.h"
#include "readfont.h"
#include "readtext.h"
#include "readsprite.h"
#include "readbutton.h"
#include "readsound.h"

#include "blocktypes.h"

void
readBackgroundColor(ddMovieClip* p, ddReader* r, int length)
{
	r->background = readRGB(r);
}


ddColor
ddReader_getBackgroundColor(ddReader* r)
{
	return r->background;
}


void
readShowFrame(ddMovieClip* p, ddReader* r, int length)
{
	ddMovieClip_nextFrame(p);
}


int
readMovie(ddReader* r)
{
	ddRect bounds;
	ddMovieClip* p = r->playclip;
	int byte = readUInt8(r);
	
	if ( byte == 'C' )
	{
		// file is compressed

		unsigned char* buf = NULL;
		unsigned char* dest;
		unsigned long size;
		
		int len = 0;
		
		if ( readUInt8(r) != 'W' || readUInt8(r) != 'S' )
			return -1;

		r->version = readUInt8(r);
		size = readUInt32(r);

		dest = dd_malloc(size);

		// XXX - should do this in a streaming way

		while ( !ddReader_isEOF(r) )
		{
			if ( len % 1024 == 0 )
				buf = dd_realloc(buf, len + 1024);
			
			buf[len++] = readUInt8(r);
		}

		if ( uncompress(dest, &size, buf, len) != Z_OK )
			return -1;

		ddReader_setBuffer(r, (char*)dest, size);
		
		p->bytesTotal = size;
		p->bytesTotal = r->filesize = size;
	}
	else
	{
		if ( byte != 'F' || readUInt8(r) != 'W' || readUInt8(r) != 'S' )
			return -1;

		r->version = readUInt8(r);
		p->bytesTotal = r->filesize = readUInt32(r);
	}

	p->bytesLoaded = p->bytesTotal; // XXX

	bounds = readRect(r);
	ddMovieClip_setWidth(p, ddRect_getWidth(bounds));
	ddMovieClip_setHeight(p, ddRect_getHeight(bounds));

	r->frameRate = readUInt8(r) / 256.0;
	r->frameRate += readUInt8(r);

	p->framesTotal = readUInt16(r);

	for ( ;; )
	{
		int offset = ddReader_getOffset(r);
		int nextoffset;
		
		int block = readUInt16(r);
		Blocktype type = block >> 6;

		int length = block & ((1 << 6) - 1);

		if ( length == 63 ) /* it's a long block. */
			length = readUInt32(r);
		
		nextoffset = ddReader_getOffset(r) + length;

		if ( type == 0 || ddReader_getOffset(r) >= r->filesize || length < 0 )
			return 0;

		switch ( type )
		{
			case DEFINESHAPE:
				readDefineShape(p, r, length);
				break;
				
			case DEFINESHAPE2:
				readDefineShape2(p, r, length);
				break;
				
			case DEFINESHAPE3:
				readDefineShape3(p, r, length);
				break;
				
			case DEFINEMORPHSHAPE:
				readDefineMorphShape(p, r, length);
				break;
				
			case PLACEOBJECT:
				readPlaceObject(p, r, length);
				break;
				
			case PLACEOBJECT2:
				readPlaceObject2(p, r, length);
				break;
				
			case REMOVEOBJECT:
				readRemoveObject(p, r, length);
				break;
				
			case REMOVEOBJECT2:
				readRemoveObject2(p, r, length);
				break;
				
			case SHOWFRAME:
				readShowFrame(p, r, length);
				break;
				
			case DEFINELOSSLESS:
				readDefineBitsLossless(p, r, length, 1);
				break;

			case DEFINELOSSLESS2:
				readDefineBitsLossless(p, r, length, 2);
				break;

			case JPEGTABLES:
				readJpegTables(p, r, length);
				break;

			case DEFINEBITS:
				readDefineBitsJpeg(p, r, length);
				break;

			case DEFINEBITSJPEG2:
				readDefineBitsJpeg2(p, r, length);
				break;

			case DEFINEBITSJPEG3:
				readDefineBitsJpeg3(p, r, length);
				break;
			
			case DEFINETEXT:
				readDefineText(p, r, length);
				break;
				
			case DEFINETEXT2:
				readDefineText2(p, r, length);
				break;
				
			case DEFINEFONTINFO:
				readFontInfo(p, r, length);
				break;
				
			case DEFINEFONTINFO2:
				readFontInfo2(p, r, length);
				break;
				
			case DEFINEFONT:
				readDefineFont(p, r, length);
				break;
				
			case DEFINEFONT2:
				readDefineFont2(p, r, length);
				break;
				
			case DEFINESPRITE:
				readDefineSprite(p, r, length);
				break;
				
			case SETBACKGROUNDCOLOR:
				readBackgroundColor(p, r, length);
				break;
				
			case FRAMELABEL:
				readFrameLabel(p, r, length);
				break;
				
			case DOACTION:
				readClipAction(p, r, length);
				break;

			case INITCLIPACTION:
				readInitClipAction(p, r, length);
				break;
				
			case DEFINEBUTTON:
				readDefineButton(p, r, length);
				break;
				
			case DEFINEBUTTON2:
				readDefineButton2(p, r, length);
				break;
				
			case LIBRARYSYMBOL:
				readLibrarySymbol(p, r, length);
				break;
				
			case TEXTFIELD:
				readTextField(p, r, length);
				break;
				
			case DEFINESOUND:
				readDefineSound(p, r, length);
				break;

			case STARTSOUND:
				readStartSound(p, r, length);
				break;

			case DEFINEBUTTONSOUND:
				skipBytes(r, length);
				break;
				
			case SOUNDSTREAMHEAD:
			case SOUNDSTREAMHEAD2:
				readSoundStreamHead(p, r, length);
				break;
				
			case SOUNDSTREAMBLOCK:
				readSoundStreamBlock(p, r, length);
				break;

			case PROTECT:
			case PASSWORD:
				skipBytes(r, length);
				break;

			default:
				dd_warn("Unrecognized block at 0x%x: type %i, length %i\n", offset, type, length);
				skipBytes(r, length);
		}
		
		ddReader_setOffset(r, nextoffset);
	}

	return 0;
}
