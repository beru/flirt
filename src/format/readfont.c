/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "readfont.h"
#include "../player/font.h"

static int
readFontShapeRec(ddReader* r, ddShape* shape, int *fillBits, int *lineBits)
{
	int type = readBits(r, 1);

	if ( type == 0 ) /* state change */
	{
		int flags = readBits(r, 1);

		if ( flags == 1 ) // newstyles flag
			return 1;
		
#define SHAPEREC_NEWSTYLES  (1<<4)
#define SHAPEREC_LINESTYLE  (1<<3)
#define SHAPEREC_FILLSTYLE1 (1<<2)
#define SHAPEREC_FILLSTYLE0 (1<<1)
#define SHAPEREC_MOVETO     (1<<0)

		flags = readBits(r, 4);

		if ( flags == 0 )
			return 0;

		if ( flags & SHAPEREC_MOVETO )
		{
			int moveBits = readBits(r, 5);
			r->x = readSBits(r, moveBits);
			r->y = readSBits(r, moveBits);
			ddShape_moveTo(shape, FIXED_T(r->x), FIXED_T(r->y));
		}

		if ( flags & SHAPEREC_FILLSTYLE0 )
		{
			readBits(r, *fillBits);
			ddShape_setLeftFill(shape, 0, 1);
		}

		if ( flags & SHAPEREC_FILLSTYLE1 )
		{
			readBits(r, *fillBits);
			ddShape_setRightFill(shape, 0, 1);
		}

		if ( flags & SHAPEREC_LINESTYLE )
		{
			readBits(r, *lineBits);
			ddShape_setLine(shape, 0, 0);
		}

		if ( flags & SHAPEREC_NEWSTYLES )
		{
			dd_warn("found newstyles flag in font shape record\n");
			return 0;
		}
	}
	else /* it's an edge record */
	{
		int straight = readBits(r, 1);
		int numBits = readBits(r, 4) + 2;

		if ( straight == 1 )
		{
			if ( readBits(r, 1) ) /* general line */
			{
				r->x += readSBits(r, numBits);
				r->y += readSBits(r, numBits);
			}
			else
			{
				if ( readBits(r, 1) ) /* vert = 1 */
					r->y += readSBits(r, numBits);
				else
					r->x += readSBits(r, numBits);
			}

			ddShape_lineTo(shape, FIXED_T(r->x), FIXED_T(r->y));
		}
		else
		{
			int x1 = readSBits(r, numBits);
			int y1 = readSBits(r, numBits);
			int x2 = readSBits(r, numBits);
			int y2 = readSBits(r, numBits);

			ddShape_curveTo(shape,
				   FIXED_T(r->x + x1), FIXED_T(r->y + y1),
				   FIXED_T(r->x + x1 + x2), FIXED_T(r->y + y1 + y2));

			r->x += x1 + x2;
			r->y += y1 + y2;
		}
	}

	return 1;
}


void
readDefineFont(ddMovieClip* p, ddReader* r, int length)
{
	int here, off0, off, i, nShapes, fillBits, lineBits;
	int *offset;
	ddFont* font;
	ddShape* shape;

	int fontID = readUInt16(r);

	off0 = readUInt16(r);

	nShapes = off0 / 2;
	font = dd_newFont(nShapes);
	
	offset = dd_malloc(nShapes * sizeof(int));

	for ( i = 1; i < nShapes; ++i )
	{
		off = readUInt16(r);
		offset[i-1] = off-off0;
	}

	offset[nShapes-1] = length - 2 - (nShapes * 2);

	here = ddReader_getOffset(r);

	r->x = 0;
	r->y = 0;

	for ( i = 0; i < nShapes; ++i )
	{
		byteAlign(r);

		fillBits = readBits(r, 4);
		lineBits = readBits(r, 4);

		if ( ddReader_getOffset(r) == here + offset[i] )
		{
			ddFont_addGlyph(font, NULL, i);
			continue;
		}

		shape = dd_newShape();

		while ( ddReader_getOffset(r) < here + offset[i] )
			readFontShapeRec(r, shape, &fillBits, &lineBits);

		ddFont_addGlyph(font, shape, i);
	}

	dd_free(offset);

	ddMovieClip_addCharacter(p, fontID, (ddCharacter*)font);
}

#define FONTINFO2_HASLAYOUT		(1<<7)
#define FONTINFO2_SHIFTJIS		(1<<6)
#define FONTINFO2_UNICODE		(1<<5)
#define FONTINFO2_ANSI			(1<<4)
#define FONTINFO2_WIDEOFFSETS	        (1<<3)
#define FONTINFO2_WIDECODES		(1<<2)
#define FONTINFO2_ITALIC		(1<<1)
#define FONTINFO2_BOLD			(1<<0)

