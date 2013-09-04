/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_DRAWMORPH_H_INCLUDED
#define DD_PLAYER_DRAWMORPH_H_INCLUDED

typedef struct _ddDrawMorph ddDrawMorph;

#include "drawable.h"
#include "morphshape.h"
#include "drawshape.h"

struct _ddDrawMorph
{
	ddDrawable parent;
	
	ddMorphShape* morphShape;

	ddShape* shape;
	ddShapeInstance* instance;

	int ratio;
};


ddDrawMorph*
dd_newDrawMorph(ddMorphShape* m);

void
ddDrawMorph_setRatio(ddDrawMorph* morph, UInt16 ratio);

#endif /* DD_PLAYER_DRAWMORPH_H_INCLUDED */
