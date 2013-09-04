/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_TEXT_H_INCLUDED
#define DD_PLAYER_TEXT_H_INCLUDED

#include "../dd.h"

typedef struct _ddText ddText;
typedef struct _ddTextRecord ddTextRecord;

#include "../render/rect.h"
#include "character.h"
#include "font.h"

struct _ddTextRecord
{
	struct _ddTextRecord *next;
	ddFont* font;
	ddColor color;
	int height;
	int x;
	int y;
	int xCur;
	int nGlyphs;
	int *glyphs;
	fixed *advances;
};

struct _ddText
{
	ddCharacter parent;
	struct _ddTextRecord *record;
	struct _ddTextRecord *tail;
	ddRect bounds;
	ddMatrix matrix;
};


ddText*
dd_newText();

void
ddText_newRecord(ddText* text);

void
ddText_setBounds(ddText* text, ddRect bounds);

void
ddText_setMatrix(ddText* text, ddMatrix matrix);

void
ddText_setHeight(ddText* text, int height);

void
ddText_setXOffset(ddText* text, int x);

void
ddText_setYOffset(ddText* text, int y);

void
ddText_setColor(ddText* text, ddColor color);

void
ddText_setFont(ddText* text, ddFont* font);

void
ddText_addGlyph(ddText* text, int glyph, fixed advance);

#endif /* DD_PLAYER_TEXT_H_INCLUDED */
