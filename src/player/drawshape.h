/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_DRAWSHAPE_H_INCLUDED
#define DD_PLAYER_DRAWSHAPE_H_INCLUDED

#include "../dd.h"

typedef struct _ddDrawShape ddDrawShape;

#include "drawable.h"
#include "../render/shapeinstance.h"

struct _ddDrawShape
{
	ddDrawable parent;
	ddShapeInstance* shape;
};

ddDrawShape*
dd_newDrawShape(ddShapeInstance* shape);

int
ddDrawShape_hitTest(ddDrawShape* s, fixed x, fixed y);

#endif /* DD_PLAYER_DRAWSHAPE_H_INCLUDED */