void
readDefineFont2(ddMovieClip* p, ddReader* r, int length)
{
	int nGlyphs, namelen, i, fillBits, lineBits;
	int here = ddReader_getOffset(r);
	char* name;

	ddShape* shape;
	ddFont* font;

	unsigned int* offset;
	int fontID = readUInt16(r);
	int flags = readUInt8(r);

	readUInt8(r); /* "reserved" */

	namelen = readUInt8(r);
	name = dd_malloc(namelen+1);

	for ( i = 0; i < namelen; ++i )
		name[i] = readUInt8(r);

	name[namelen] = '\0';

	nGlyphs = readUInt16(r);
	font = dd_newFont(nGlyphs);
	
	offset = (unsigned int *)malloc((nGlyphs+1)*sizeof(int));

	/* offset table */

	here = ddReader_getOffset(r);

	for ( i = 0; i <= nGlyphs; ++i )
	{
		if ( flags & FONTINFO2_WIDEOFFSETS )
			offset[i] = readUInt32(r) - 4 * nGlyphs - 4;
		else
			offset[i] = readUInt16(r) - 2 * nGlyphs - 2;
	}

	here = ddReader_getOffset(r);

	r->x = 0;
	r->y = 0;

	/* shape table */
	for ( i = 0; i < nGlyphs; ++i )
	{
		byteAlign(r);

		fillBits = readBits(r, 4);
		lineBits = readBits(r, 4);

		byteAlign(r);

		if ( ddReader_getOffset(r) == here + offset[i] )
		{
			ddFont_addGlyph(font, NULL, i);
			continue;
		}

		shape = dd_newShape();

		while ( ddReader_getOffset(r) < here + offset[i+1] )
		{
			if ( readFontShapeRec(r, shape, &fillBits, &lineBits) == 0 )
				break;
		}

		byteAlign(r);

		while ( ddReader_getOffset(r) < here + offset[i+1] )
			readUInt8(r);
		
		ddFont_addGlyph(font, shape, i);
	}

	dd_free(offset);

	ddMovieClip_addCharacter(p, fontID, (ddCharacter*)font);

	for ( i = 0; i < nGlyphs; ++i )
	{
		int code;

		if ( flags & FONTINFO2_WIDECODES )
			code = readUInt16(r);
		else
			code = readUInt8(r);

		ddFont_addCharCode(font, i, code);
	}

	// XXX - add support for layout info

	if ( flags & FONTINFO2_HASLAYOUT )
	{
		int kernCount, code1, code2, adjustment;

		font->ascender = FIXED_T(readSInt16(r));
		font->descender = FIXED_T(readSInt16(r));
		font->leading = FIXED_T(readSInt16(r));

		for ( i = 0; i < nGlyphs; ++i )
			font->advances[i] = FIXED_T(readSInt16(r));

		for ( i = 0; i < nGlyphs; ++i )
			readRect(r); /* bounds */

		kernCount = readUInt16(r);

		for ( i = 0; i < kernCount; ++i )
		{
			code1 = (flags & FONTINFO2_WIDECODES) ? readUInt16(r) : readUInt8(r);
			code2 = (flags & FONTINFO2_WIDECODES) ? readUInt16(r) : readUInt8(r);
			adjustment = readSInt16(r);
		}
	}
}

#define FONTINFO_RESERVED	(1<<6 | 1<<7)
#define FONTINFO_UNICODE	(1<<5)
#define FONTINFO_SHIFTJIS	(1<<4)
#define FONTINFO_ANSI		(1<<3)
#define FONTINFO_ITALIC		(1<<2)
#define FONTINFO_BOLD		(1<<1)
#define FONTINFO_WIDE		(1<<0)

void readFontInfo(ddMovieClip* p, ddReader* r, int length)
{
	int flags, i;
	int fontID = readUInt16(r);
	char* name;

	int namelen = readUInt8(r);
	int nGlyphs = length - namelen - 4;

	ddCharacter* c = ddMovieClip_getCharacter(p, fontID);
	ddFont* font;

	if ( c == NULL || c->type != FONT_CHAR )
	{
		dd_warn("fontInfo found for non-font character");
		return;
	}

	font = (ddFont*)c;

	name = dd_malloc(namelen + 1);

	for ( i = 0; i < namelen; ++i )
		name[i] = readUInt8(r);

	name[namelen] = '\0';

	font->name = name;

	flags = readUInt8(r);

	if ( flags & FONTINFO_WIDE )
		nGlyphs /= 2;

	// XXX - other flags we should worry about?

	for ( i = 0; i < nGlyphs; ++i )
	{
		int code;
		
		if ( flags & FONTINFO_WIDE )
			code = readUInt16(r);
		else
			code = readUInt8(r);

		ddFont_addCharCode(font, i, code);
	}
}


void readFontInfo2(ddMovieClip* p, ddReader* r, int length)
{
	int end = ddReader_getOffset(r) + length;
	int fontid = readUInt16(r);
	int namelen = readUInt8(r);
	int flags;
	int i;
	int nGlyphs;
	int* glyphCodes;
	int languageCode;
	
	char* name = dd_malloc(namelen + 1);
	
	ddCharacter* c = ddMovieClip_getCharacter(p, fontid);
	ddFont* font;
	
	if ( c == NULL || c->type != FONT_CHAR )
	{
		dd_warn("fontInfo found for non-font character");
		return;
	}
	
	font = (ddFont*)c;
	
	for ( i = 0; i < namelen; ++i )
		name[i] = readUInt8(r);
	
	name[namelen] = '\0';
	
	flags = readUInt8(r);
	
	languageCode = readUInt8(r);
	
	nGlyphs = (end - ddReader_getOffset(r)) / 2;
	
	glyphCodes = dd_malloc(nGlyphs * sizeof(int));
	
	for ( i = 0; i < nGlyphs; ++i )
		glyphCodes[i] = readUInt16(r);
}

