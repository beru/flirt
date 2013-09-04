/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>

#include "drawtextfield.h"


static void
rebuildShapeList(ddDrawTextField* field)
{
	char* str = field->text;
	ddFont* font = field->textfield->font;
	ddColor color = field->textfield->color;
	int len = (str == NULL) ? 0 : strlen(str);
	int glyphCount = 0;
	int i;
	fixed x = 0;

	for ( i = 0; i < field->nGlyphs; ++i )
		dd_destroyShapeInstance(field->glyphs[i]);

	// XXX - add justification code
	
	if ( len > 0 )
	{
		field->glyphs = dd_realloc(field->glyphs, len * sizeof(ddShape*));
		field->advances = dd_realloc(field->advances, len * sizeof(fixed));
	}
	
	for ( i = 0; i < len; ++i )
	{
		int glyphid = ddFont_getGlyphCode(font, str[i]);
		ddShape* glyph;
		ddShapeInstance* s;

		if ( glyphid == -1 )
			continue;

		glyph = ddFont_getGlyph(font, glyphid);

		if ( glyph == NULL )
			continue;

		s = dd_newShapeInstance(glyph);
		ddShapeInstance_setGlyphColor(s, color);

		field->glyphs[glyphCount] = s;
		field->advances[glyphCount] = x;

		++glyphCount;

		x += ddFont_getGlyphAdvance(font, glyphid);
	}

	field->nGlyphs = glyphCount;
	field->changed = DD_FALSE;
}


void
ddDrawTextField_getUpdateList(ddDrawable* d, ddUpdateList* list, ddMatrix matrix)
{
	ddMatrix m;
	ddMatrix xform;
	int i;
	
	ddDrawTextField* text = (ddDrawTextField*)d;
	fixed height = text->textfield->height;

	d->bounds = ddInvalidRect;
	
	if ( text->variableName != NULL )
	{
		ddActionValue val = ddActionObject_getProperty(NULL, (ddActionObject*)text->parentClip->actionClip, text->variableName);
		
		if ( val.type == VALUE_STRING )
			ddDrawTextField_setText(text, val.data.stringValue);
		
		ddActionValue_release(val);
	}
	
	// XXX - avoid redrawing unchanged characters
	
	if ( text->changed )
		rebuildShapeList(text);

	xform = ddMatrix_multiply(matrix, d->matrix);

	for ( i = 0; i < text->nGlyphs; ++i )
	{
		// XXX - I have no idea what 38 is, but this makes it work..
		m = dd_makeMatrix(height, 0, 0, height, ((text->advances[i] * height) >> 10),  38 * height);
		m = ddMatrix_multiply(xform, m);

		ddShapeInstance_getUpdateList(text->glyphs[i], list, m);
		d->bounds = ddRect_containRect(d->bounds, text->glyphs[i]->bounds);
	}
}


void
ddDrawTextField_drawInImage(ddDrawable* d, ddImage* image, ddUpdateList* list,
							ddMatrix matrix, ddCXform cXform, ddRect clipRect)
{
	ddMatrix m;

	int i;
	ddDrawTextField* text = (ddDrawTextField*)d;
	fixed height = text->textfield->height;

	ddMatrix xform = ddMatrix_multiply(matrix, d->matrix);

	for ( i = 0; i < text->nGlyphs; ++i )
	{
		ddCXform c = ddCXform_compose(cXform, d->cXform);

		// XXX - I have no idea what 38 is, but this makes it work..
		m = dd_makeMatrix(height, 0, 0, height, ((text->advances[i] * height) >> 10), 38 * height);
		m = ddMatrix_multiply(xform, m);

		ddShapeInstance_drawInImage(text->glyphs[i], image, m, c, clipRect);
	}	
}


void
dd_destroyDrawTextField(ddDrawable* d)
{
	int i;
	ddDrawTextField* text = (ddDrawTextField*)d;

	if ( text->parentClip != NULL )
		ddActionMovieClip_removeTextField(text->parentClip->actionClip, text);

	if ( text->text != NULL )
		dd_free(text->text);

	for ( i = 0; i < text->nGlyphs; ++i )
		dd_destroyShapeInstance(text->glyphs[i]);

	dd_free(text->glyphs);
	dd_free(text->advances);
	
	if ( text->variableName != NULL )
		dd_free(text->variableName);

	dd_free(d);
}


ddDrawTextField*
dd_newDrawTextField(ddTextField* text)
{
	ddDrawTextField* drawfield = dd_malloc(sizeof(ddDrawTextField));
	ddDrawable_init((ddDrawable*)drawfield);

	drawfield->parent.type = TEXTFIELD_CHAR;
	drawfield->parent.getUpdateList = ddDrawTextField_getUpdateList;
	drawfield->parent.drawInImage = ddDrawTextField_drawInImage;
	drawfield->parent.destroy = dd_destroyDrawTextField;

	drawfield->textfield = text;
	drawfield->parentClip = NULL;
	
	if ( text->text != NULL )
		drawfield->text = strdup(text->text);
	else
		drawfield->text = strdup("");

	drawfield->selection = 0;
	drawfield->changed = DD_TRUE;

	drawfield->nGlyphs = 0;
	drawfield->glyphs = NULL;
	drawfield->advances = NULL;
	drawfield->variableName = NULL;
	
	return drawfield;
}


void
ddDrawTextField_setText(ddDrawTextField* field, char* text)
{
	if ( field->text != NULL )
		dd_free(field->text);

	field->text = text;
	field->changed = DD_TRUE;
}


const char*
ddDrawTextField_getText(ddDrawTextField* field)
{
	return field->text;
}


void
ddDrawTextField_setVariableName(ddDrawTextField* field, const char* varname)
{
	if ( field->variableName != NULL )
		dd_free(field->variableName);
	
	field->variableName = strdup(varname);
}


void
ddDrawTextField_setParentClip(ddDrawTextField* field, ddDrawClip* parentClip)
{
	field->parentClip = parentClip;
}
