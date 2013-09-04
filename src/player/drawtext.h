/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_DRAWTEXT_H_INCLUDED
#define DD_PLAYER_DRAWTEXT_H_INCLUDED

#include "../dd.h"

typedef struct _ddDrawText ddDrawText;
typedef struct _ddDrawTextRecord ddDrawTextRecord;

#include "drawable.h"
#include "font.h"
#include "text.h"

#include "../render/shapeinstance.h"

/* drawtext is a big list of shapes */

struct _ddDrawTextRecord
{
	ddShapeInstance* shape;
	int height;
	int x;
	int y;
};

struct _ddDrawText
{
	ddDrawable parent;
	
	int nRecords;
	struct _ddDrawTextRecord* records;
};

ddDrawText*
dd_newDrawText(ddText* text);

int
ddDrawText_hitTest(ddDrawText* t, fixed x, fixed y);

#endif /* DD_PLAYER_DRAWTEXT_H_INCLUDED */
