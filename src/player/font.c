/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "font.h"

static void
dd_destroyFont(ddCharacter* c)
{
	ddFont* font = (ddFont*)c;
	int i;

	for ( i = 0; i < font->nGlyphs; ++i )
	{
		if ( font->glyphs[i] != NULL )
			dd_destroyShape(font->glyphs[i]);
	}

	dd_free(font->glyphs);
	dd_free(font->advances);
	dd_free(font->charmap);

	if ( font->name != NULL )
		dd_free(font->name);
	
	dd_free(c);
}


ddFont*
dd_newFont(int glyphCount)
{
	ddFont* f = dd_malloc(sizeof(ddFont));
	int i;

	f->parent.type = FONT_CHAR;
	f->parent.destroy = dd_destroyFont;
	f->parent.instantiate = NULL;

	f->glyphs = dd_malloc(glyphCount * sizeof(ddShape*));
	f->advances = dd_malloc(glyphCount * sizeof(fixed));
	f->nGlyphs = glyphCount;

	for ( i = 0; i < glyphCount; ++i )
	{
		f->glyphs[i] = NULL;
		f->advances[i] = 0;
	}

	f->name = NULL;

	f->mapCount = 256;
	f->charmap = dd_malloc(256 * sizeof(int));
	
	for ( i = 0; i < 256; ++i )
		f->charmap[i] = -1;

	return f;
}


int
ddFont_getGlyphCode(ddFont* font, int charcode)
{
	if ( charcode >= 256 )
	{
		dd_warn("Only 256 char codes supported");
		return -1;
	}

	return font->charmap[charcode];
}


void
ddFont_addCharCode(ddFont* font, int glyph, int charcode)
{
	if ( charcode >= 256 )
		dd_warn("Only 256 char codes supported");
	else
		font->charmap[charcode] = glyph;
}


void
ddFont_addGlyph(ddFont* font, ddShape* shape, int id)
{
	if ( id >= font->nGlyphs )
	{
		font->nGlyphs = id + 1;
		font->glyphs = dd_realloc(font->glyphs, font->nGlyphs * sizeof(ddShape*));
	}

	font->glyphs[id] = shape;
}


ddShape*
ddFont_getGlyph(ddFont* font, int id)
{
	if ( id >= font->nGlyphs )
		return NULL;

	return font->glyphs[id];
}


fixed
ddFont_getGlyphAdvance(ddFont* font, int id)
{
	fixed advance;

	if ( id >= font->nGlyphs )
		return 0;

	advance = font->advances[id];
	
	if ( advance != 0 )
		return advance;

	// XXX - use layout info if available

	return ddRect_getWidth(ddShape_getBounds(font->glyphs[id]));
}
