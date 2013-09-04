/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_DRAWTEXTFIELD_H_INCLUDED
#define DD_PLAYER_DRAWTEXTFIELD_H_INCLUDED

#include "../dd.h"

typedef struct _ddDrawTextField ddDrawTextField;

#include "drawable.h"
#include "font.h"
#include "drawtext.h"
#include "textfield.h"
#include "drawclip.h"

struct _ddDrawTextField
{
	ddDrawable parent;

	ddTextField* textfield;
	ddDrawClip* parentClip;
	
	char* variableName;
	
	int nGlyphs;
	ddShapeInstance** glyphs;
	fixed* advances;

	char* text;
	int selection;
	boolean changed;
};

ddDrawTextField*
dd_newDrawTextField(ddTextField* text);

void
ddDrawTextField_setText(ddDrawTextField* field, char* text);

const char*
ddDrawTextField_getText(ddDrawTextField* field);

void
ddDrawTextField_setVariableName(ddDrawTextField* field, const char* varname);

void
ddDrawTextField_setParentClip(ddDrawTextField* field, ddDrawClip* parent);

#endif /* DD_PLAYER_DRAWTEXTFIELD_H_INCLUDED */
