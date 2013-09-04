/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "morphshape.h"
#include "drawmorph.h"
#include "player.h"

static ddDrawable*
ddMorphShape_instantiate(ddPlayer* player, ddCharacter* c)
{
	ddDrawMorph* morph = dd_newDrawMorph((ddMorphShape*)c);
	ddDrawMorph_setRatio(morph, 0);
	return (ddDrawable*)morph;
}


static void
dd_destroyMorphShape(ddCharacter* c)
{
	ddMorphShape* morph = (ddMorphShape*)c;

	dd_destroyShape(morph->shape1);
	dd_destroyShape(morph->shape2);

	dd_free(c);
}


ddMorphShape*
dd_newMorphShape(ddShape* shape1, ddShape* shape2)
{
	ddMorphShape* morph = dd_malloc(sizeof(ddMorphShape));

	morph->parent.type = MORPH_CHAR;
	morph->parent.instantiate = ddMorphShape_instantiate;
	morph->parent.destroy = dd_destroyMorphShape;

	morph->shape1 = shape1;
	morph->shape2 = shape2;

	return morph;
}
