/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "textfield.h"
#include "drawtextfield.h"
#include "player.h"

static void
dd_destroyTextField(ddCharacter* c)
{
	ddTextField* text = (ddTextField*)c;

	if ( text->text != NULL )
		dd_free(text->text);

	if ( text->varname != NULL )
		dd_free(text->varname);

	dd_free(c);
}


static ddDrawable*
ddTextField_instantiate(ddPlayer* player, ddCharacter* c)
{
	ddTextField* text = (ddTextField*)c;

	return (ddDrawable*)dd_newDrawTextField(text);
}


ddTextField*
dd_newTextField()
{
	ddTextField* text = dd_malloc(sizeof(ddTextField));

	text->parent.type = TEXTFIELD_CHAR;
	text->parent.destroy = dd_destroyTextField;
	text->parent.instantiate = ddTextField_instantiate;

	text->bounds = ddInvalidRect;
	text->flags = 0;
	text->justify = TEXTFIELD_JUSTIFY_LEFT;
	text->color = (ddColor)0;
	text->font = NULL;
	text->height = 0;
	text->text = NULL;
	text->varname = NULL;
	text->leftMargin = 0;
	text->rightMargin = 0;
	text->lineSpacing = 0;
	text->indentation = 0;

	return text;
}
