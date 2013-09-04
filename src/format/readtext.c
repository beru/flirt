/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "readtext.h"
#include "blocktypes.h"
#include "../player/text.h"
#include "../player/textfield.h"

#define TEXTRECORD_STATECHANGE	(1<<7)
#define TEXTRECORD_RESERVED	(1<<6 | 1<<5 | 1<<4)
#define TEXTRECORD_HASFONT	(1<<3)
#define TEXTRECORD_HASCOLOR	(1<<2)
#define TEXTRECORD_HASYOFF	(1<<1)
#define TEXTRECORD_HASXOFF	(1<<0)
#define TEXTRECORD_NUMGLYPHS	0x7f

int
readTextRecord(ddMovieClip* p, ddReader* r, ddText* text,
			   int glyphBits, int advanceBits, Blocktype type)
{
	int flags = readUInt8(r);

	int numGlyphs;
	int i;
	
	if ( flags == 0 )
		return 0;

	if ( text->record->nGlyphs != 0 )
		ddText_newRecord(text);

	if ( flags & TEXTRECORD_HASFONT )
	{
		int fontID = readUInt16(r);
		ddCharacter* font = ddMovieClip_getCharacter(p, fontID);

		/* XXX */
		if ( font != NULL && font->type != FONT_CHAR )
			dd_error("Selected character isn't a font!\n");

		ddText_setFont(text, (ddFont*)font);
	}

	if ( flags & TEXTRECORD_HASCOLOR )
	{
		if ( type == DEFINETEXT2 )
			ddText_setColor(text, readRGBA(r));
		else
			ddText_setColor(text, readRGB(r));
	}

	if ( flags & TEXTRECORD_HASXOFF )
		ddText_setXOffset(text, FIXED_T(readSInt16(r)));

	if ( flags & TEXTRECORD_HASYOFF )
		ddText_setYOffset(text, FIXED_T(readSInt16(r)));

	if ( flags & TEXTRECORD_HASFONT )
		ddText_setHeight(text, FIXED_I(readUInt16(r))>>10);

	numGlyphs = readUInt8(r);

	for ( i = 0; i < numGlyphs; ++i )
	{
		int index = readBits(r, glyphBits);
		fixed advance = FIXED_T(readSBits(r, advanceBits));

		ddText_addGlyph(text, index, advance);
	}

	return 1;
}


void
readDefineText(ddMovieClip* p, ddReader* r, int length)
{
	int end = ddReader_getOffset(r) + length;
	int glyphBits, advanceBits;
	int characterID = readUInt16(r);

	ddText* text = dd_newText();
	ddText_setBounds(text, readRect(r));

	byteAlign(r);

	ddText_setMatrix(text, readMatrix(r));

	glyphBits = readUInt8(r);
	advanceBits = readUInt8(r);

	while ( ddReader_getOffset(r) < end )
	{
		if ( readTextRecord(p, r, text, glyphBits, advanceBits, DEFINETEXT) == 0 )
			break;
	}

	ddMovieClip_addCharacter(p, characterID, (ddCharacter*)text);
}


void
readDefineText2(ddMovieClip* p, ddReader* r, int length)
{
	int end = ddReader_getOffset(r) + length;
	int glyphBits, advanceBits;
	int characterID = readUInt16(r);

	ddText* text = dd_newText();
	ddText_setBounds(text, readRect(r));

	byteAlign(r);

	ddText_setMatrix(text, readMatrix(r));

	glyphBits = readUInt8(r);
	advanceBits = readUInt8(r);

	while ( ddReader_getOffset(r) < end )
	{
		if ( readTextRecord(p, r, text, glyphBits, advanceBits, DEFINETEXT2) == 0 )
			break;
	}

	ddMovieClip_addCharacter(p, characterID, (ddCharacter*)text);
}


void
readTextField(ddMovieClip* p, ddReader* r, int length)
{
	int flags, end = ddReader_getOffset(r) + length;
	int fontID;
	ddCharacter* font;

	ddTextField* text = dd_newTextField();

	int characterID = readUInt16(r);

	ddTextField_setBounds(text, readRect(r));

	flags = readUInt16(r);
	ddTextField_setFlags(text, flags);

	fontID = readUInt16(r);
	font = ddMovieClip_getCharacter(p, fontID);

	/* XXX */
	if ( font->type != FONT_CHAR )
		dd_error("Selected character isn't a font!\n");

	ddTextField_setFont(text, (ddFont*)font);

	ddTextField_setHeight(text, FIXED_I(readUInt16(r))>>10);
	ddTextField_setColor(text, readRGBA(r));

	if ( flags & TEXTFIELD_HASLENGTH )
		ddTextField_setLength(text, readUInt16(r));

	ddTextField_justify(text, readUInt8(r));

	ddTextField_setLeftMargin(text, readUInt16(r));
	ddTextField_setRightMargin(text, readUInt16(r));
	ddTextField_setIndentation(text, readUInt16(r));
	ddTextField_setLineSpacing(text, readUInt16(r));

	ddTextField_setVariableName(text, readString(r));

	if ( ddReader_getOffset(r) < end )
		ddTextField_setText(text, readString(r));

	if ( ddReader_getOffset(r) < end )
		skipBytes(r, end-ddReader_getOffset(r));

	ddMovieClip_addCharacter(p, characterID, (ddCharacter*)text);
}
