/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_CHARACTER_H_INCLUDED
#define DD_PLAYER_CHARACTER_H_INCLUDED

/* a character is a template for a display item. */

#include "../dd.h"

typedef struct _ddCharacter ddCharacter;
typedef struct _ddShapeCharacter ddShapeCharacter;
typedef struct _ddBitmapCharacter ddBitmapCharacter;
typedef struct _ddButtonCharacter ddButtonCharacter;
typedef struct _ddTextCharacter ddTextCharacter;
typedef struct _ddFontCharacter ddFontCharacter;
typedef struct _ddMovieClipCharacter ddMovieClipCharacter;
typedef struct _ddTextFieldCharacter ddTextFieldCharacter;
typedef struct _ddSoundCharacter ddSoundCharacter;

typedef enum
{
	SHAPE_CHAR,
	MORPH_CHAR,
	BITMAP_CHAR,
	BUTTON_CHAR,
	TEXT_CHAR,
	FONT_CHAR,
	MOVIECLIP_CHAR,
	TEXTFIELD_CHAR,
	SOUND_CHAR
} ddCharacterType;

#include "drawable.h"

struct _ddPlayer;

struct _ddCharacter
{
	ddCharacterType type;
	ddDrawable* (*instantiate)(struct _ddPlayer* player, ddCharacter* c);
	void (*destroy)(ddCharacter* c);
};


static inline void
dd_destroyCharacter(ddCharacter* c)
{
	if ( c != NULL && c->destroy != NULL )
		c->destroy(c);
}


static inline ddDrawable*
ddCharacter_instantiate(struct _ddPlayer* player, ddCharacter* c)
{
	if ( c != NULL && c->instantiate != NULL )
		return c->instantiate(player, c);
	else
		return NULL;
}


#include "../render/image.h"

struct _ddBitmapCharacter
{
	ddCharacter parent;
	ddImage* bitmap;
};


ddBitmapCharacter*
dd_newBitmapCharacter(ddImage* image);


#include "../render/shapeinstance.h"

struct _ddShapeCharacter
{
	ddCharacter parent;
	ddShape* shape;
};

ddShapeCharacter*
dd_newShapeCharacter(ddShape* shape);


#endif /* DD_PLAYER_CHARACTER_H_INCLUDED */
