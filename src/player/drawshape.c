/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "drawshape.h"

#include "../render/image.h"
#include "../render/matrix.h"

static void
ddDrawShape_getUpdateList(ddDrawable* d, ddUpdateList* list, ddMatrix matrix)
{
	ddDrawShape* s = (ddDrawShape*)d;

	// XXX - include mask shape?

	ddMatrix m = ddMatrix_multiply(matrix, d->matrix);
	ddShapeInstance_getUpdateList(s->shape, list, m);
	d->bounds = s->shape->bounds;
}


static void
ddDrawShape_drawInImage(ddDrawable* d, ddImage* image, ddUpdateList* list,
						ddMatrix matrix, ddCXform cXform, ddRect clipRect)
{
	ddDrawShape* s = (ddDrawShape*)d;

	//if ( ddUpdateList_intersectsRect(list, d->bounds) )
	//{
		ddMatrix m = ddMatrix_multiply(matrix, d->matrix);
		ddCXform c = ddCXform_compose(cXform, d->cXform);
		ddShapeInstance_drawInImage(s->shape, image, m, c, clipRect);
	//}
}


static void
dd_destroyDrawShape(ddDrawable* d)
{
	dd_destroyShapeInstance(((ddDrawShape*)d)->shape);
	dd_free(d);
}


ddDrawShape*
dd_newDrawShape(ddShapeInstance* shape)
{
	ddDrawShape* s = dd_malloc(sizeof(ddDrawShape));
	ddDrawable_init((ddDrawable*)s);

	s->parent.type = SHAPE_CHAR;
	s->parent.getUpdateList = ddDrawShape_getUpdateList;
	s->parent.drawInImage = ddDrawShape_drawInImage;
	s->parent.destroy = dd_destroyDrawShape;
	s->parent.bounds = ddInvalidRect; //ddShapeInstance_getBounds(shape);
	s->shape = shape;

	return s;
}


int
ddDrawShape_hitTest(ddDrawShape* s, fixed x, fixed y)
{
	if ( s->shape != NULL )
		return ddShapeInstance_hitTest(s->shape, x, y);
	else
		return DD_FALSE;
}
