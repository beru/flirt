/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_RENDER_SHAPEINSTANCE_H_INCLUDED
#define DD_RENDER_SHAPEINSTANCE_H_INCLUDED

#include "../dd.h"

typedef struct _ddShapeInstance ddShapeInstance;

#include "shape.h"
#include "matrix.h"
#include "fill.h"
#include "edgelist.h"
#include "updatelist.h"
#include "cxform.h"


typedef enum
{
	SHAPE_RENDER_AA = (1 << 0)
} ddShapeOption;

struct _ddShapeInstance
{
	/* the shape this in an instance of */
	ddShape* shape;

	ddRect bounds;

	struct _ddShapeInstance* maskShape;

	/* each shape instance needs its own copies of the shape fills */
	ddFillInstance* fills;
	int nFills;

	/* the current matrix- saved so that we can avoid rebuilding vlist */
	ddMatrix matrix;

	/* edgelist used for rasterizing */
	ddEdgeList* edgelist;

	ddShapeOption options;
};

ddShapeInstance*
dd_newShapeInstance(ddShape* shape);

void
dd_destroyShapeInstance(ddShapeInstance* i);

ddRect
ddShapeInstance_getBounds(ddShapeInstance* s);

int
ddShapeInstance_updateVertexList(ddShapeInstance* s, ddMatrix matrix);

void
ddShapeInstance_getUpdateList(ddShapeInstance* s,
							  ddUpdateList* list, ddMatrix matrix);

void
ddShapeInstance_drawInImage(ddShapeInstance* s, ddImage* image, ddMatrix matrix, ddCXform cXform, ddRect clipRect);

void
ddShapeInstance_drawWithMaskInImage(ddShapeInstance* s, ddImage* image,
									ddMatrix matrix, ddCXform cXform, ddRect clipRect, ddShapeInstance* mask);

void
ddShapeInstance_setMaskShape(ddShapeInstance* s, ddShapeInstance* mask);

int
ddShapeInstance_hitTest(ddShapeInstance* s, fixed x, fixed y);

ddRect
ddShapeInstance_getBounds(ddShapeInstance* s);

static inline ddFillInstance*
ddShapeInstance_getFill(ddShapeInstance* s, int idx)
{
	if ( idx == -1 )
		return NULL;
	else
		return &(s->fills[idx]);
}

void
ddShapeInstance_setGlyphColor(ddShapeInstance* s, ddColor color);

#endif /* DD_RENDER_SHAPEINSTANCE_H_INCLUDED */
