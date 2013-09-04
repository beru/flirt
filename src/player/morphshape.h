/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_MORPHSHAPE_H_INCLUDED
#define DD_PLAYER_MORPHSHAPE_H_INCLUDED

typedef struct _ddMorphShape ddMorphShape;

#include "character.h"

struct _ddMorphShape
{
	ddCharacter parent;

	ddShape* shape1;
	ddShape* shape2;
};

ddMorphShape*
dd_newMorphShape(ddShape* shape1, ddShape* shape2);

#endif /* DD_PLAYER_MORPHSHAPE_H_INCLUDED */
