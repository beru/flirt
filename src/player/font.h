/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_FONT_H_INCLUDED
#define DD_PLAYER_FONT_H_INCLUDED

#include "../dd.h"

typedef struct _ddFont ddFont;

#include "../render/shape.h"
#include "character.h"

struct _ddFont
{
	ddCharacter parent;
	int nGlyphs;
	ddShape** glyphs;
	fixed* advances;
	fixed ascender;
	fixed descender;
	fixed leading;
	char* name;
	int mapCount;
	int* charmap;

	// XXX - missing kern table
};


ddFont*
dd_newFont(int glyphCount);

void
ddFont_addGlyph(ddFont* font, ddShape* shape, int id);

ddShape*
ddFont_getGlyph(ddFont* font, int id);

int
ddFont_getGlyphCode(ddFont* font, int charcode);

void
ddFont_addCharCode(ddFont* font, int glyph, int charcode);

fixed
ddFont_getGlyphAdvance(ddFont* font, int glyph);

#endif /* DD_PLAYER_FONT_H_INCLUDED */
