/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "text.h"
#include "drawtext.h"
#include "player.h"

static void
dd_destroyText(ddCharacter* c)
{
	ddText* text = (ddText*)c;
	struct _ddTextRecord *record = text->record;
	struct _ddTextRecord *next;

	while ( record != NULL )
	{
		next = record->next;

		dd_free(record->glyphs);
		dd_free(record->advances);
		dd_free(record);

		record = next;
	}

	dd_free(text);
}


static ddDrawable*
ddText_instantiate(ddPlayer* player, ddCharacter* c)
{
	return (ddDrawable*)dd_newDrawText((ddText*)c);
}


ddText*
dd_newText()
{
	ddText* text = dd_malloc(sizeof(ddText));
	struct _ddTextRecord *record = dd_malloc(sizeof(struct _ddTextRecord));

	text->parent.type = TEXT_CHAR;
	text->parent.destroy = dd_destroyText;
	text->parent.instantiate = ddText_instantiate;

	text->record = text->tail = record;
	text->bounds = ddInvalidRect;
	text->matrix = ddEmptyMatrix;

	record->next = NULL;
	record->font = NULL;
	record->color = DDCOLOR(0xff, 0, 0, 0);
	record->height = 1024;
	record->x = 0;
	record->y = 0;
	record->xCur = 0;
	record->nGlyphs = 0;
	record->glyphs = NULL;
	record->advances = NULL;

	return text;
}


/* parser is going to signal us to make a new record.
kinda clumsy, but more efficient I guess.. */

void
ddText_newRecord(ddText* text)
{
	struct _ddTextRecord *record = dd_malloc(sizeof(struct _ddTextRecord));

	record->next = NULL;
	record->font = text->tail->font;
	record->color = text->tail->color;
	record->height = text->tail->height;
	record->x = text->tail->xCur;
	record->y = text->tail->y;
	record->xCur = record->x;
	record->nGlyphs = 0;
	record->glyphs = NULL;
	record->advances = NULL;

	text->tail->next = record;
	text->tail = record;
}


void
ddText_setBounds(ddText* text, ddRect bounds)
{
	text->bounds = bounds;
}


void
ddText_setMatrix(ddText* text, ddMatrix matrix)
{
	text->matrix = matrix;
}


void
ddText_setHeight(ddText* text, int height)
{
	text->tail->height = height;
}


void
ddText_setXOffset(ddText* text, int x)
{
	text->tail->x = x;
}


void
ddText_setYOffset(ddText* text, int y)
{
	text->tail->y = y;
}


void
ddText_setColor(ddText* text, ddColor color)
{
	text->tail->color = color;
}


void
ddText_setFont(ddText* text, ddFont* font)
{
	text->tail->font = font;
}


void
ddText_addGlyph(ddText* text, int glyph, fixed advance)
{
	struct _ddTextRecord *record = text->tail;
	int nGlyphs = record->nGlyphs;

	record->glyphs = dd_realloc(record->glyphs, (nGlyphs + 1) * sizeof(int));
	record->advances = dd_realloc(record->advances, (nGlyphs + 1) * sizeof(fixed));

	record->glyphs[nGlyphs] = glyph;
	record->advances[nGlyphs] = advance;
	record->xCur += advance;
	++record->nGlyphs;
}
