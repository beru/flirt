/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>

#include "shapeinstance.h"

/* all displayed shapes are created here from character list shapes.
each gets its own copy of the fills so that it can cache fill matrix
inverses */

ddShapeInstance*
dd_newShapeInstance(ddShape* shape)
{
	int i;
	ddShapeInstance* s = dd_malloc(sizeof(ddShapeInstance));

	s->shape = shape;
	s->maskShape = NULL;
	s->bounds = ddInvalidRect; // shape->bounds;
	s->matrix = ddEmptyMatrix;
	s->edgelist = NULL;
	s->options = SHAPE_RENDER_AA; /* XXX */

	s->fills = dd_malloc(shape->nFills * sizeof(struct _ddFillInstance));
	s->nFills = shape->nFills;

	ddShape_end(s->shape); /* XXX - can we push this further up? */

	for ( i = 0; i < shape->nFills; ++i )
		s->fills[i] = dd_newFillInstance(shape->fills[i]);

	return s;
}


void
dd_destroyShapeInstance(ddShapeInstance* shape)
{
	int i;
	
	if ( shape->edgelist != NULL )
		dd_destroyEdgeList(shape->edgelist);
	
	for ( i = 0; i < shape->nFills; ++i )
		dd_destroyFillInstance(&shape->fills[i]);
	
	if ( shape->fills != NULL )
		dd_free(shape->fills);

	dd_free(shape);
}


void
ddShapeInstance_getUpdateList(ddShapeInstance* s, ddUpdateList* list, ddMatrix matrix)
{
	/* have to include moved-from space in update rect as well.. */
	ddRect oldbounds = s->bounds;
	
#ifdef DD_DEBUG
	dd_logShape(s->shape);
#endif

	// XXX - intersect bounds with mask here, or up in displaylist?

	// n.b.- drawbuttons pass null updatelists for their hit frames

	if ( ddShapeInstance_updateVertexList(s, matrix) && list != NULL )
	{
		ddUpdateList_includeRect(list, oldbounds);
		ddUpdateList_includeRect(list, s->bounds);
	}
}


ddRect
ddShapeInstance_getBounds(ddShapeInstance* s)
{
	return s->bounds;
}


void
ddShapeInstance_drawInImage(ddShapeInstance* s, ddImage* image,
							ddMatrix matrix, ddCXform cXform, ddRect clipRect)
{
	ddRect bounds;
	int y, endy;

	ddEdgeList* edgelist = s->edgelist;

	if ( s->maskShape != NULL )
	{
		ddShapeInstance_drawWithMaskInImage(s, image, matrix, cXform, clipRect, s->maskShape);
		return;
	}

	bounds = ddRect_intersectRect(s->bounds, clipRect);

	if ( INT_F(bounds.top) >= image->height || INT_F(bounds.bottom) < 0 ||
		 INT_F(bounds.left) >= image->width || INT_F(bounds.right) < 0 )
		return;

#ifdef DD_RENDER_TOPDOWN
	ddEdgeList_strokeEdges(edgelist, image, matrix, cXform, clipRect);
#endif
	
	y = INT_F(bounds.top);

	//ddEdgeList_advanceToScanline(edgelist, y, bounds.left, bounds.right);
	
	endy = dd_min(INT_F(bounds.bottom - 1), image->height - 1);

	while ( y <= endy )
	{
		ddEdgeList_renderScanline(edgelist, y, bounds.left, bounds.right, image, cXform, NULL);
		++y;
	}

#ifndef DD_RENDER_TOPDOWN
	ddEdgeList_strokeEdges(edgelist, image, matrix, cXform, clipRect);
#endif
	
	/* if shape runs off the bottom, we'll have leftovers here.. */

	ddEdgeList_reset(edgelist);
}


void
ddShapeInstance_drawWithMaskInImage(ddShapeInstance* s, ddImage* image,
									ddMatrix matrix, ddCXform cXform,
									ddRect clipRect, ddShapeInstance* mask)
{
	int y, endy;

	ddEdgeList* edgelist = s->edgelist;
	ddEdgeList* masklist = mask->edgelist;

	ddRect bounds = ddRect_intersectRect(s->bounds, clipRect);

	if ( !ddRect_intersectsRect(bounds, mask->bounds) ||
		 INT_F(bounds.top) >= image->height || INT_F(bounds.bottom) < 0 ||
		 INT_F(bounds.left) >= image->width || INT_F(bounds.right) < 0 )
		return;
	
#ifdef DD_RENDER_TOPDOWN
	ddEdgeList_strokeEdges(edgelist, image, matrix, cXform, clipRect);
#endif	

	y = INT_F(bounds.top);

	ddEdgeList_advanceToScanline(edgelist, y, bounds.left, bounds.right);

	endy = dd_min(INT_F(bounds.bottom), image->height - 1);

	while ( y <= endy )
	{
		ddEdgeList_renderMaskedScanline(edgelist, masklist,
								  y, bounds.left, bounds.right, image, cXform);
		++y;
	}

#ifndef DD_RENDER_TOPDOWN
	ddEdgeList_strokeEdges(edgelist, image, matrix, cXform, clipRect);
#endif
	
	/* if shape runs off the bottom, we'll have leftovers here.. */

	ddEdgeList_reset(edgelist);
	ddEdgeList_reset(masklist);
}


