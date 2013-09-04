/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_TEXTFIELD_H_INCLUDED
#define DD_PLAYER_TEXTFIELD_H_INCLUDED

#include "../dd.h"

typedef struct _ddTextField ddTextField;

#include "../render/rect.h"

#include "character.h"
#include "font.h"

#define TEXTFIELD_HASLENGTH (1<<1)
#define TEXTFIELD_NOEDIT    (1<<3)
#define TEXTFIELD_PASSWORD  (1<<4)
#define TEXTFIELD_MULTILINE (1<<5)
#define TEXTFIELD_WORDWRAP  (1<<6)
#define TEXTFIELD_DRAWBOX   (1<<11)
#define TEXTFIELD_NOSELECT  (1<<12)

typedef enum
{
	TEXTFIELD_JUSTIFY_LEFT    = 0,
	TEXTFIELD_JUSTIFY_RIGHT   = 1,
	TEXTFIELD_JUSTIFY_CENTER  = 2,
	TEXTFIELD_JUSTIFY_JUSTIFY = 3
} ddTextFieldJustification;

struct _ddTextField
{
	ddCharacter parent;

	ddRect bounds;
	int flags;
	ddTextFieldJustification justify;
	ddColor color;
	ddFont* font;
	fixed height;
	int length;
	char* text;
	char* varname;
	fixed leftMargin;
	fixed rightMargin;
	fixed lineSpacing;
	fixed indentation;
};


ddTextField*
dd_newTextField();

static inline void
ddTextField_setBounds(ddTextField* text, ddRect bounds)
{
	text->bounds = bounds;
}


static inline void
ddTextField_setLeftMargin(ddTextField* text, fixed leftMargin)
{
	text->leftMargin = leftMargin;
}


static inline void
ddTextField_setRightMargin(ddTextField* text, fixed rightMargin)
{
	text->rightMargin = rightMargin;
}


static inline void
ddTextField_setColor(ddTextField* text, ddColor color)
{
	text->color = color;
}

static inline void
ddTextField_setIndentation(ddTextField* text, fixed indentation)
{
	text->indentation = indentation;
}

static inline void
ddTextField_setText(ddTextField* text, char* initialText)
{
	text->text = initialText;
}

static inline void
ddTextField_setVariableName(ddTextField* text, char* varname)
{
	text->varname = varname;
}

static inline void
ddTextField_setLineSpacing(ddTextField* text, fixed lineSpacing)
{
	text->lineSpacing = lineSpacing;
}

static inline void
ddTextField_justify(ddTextField* text, ddTextFieldJustification justify)
{
	text->justify = justify;
}

static inline void
ddTextField_setFont(ddTextField* text, ddFont* font)
{
	text->font = font;
}

static inline void
ddTextField_setFlags(ddTextField* text, int flags)
{
	text->flags = flags;
}

static inline void
ddTextField_setHeight(ddTextField* text, fixed height)
{
	text->height = height;
}

static inline void
ddTextField_setLength(ddTextField* text, int length)
{
	text->length = length;
}

#endif /* DD_PLAYER_TEXTFIELD_H_INCLUDED */
