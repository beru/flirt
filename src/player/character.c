/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "character.h"
#include "movieclip.h"

static void
dd_destroyBitmapCharacter(ddCharacter* c)
{
	dd_destroyImage(((ddBitmapCharacter*)c)->bitmap);
	dd_free(c);
}


ddBitmapCharacter*
dd_newBitmapCharacter(ddImage* bitmap)
{
	ddBitmapCharacter* b = dd_malloc(sizeof(ddBitmapCharacter));
	b->parent.type = BITMAP_CHAR;
	b->parent.destroy = dd_destroyBitmapCharacter;
	b->parent.instantiate = NULL;
	b->bitmap = bitmap;
	return b;
}


#include "drawshape.h"

static void
dd_destroyShapeCharacter(ddCharacter* c)
{
	dd_destroyShape(((ddShapeCharacter*)c)->shape);
	dd_free(c);
}


static ddDrawable*
ddShapeCharacter_instantiate(ddPlayer* player, ddCharacter* c)
{
	ddShapeCharacter* s = (ddShapeCharacter*)c;
	return (ddDrawable*)dd_newDrawShape(dd_newShapeInstance(s->shape));
}


ddShapeCharacter*
dd_newShapeCharacter(ddShape* shape)
{
	ddShapeCharacter* s = dd_malloc(sizeof(ddShapeCharacter));
	s->parent.type = SHAPE_CHAR;
	s->parent.destroy = dd_destroyShapeCharacter;
	s->parent.instantiate = ddShapeCharacter_instantiate;
	s->shape = shape;
	return s;
}


#ifdef DD_INCLUDE_DEBUGGER

ddCharacterType
ddCharacter_getType(ddCharacter* character)
{
	return character->type;
}

#endif