void
ddShapeInstance_setMaskShape(ddShapeInstance* s, ddShapeInstance* mask)
{
	// XXX - force redraw?
	s->maskShape = mask;
}


int
ddShapeInstance_hitTest(ddShapeInstance* s, fixed x, fixed y)
{
	if ( s->maskShape != NULL && ddShapeInstance_hitTest(s->maskShape, x, y) == 0 )
		return 0;
	
	if ( !ddRect_containsPoint(s->bounds, x, y) )
		return 0;
	
	return ddEdgeList_hitTest(s->edgelist, x, y);
}


void
ddFillInstance_recalcInverse(ddFillInstance* fill, ddMatrix matrix)
{
	if ( fill->fill.type == FILL_SOLID )
		return;

	if ( fill->fill.type == FILL_CLIPPED_BITMAP || fill->fill.type == FILL_TILED_BITMAP )
	{
		fill->inverse = ddMatrix_multiply(matrix, fill->fill.data.bitmap.matrix);
		fill->inverse = ddMatrix_invert(fill->inverse);
	}
	else if ( fill->fill.type == FILL_RADIAL_GRADIENT || fill->fill.type == FILL_LINEAR_GRADIENT )
	{
		fill->inverse = ddMatrix_multiply(matrix, fill->fill.data.gradient.matrix);
		fill->inverse = ddMatrix_invert(fill->inverse);
	}
}


void
ddShapeInstance_recalcFillInverses(ddShapeInstance* s, ddMatrix matrix)
{
	int i;

	for ( i = 0; i < s->nFills; ++i )
		ddFillInstance_recalcInverse(ddShapeInstance_getFill(s, i), matrix);
}


/* matrix handed in from display list returns nonzero if needs redrawing */

int
ddShapeInstance_updateVertexList(ddShapeInstance* s, ddMatrix matrix)
{
	/* check matrix diff to see how much work we need to do.. */

	ddMatrixState state = ddMatrix_compare(&s->matrix, &matrix);

	/* scaled curves need to be reinterpolated.. */

	if ( s->shape->hasCurve && state != MATRIX_UPTODATE && state != MATRIX_MOVED )
		state = MATRIX_MUNGED;

	// XXX - optimizations not yet done..
	if ( state == MATRIX_STRETCHED || state == MATRIX_SCALED /*|| state == MATRIX_MOVED*/ )
		state = MATRIX_MUNGED;

	switch ( state )
	{
		case MATRIX_UPTODATE:
			return 0;

		case MATRIX_MUNGED:
			ddShapeInstance_recalcFillInverses(s, matrix);

			if ( s->edgelist != NULL )
				dd_destroyEdgeList(s->edgelist);

			// XXX - would be good to postpone this to draw time
			// in case the character's offscreen:
			
			s->edgelist = dd_newEdgeList(s, matrix);

			break;

		case MATRIX_STRETCHED:
			ddShapeInstance_recalcFillInverses(s, matrix);
			ddEdgeList_recalcSlopes(s->edgelist, matrix, &s->bounds);
			break;

		case MATRIX_SCALED:
			ddShapeInstance_recalcFillInverses(s, matrix);
			ddEdgeList_recalcCoords(s->edgelist, matrix, &s->bounds);
			break;

		case MATRIX_MOVED:
			ddShapeInstance_recalcFillInverses(s, matrix);
			ddEdgeList_displace(s->edgelist, matrix.dx - s->matrix.dx,
						 matrix.dy - s->matrix.dy, &s->bounds);
			break;

		default:
			dd_warn("Unknown shape state %i!", state);
			return 0;
	}

	s->matrix = matrix;
	return 1;
}


/* assume this is a glyph, and set the fill color to the glyph color */

void
ddShapeInstance_setGlyphColor(ddShapeInstance* s, ddColor color)
{
	ddFill fill = { FILL_SOLID, { color } };
	
	if ( s->nFills == 0 )
	{
		s->fills = dd_realloc(s->fills, sizeof(struct _ddFillInstance));
		s->nFills = 1;
	}
	else
		dd_warn("glyph shape instance already has fill!");

	s->fills[0] = dd_newFillInstance(fill);
}
