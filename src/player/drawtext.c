/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "drawtext.h"

static void
ddDrawText_getUpdateList(ddDrawable* d, ddUpdateList* list, ddMatrix matrix)
{
	ddMatrix m;
	ddMatrix xform;

	int i;
	ddDrawText* t = (ddDrawText*)d;

	d->bounds = ddInvalidRect;
	xform = ddMatrix_multiply(matrix, d->matrix);

	for ( i = 0; i < t->nRecords; ++i )
	{
		ddDrawTextRecord* record = &(t->records[i]);

		m = dd_makeMatrix(record->height, 0, 0, record->height, record->x, record->y);
		m = ddMatrix_multiply(xform, m);

		ddShapeInstance_getUpdateList(record->shape, list, m);
		d->bounds = ddRect_containRect(d->bounds, record->shape->bounds);
	}
}


static void
ddDrawText_drawInImage(ddDrawable* d, ddImage* image, ddUpdateList* list,
					   ddMatrix matrix, ddCXform cXform, ddRect clipRect)
{
	ddMatrix xform;
	ddMatrix m;

	int i;
	ddDrawText* t = (ddDrawText*)d;

	xform = ddMatrix_multiply(matrix, d->matrix);

	for ( i = 0; i < t->nRecords; ++i )
	{
		ddDrawTextRecord* record = &(t->records[i]);
		ddCXform c = ddCXform_compose(cXform, d->cXform);
		
		m = dd_makeMatrix(record->height, 0, 0, record->height, record->x, record->y);
		m = ddMatrix_multiply(xform, m);

		ddShapeInstance_drawInImage(record->shape, image, m, c, clipRect);
	}
}


void
dd_destroyDrawText(ddDrawable* d)
{
	int i;
	ddDrawText* t = (ddDrawText*)d;

	for ( i = 0; i < t->nRecords; ++i )
		dd_destroyShapeInstance(t->records[i].shape);

	dd_free(t->records);
	dd_free(d);
}


static void
ddDrawText_addGlyph(ddDrawText* d, ddFont* font, int glyphid,
					ddColor color, int height, int x, int y)
{
	ddShapeInstance* s = dd_newShapeInstance(ddFont_getGlyph(font, glyphid));

	if ( s == NULL )
		return;

	d->records = dd_realloc(d->records,
						  (d->nRecords + 1) * sizeof(struct _ddDrawTextRecord));

	ddShapeInstance_setGlyphColor(s, color);

	d->records[d->nRecords].shape = s;
	d->records[d->nRecords].height = height;
	d->records[d->nRecords].x = x;
	d->records[d->nRecords].y = y;

	++d->nRecords;
}


/* make a drawable instance of this text object */

ddDrawText*
dd_newDrawText(ddText* text)
{
	ddDrawText* d = dd_malloc(sizeof(ddDrawText));
	ddTextRecord* record = text->record;
	int i;

	ddDrawable_init((ddDrawable*)d);

	d->parent.type = TEXT_CHAR;
	d->parent.destroy = dd_destroyDrawText;
	d->parent.getUpdateList = ddDrawText_getUpdateList;
	d->parent.drawInImage = ddDrawText_drawInImage;
	d->parent.bounds = ddInvalidRect;

	d->records = NULL;
	d->nRecords = 0;

	while ( record != NULL )
	{
		ddColor color = record->color;
		int x = record->x;
		int y = record->y;
		int nGlyphs = record->nGlyphs;
		ddFont* font = record->font;
		int height = record->height;

		if ( font == NULL )
		{
			record = record->next;
			continue;
		}
		
		for ( i = 0; i < nGlyphs; ++i )
		{
			ddDrawText_addGlyph(d, font, record->glyphs[i], color, height, x, y);

			x += record->advances[i];
		}

		record = record->next;
	}

	return d;
}


int
ddDrawText_hitTest(ddDrawText* t, fixed x, fixed y)
{
	int i;

	for ( i = 0; i < t->nRecords; ++i )
	{
		if ( ddShapeInstance_hitTest(t->records[i].shape, x, y) )
			return DD_TRUE;
	}

	return DD_FALSE;
}
