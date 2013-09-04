/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <stdio.h>
#include <string.h>

#include "edgelist.h"

struct _ddEdge
{
	/* endpoints for this edge- starty < endy */
	fixed startx;
	fixed starty;
	fixed endx;
	fixed endy;

	fixed dx;		// change in x each scan line, -1/slope
	fixed dy;		// change in y each pixel, aka slope

	fixed initx;	// initial x and y values,
	fixed inity;	// projected back to pixel boundaries

	fixed x;		// registers for rasterizing- x+=dx each scan line,
	fixed y;		// y is fractional value at last pixel crossing
					// x is initially set for sorting
	
	ddColor lineColor;
	UInt16 lineWidth;
	boolean flipped; // stroke method wants to know actual drawing order, but we've
					 // already flipped the line if starty > endy--so note it here
	
	ddRasterMethod scan;
	ddFillInstance* leftfill;
	ddFillInstance* rightfill;
	
	//int replace; // edge (index) to replace; -1 is none

	// for linking into an list of active edges for the current scan line
	struct _ddEdge* next;
	struct _ddEdge* last;
};


static void
ddEdge_setInitXY(ddEdge* edge);


void
dd_destroyEdgeList(ddEdgeList* list)
{
#ifdef DD_DEBUG
	printf("%p -> destroy edgeList\n", list);
#endif
	
	if ( list->next != NULL )
		dd_destroyEdgeList(list->next);

	if ( list->edges != NULL )
		dd_free(list->edges);
	
	if ( list->sortedEdges != NULL )
		dd_free(list->sortedEdges);
	
	dd_free(list);
}


static int
_Edge_compare(const void* a, const void* b)
{
	const ddEdge* va = *(ddEdge**)a;
	const ddEdge* vb = *(ddEdge**)b;

	// first sort by scan line

	if ( va->starty < vb->starty )
		return -1;
	else if ( va->starty > vb->starty )
		return 1;
	else if ( va->startx < vb->startx )
		return -1;
	else if ( va->startx > vb->startx )
		return 1;
	else if ( va->dx < vb->dx )
		return -1;
	else if ( va->dx > vb->dx )
		return 1;
	else
		return 0;
}


static void inline
ddEdgeList_order(ddEdgeList* list, int nVerts)
{
	int i;
	
	if ( list->sortedEdges == NULL )
		list->sortedEdges = dd_malloc(sizeof(ddEdge*) * list->nEdges);
	
	for ( i = 0; i < list->nEdges; ++i )
		list->sortedEdges[i] = &list->edges[i];
	
	qsort(list->sortedEdges, nVerts, sizeof(ddEdge*), _Edge_compare);
}


struct builderState
{
	ddEdge* list;
	int nEdges;
	ddShapeInstance* shape;
	fixed x;
	fixed y;
	boolean swapFills;
	ddRasterMethod leftScan;
	ddFillInstance* leftFill;
	ddRasterMethod rightScan;
	ddFillInstance* rightFill;
	fixed lineWidth;
	ddColor lineColor;
	ddRect bounds;
	int aa;
	int dir;
};


enum edgeDir
{
	EDGE_UP = -1,
	EDGE_DOWN = 1
};

static void
dd_initBuilderState(struct builderState *b, ddShapeInstance* s, ddMatrix m)
{
	b->list = NULL;
	b->nEdges = 0;
	b->shape = s;
	b->aa = s->options & SHAPE_RENDER_AA;
	b->swapFills = DD_FALSE;
	b->leftScan = NULL;
	b->rightScan = NULL;
	b->leftFill = NULL;
	b->rightFill = NULL;
	b->x = m.dx;
	b->y = m.dy;
	b->lineWidth = 0;
	b->lineColor = 0;
	b->bounds = ddInvalidRect;
	s->bounds = ddInvalidRect;
	b->dir = EDGE_UP;
}


static void
ddEdgeList_addEdge(struct builderState *b,
				   fixed startx, fixed starty, fixed endx, fixed endy)
{
	ddEdge* edge;

	b->bounds = ddRect_containPoint(b->bounds, startx - b->lineWidth, starty - b->lineWidth);
	b->bounds = ddRect_containPoint(b->bounds, startx + b->lineWidth, starty + b->lineWidth);

	// XXX - one of these should be redundant, but sometimes there's a double moveto
	b->bounds = ddRect_containPoint(b->bounds, endx - b->lineWidth, endy - b->lineWidth);
	b->bounds =	ddRect_containPoint(b->bounds, endx + b->lineWidth, endy + b->lineWidth);
	
	b->list = dd_realloc(b->list, sizeof(struct _ddEdge) * (b->nEdges + 1));

	edge = &(b->list[b->nEdges]);

	edge->last = NULL;
	edge->next = NULL;
	//edge->replace = -1;
	
	if ( endy == starty )
		++endy;
	
	if ( endy > starty )
	{
		edge->startx = startx;
		edge->starty = starty;
		edge->endx = endx;
		edge->endy = endy;
		
		if ( b->swapFills )
		{
			edge->leftfill = b->leftFill;
			edge->rightfill = b->rightFill;
			edge->scan = b->rightScan;
		}
		else
		{
			edge->leftfill = b->rightFill;
			edge->rightfill = b->leftFill;
			edge->scan = b->leftScan;
		}

		edge->flipped = DD_FALSE;
		
		//if ( b->dir == EDGE_DOWN && b->nEdges > 0 )
		//	edge->replace = b->nEdges - 1;
		
		b->dir = EDGE_DOWN;
	}
	else
	{
		edge->startx = endx;
		edge->starty = endy;
		edge->endx = startx;
		edge->endy = starty;
		
		if ( b->swapFills )
		{
			edge->rightfill = b->leftFill;
			edge->leftfill = b->rightFill;
			edge->scan = b->leftScan;
		}
		else
		{
			edge->rightfill = b->rightFill;
			edge->leftfill = b->leftFill;
			edge->scan = b->rightScan;
		}

		edge->flipped = DD_TRUE;

		//if ( b->dir == EDGE_UP && b->nEdges > 0 )
		//	(edge-1)->replace = b->nEdges;

		b->dir = EDGE_UP;
	}

	if ( edge->endy > edge->starty )
		edge->dx = fixed_divide(edge->endx - edge->startx, edge->endy - edge->starty);
	else
		edge->dx = 0;

	if ( edge->startx != edge->endx )
		edge->dy = fixed_divide(edge->endy - edge->starty, abs(edge->endx - edge->startx));
	else
		edge->dy = 0;
	
	ddEdge_setInitXY(edge);
	
	edge->y = 0;

	edge->last = NULL;
	edge->next = NULL;

	edge->lineWidth = b->lineWidth;
	edge->lineColor = b->lineColor;
	
	++b->nEdges;
}


static void
ddEdge_setInitXY(ddEdge* edge)
{
	// project x, y back to pixel boundary crossings

	edge->initx = edge->startx - fixed_mult(edge->dx, FRACT_F(edge->starty));

	if ( edge->dx < 0 )
		edge->inity = FRACT_F(edge->starty) - fixed_mult(edge->dy, FIXED_I(1) - FRACT_F(edge->startx));
	else
		edge->inity = FRACT_F(edge->starty) - fixed_mult(edge->dy, FRACT_F(edge->startx));
}


static void
ddEdgeList_addEdgeTo(struct builderState *b, fixed x, fixed y)
{
	ddEdgeList_addEdge(b, b->x, b->y, x, y);
	b->x = x;
	b->y = y;
}


static void
ddEdgeList_moveTo(struct builderState *b, fixed x, fixed y)
{
	b->x = x;
	b->y = y;
}

/* XXX */
#define CURVE_LIMIT FIXED_I(1)

static void
ddEdgeList_addCurveSegment(struct builderState *b,
						   fixed x1, fixed y1, fixed x2, fixed y2,
						   fixed x3, fixed y3)
{
	// would be nice to have something more like "flatness" that doesn't scale with curve length
	//fixed d = abs(fixed_mult(x3-x1, x2-x1) + fixed_mult(y3-y1, y2-y1));
	
	//d = fixed_divide(d, abs(x2-x1) + abs(y2-y1) + abs(x3-x2) + abs(y3-y2));

	fixed d = abs(x2-x1) + abs(y2-y1) + abs(x3-x2) + abs(y3-y2);
	
	if ( d > CURVE_LIMIT )
	{
		// use subdivision method to get a better approximation
		// XXX - is there a better division than t=1/2?
		
		fixed xa = (x1+x2) / 2;
		fixed ya = (y1+y2) / 2;
		fixed xb = (x2+x3) / 2;
		fixed yb = (y2+y3) / 2;
		fixed xc = (xa+xb) / 2;
		fixed yc = (ya+yb) / 2;

		ddEdgeList_addCurveSegment(b, x1, y1, xa, ya, xc, yc);
		ddEdgeList_addCurveSegment(b, xc, yc, xb, yb, x3, y3);
	}
	else
		ddEdgeList_addEdgeTo(b, x3, y3);
}


void
ddEdgeList_closePath(ddEdgeList* list, struct builderState *b)
{
	int i;
	
	list->bounds = b->bounds;
	list->nEdges = b->nEdges;
	list->edges = b->list;
	
	ddEdgeList_order(list, list->nEdges);
	
	// first edge determines if left/right fill is swapped
	
	for ( i = 0; i < list->nEdges; ++i )
	{
		ddEdge* edge = list->sortedEdges[i];
		
		if ( edge->leftfill != NULL && edge->rightfill != NULL )
			dd_log("first edge has both fills!");
		
		if ( edge->leftfill != NULL )
		{
			list->swapFills = DD_TRUE;
			break;
		}

		if ( edge->rightfill != NULL )
		{
			list->swapFills = DD_FALSE;
			break;
		}
	}
	
	b->bounds = ddInvalidRect;
	b->nEdges = 0;
	b->list = NULL;
}


ddEdgeList*
dd_newEdgeList_empty()
{
	ddEdgeList* list = dd_malloc(sizeof(ddEdgeList));
	
	list->next = NULL;
	list->bounds = ddInvalidRect;
	list->edges = NULL;
	list->sortedEdges = NULL;
	list->nEdges = 0;
	list->index = 0;
	list->start = NULL;
	list->swapFills = DD_FALSE;
	
	return list;
}


/* turn the shape's records into a (sorted) vertexlist */

ddEdgeList*
dd_newEdgeList(ddShapeInstance* s, ddMatrix matrix)
{
	ddEdgeList* retlist;
	ddEdgeList* list;
	ddShape* shape = s->shape;
	ddShapeRecord* record;
	ddShapeRecordType type;
	ddRect bounds = ddInvalidRect;
	struct builderState b;
	int i;

	retlist = dd_newEdgeList_empty();

	list = retlist;

	dd_initBuilderState(&b, s, matrix);

	if ( ddMatrix_isFlipped(matrix) )
		b.swapFills = DD_TRUE;

	for ( i = 0; ; ++i )
	{
		record = &(shape->records[i]);
		type = record->type;

		if ( type == SHAPERECORD_END )
			break;

		switch ( type )
		{
			case SHAPERECORD_END:
				break;

			case SHAPERECORD_NEWPATH:
			{
				ddEdgeList_closePath(list, &b);
				
				bounds = ddRect_containRect(bounds, list->bounds);
				
				list->next = dd_newEdgeList_empty();
				list = list->next;
				
				break;
			}
			
			case SHAPERECORD_MOVETO:
			{
				fixed x = record->data.coords.x1;
				fixed y = record->data.coords.y1;

				ddMatrix_apply(matrix, &x, &y);
				ddEdgeList_moveTo(&b, x, y);

				break;
			}

			case SHAPERECORD_LINETO:
			{
				fixed x = record->data.coords.x1;
				fixed y = record->data.coords.y1;

				ddMatrix_apply(matrix, &x, &y);
				ddEdgeList_addEdgeTo(&b, x, y);

				break;
			}

			case SHAPERECORD_CURVETO:
			{
				fixed x1 = record->data.coords.x1;
				fixed y1 = record->data.coords.y1;
				fixed x2 = record->data.coords.x2;
				fixed y2 = record->data.coords.y2;

				ddMatrix_apply(matrix, &x1, &y1);
				ddMatrix_apply(matrix, &x2, &y2);
				ddEdgeList_addCurveSegment(&b, b.x, b.y, x1, y1, x2, y2);

				break;
			}

			case SHAPERECORD_NEWFILL0:
			{
				b.leftFill = ddShapeInstance_getFill(s, record->data.fill);
				b.leftScan = ddFillInstance_getRasterMethod(b.leftFill);
				break;
			}

			case SHAPERECORD_NEWFILL1:
			{
				b.rightFill = ddShapeInstance_getFill(s, record->data.fill);
				b.rightScan = ddFillInstance_getRasterMethod(b.rightFill);
				break;
			}

			case SHAPERECORD_NEWLINE:
			{
				b.lineWidth = record->data.line.width;
				b.lineColor = record->data.line.color;
				break;
			}

			default:
				dd_error("Unexpected vertex type: %i!", type);
		}
	}

#ifdef DD_DEBUG
	printf("%p -> new edgeList\n", b.list);
#endif

	ddEdgeList_closePath(list, &b);

	s->bounds = ddRect_containRect(bounds, list->bounds);

	return retlist;
}


void
ddEdgeList_recalcSlopes(ddEdgeList* list, ddMatrix matrix, ddRect* bounds)
{
	/*
	 shape has been scaled non-proportionally but not rotated,
	 so edgelist doesn't need to be resorted
	*/

	/* XXX */
	
	if ( list->next != NULL )
		ddEdgeList_recalcSlopes(list->next, matrix, bounds);
}


void
ddEdgeList_recalcCoords(ddEdgeList* list, ddMatrix matrix, ddRect* bounds)
{
	int i;
	
	/* coords have been scaled proportionally, so slopes are the same */

	*bounds = ddInvalidRect;

	for ( i = 0; i < list->nEdges; ++i )
	{
		/* XXX - this isn't right, vert might be off by one..
		but this is the idea, anyway. */

		/*
		 vlist->x = vlist->vert->x1;
		 vlist->y = vlist->vert->y1;
		 ddMatrix_apply(matrix, &vlist->x, &vlist->y);
		 bounds = ddRect_containPoint(bounds, vlist->x, vlist->y);
		 */

		/* XXX - also fix bounds */
	}
	
	if ( list->next != NULL )
		ddEdgeList_recalcCoords(list->next, matrix, bounds);
}


void
ddEdgeList_displaceEach(ddEdgeList* list, fixed dx, fixed dy)
{
	int i;
	
	for ( i = 0; i < list->nEdges; ++i )
	{
		list->edges[i].startx += dx;
		list->edges[i].starty += dy;
		list->edges[i].endx += dx;
		list->edges[i].endy += dy;
		
		ddEdge_setInitXY(&list->edges[i]);
	}
	
	list->bounds = ddRect_displace(list->bounds, dx, dy);

	if ( list->next != NULL )
		ddEdgeList_displaceEach(list->next, dx, dy);
}

void
ddEdgeList_displace(ddEdgeList* list, fixed dx, fixed dy, ddRect* bounds)
{
	ddEdgeList_displaceEach(list, dx, dy);
	*bounds = ddRect_displace(*bounds, dx, dy);
}


/* vertexlist rendering stuff */

// o left crosses right if left->x + left->dx > right->x + right->dx
// o we should put lines up one y if it's a ^
// o if it's a next or last, scale output val by ending y value,
//   then let next/last edge run the same scan

static void
ddEdge_linkBefore(ddEdge* edge, ddEdge* next)
{
	if ( next == NULL )
		return;
	
	if ( next == edge )
		dd_log("next == edge in ddEdge_linkBefore");
	
	if ( next->last != NULL )
		next->last->next = edge;

	edge->last = next->last;
	edge->next = next;
	next->last = edge;
}


static void
ddEdge_linkAfter(ddEdge* last, ddEdge* edge)
{
	if ( last == NULL )
		return;

	if ( last == edge )
		dd_log("last == edge in ddEdge_linkAfter");
	
	if ( last->next != NULL )
		last->next->last = edge;

	edge->next = last->next;
	edge->last = last;
	last->next = edge;
}


static void
ddEdge_unlink(ddEdge* edge)
{
	if ( edge->last != NULL )
		edge->last->next = edge->next;

	if ( edge->next != NULL )
		edge->next->last = edge->last;

	edge->next = NULL;
	edge->last = NULL;
}

/*
static void
ddEdge_replace(ddEdge* edge, ddEdge* with)
{
	with->next = edge->next;
	with->last = edge->last;

	if ( edge->last != NULL )
		edge->last->next = with;

	if ( edge->next != NULL )
		edge->next->last = with;

	edge->next = NULL;
	edge->last = NULL;
}
*/

void
ddEdge_swap(ddEdge* left, ddEdge* right)
{
	if ( left->last != NULL )
		left->last->next = right;

	if ( right->next != NULL )
		right->next->last = left;
	
	left->next = right->next;
	right->last = left->last;
	left->last = right;
	right->next = left;
}


void
ddEdge_unlinkList(ddEdge* edge)
{
	while ( edge != NULL )
	{
		ddEdge* next = edge->next;
		ddEdge_unlink(edge);
		edge = next;
	}
}


static inline boolean
insertBeforeEdge(ddEdge* edge1, ddEdge* edge2, int line)
{
	// if edge1 doesn't pass pixel boundary, go ahead and drop it here
	
	if ( edge1->endy < FIXED_I(line+1) )
		return DD_TRUE;
	
	// if they both cross the pixel boundary, sort by crossing x
	
	if ( edge2->endy >= FIXED_I(line+1) )
		return (edge1->x + edge1->dx < edge2->x + edge2->dx);
	
	// otherwise, leave it until we can compare against an edge that crosses
	
	return DD_FALSE;
}


void
ddEdgeList_reset(ddEdgeList* list)
{
	ddEdge_unlinkList(list->start);
	
	list->start = NULL;
	list->index = 0;
	
	if ( list->next != NULL )
		ddEdgeList_reset(list->next);
}


void
ddEdgeList_advanceToScanline(ddEdgeList* list, int y, fixed clipLeft, fixed clipRight)
{
	// add all vertices starting on or before scanline y

	int idx = list->index;

	ddEdge* head = list->start;
	ddEdge* edge = head;

	fixed fy = FIXED_I(y+1);

	dd_log("advancing to scanline %i", y);
	
	while ( idx < list->nEdges )
	{
		ddEdge* listedge = list->sortedEdges[idx];
		
		if ( listedge->starty >= fy )
			break;
		
		if ( listedge->endy < FIXED_I(y+1) )
			dd_log("edge %i (%p): (%i.%i, %i) -> (%i.%i, %i)", idx, listedge, listedge->startx / 1024, listedge->startx % 1024, listedge->starty % 1024, listedge->endx / 1024, listedge->endx % 1024, listedge->endy % 1024);
		else
			dd_log("edge %i (%p): (%i.%i, %i) -> %i.%i", idx, listedge, listedge->startx / 1024, listedge->startx % 1024, listedge->starty % 1024, (listedge->initx + listedge->dx) / 1024, (listedge->initx + listedge->dx) % 1024);
		
		++idx;

		if ( listedge->endy < FIXED_I(y) )
			continue;

		// if we're adding the line from the start, use initx, inity

		if ( listedge->starty > FIXED_I(y) )
		{
			//if ( (listedge->startx < clipLeft && (listedge->dx <= 0 || listedge->endx <= clipLeft)) ||
			//	 (listedge->startx > clipRight && (listedge->dx >= 0 || listedge->endx >= clipRight)) )
			//	continue;
			
			listedge->x = listedge->initx;
			listedge->y = listedge->inity;
		}
		else
		{
			// else compute them
			
			listedge->x = listedge->initx + (y - INT_F(listedge->starty)) * listedge->dx;
			listedge->y = -(FIXED_I(1) - FRACT_F(listedge->x)) * listedge->dy / FIXED_I(1);

			//if ( (listedge->x < clipLeft && listedge->dx <= 0) ||
			//	 (listedge->x > clipRight && listedge->dx >= 0) )
			//	continue;
		}
		
		// add to active list - sort by bottom edge crossing

		if ( listedge->endy < FIXED_I(y+1) )
			dd_log("edge %p ends this line", listedge);

		if ( head == NULL )
		{
			dd_log("adding edge %p to empty list", listedge);
			edge = head = listedge;
		}
		else if ( insertBeforeEdge(listedge, head, y) )
		{
			dd_log("adding edge %p before %p", listedge, head);
			ddEdge_linkBefore(listedge, head);
			edge = head = listedge;
		}
		else
		{
			// XXX - I'm not sure it shouldn't be starting at head each time..?
			edge = head;
			
			while ( edge->next != NULL )
			{
				if ( insertBeforeEdge(listedge, edge->next, y) )
					break;
				
				edge = edge->next;
			}
			
			dd_log("adding edge %p after %p", listedge, edge);
			ddEdge_linkAfter(edge, listedge);
			edge = listedge;
		}
	}

	list->index = idx;
	list->start = head;
	
	//if ( list->next != NULL )
	//	ddEdgeList_advanceToScanline(list->next, y, clipLeft, clipRight);
}


void
ddEdgeList_removeEdgesBeforeScanline(ddEdgeList* list, int y, fixed clipLeft, fixed clipRight)
{
	// remove all vertices ending before scanline y or tracked outside of clip range

	ddEdge* edge = list->start;
	ddEdge* head = edge;
	
	fixed fy = FIXED_I(y);

	while ( edge != NULL )
	{
		ddEdge* next = edge->next;

		if ( edge->endy < fy /*||
			 (edge->x < clipLeft && edge->dx <= 0) ||
			 (edge->x > clipRight && edge->dx >= 0)*/ )
		{
			if ( edge == head )
				head = next;
			
			ddEdge_unlink(edge);
		}

		edge = next;
	}

	list->start = head;
	
	if ( list->next != NULL )
		ddEdgeList_removeEdgesBeforeScanline(list->next, y, clipLeft, clipRight);	  
}


#include "alphablend.h"

static inline void
doHorizontalWu(ddColor* row, ddColor color, fixed x, int clipleft, int clipright)
{
	fixed fract = FRACT_F(x) >> (FIXED_SHIFT - 8);
	int xi = INT_F(x);
	
	if ( fract <= 0x80 )
	{
		// spills left

		if ( xi > clipleft )
			row[xi-1] = alpha_blend(row[xi-1], ddColor_scale(color, 0x80 - fract));
		
		row[xi] = alpha_blend(row[xi], ddColor_scale(color, 0x7f + fract));
	}
	else
	{
		// spills right
		
		row[xi] = alpha_blend(row[xi], ddColor_scale(color, 0x180 - fract));

		if ( xi < clipright - 1 )
			row[xi+1] = alpha_blend(row[xi+1], ddColor_scale(color, fract - 0x7f));
	}
}

static inline void
doVerticalWu(UInt8* data, int rowstride, int x, ddColor color, fixed y, int cliptop, int clipbottom)
{
	fixed fract = FRACT_F(y) >> (FIXED_SHIFT - 8);
	int yi = INT_F(y);
	ddColor* p;

	data += yi * rowstride;
	
	p = (ddColor*)data + x;
	
	if ( fract <= 0x80 )
	{
		// spills up
		
		*p = alpha_blend(*p, ddColor_scale(color, 0x7f + fract));

		if ( yi > cliptop )
		{
			p = (ddColor*)((UInt8*)p - rowstride);
			*p = alpha_blend(*p, ddColor_scale(color, 0x80 - fract));
		}
	}
	else
	{
		// spills down
		
		*p = alpha_blend(*p, ddColor_scale(color, 0x180 - fract));
		
		if ( yi < clipbottom - 1 )
		{
			p = (ddColor*)((UInt8*)p + rowstride);
			*p = alpha_blend(*p, ddColor_scale(color, fract - 0x7f));
		}
	}
}

void
ddEdge_stroke(ddEdge* edge, ddColor color, ddImage* image, ddRect clipRect)
{
	// do wu antialiasing on edge
	
	fixed x;
	fixed y;
	fixed endx;
	fixed endy;
	fixed dx = edge->dx;
	int rowstride;
	
	if ( edge->flipped )
	{
		x = edge->endx;
		y = edge->endy;
		endx = edge->startx;
		endy = edge->starty;
	}
	else
	{
		x = edge->startx;
		y = edge->starty;
		endx = edge->endx;
		endy = edge->endy;
	}
	
	if ( (y < clipRect.top && endy < clipRect.top) ||
		 (y > clipRect.bottom && endy > clipRect.bottom) ||
		 (x < clipRect.left && endx < clipRect.left) ||
		 (x > clipRect.right && endx > clipRect.right) )
		return;
	
	if ( y < clipRect.top )
	{
		x += fixed_mult(dx, clipRect.top - y);
		y = clipRect.top;
	}
	else if ( y >= clipRect.bottom )
	{
		x -= fixed_mult(dx, y - (clipRect.bottom - 1));
		y = clipRect.bottom - 1;
	}

	if ( (y < clipRect.top && endy < clipRect.top) ||
		 (y > clipRect.bottom && endy > clipRect.bottom) ||
		 (x < clipRect.left && endx < clipRect.left) ||
		 (x > clipRect.right && endx > clipRect.right) )
		return;
	
	if ( x < clipRect.left )
	{
		if ( edge->flipped )
			y += fixed_mult(clipRect.left - x, -edge->dy);
		else
			y += fixed_mult(clipRect.left - x, edge->dy);
		
		x = clipRect.left;
	}
	else if ( x > clipRect.right )
	{
		if ( edge->flipped )
			y += fixed_mult(x - clipRect.right, -edge->dy);
		else
			y += fixed_mult(x - clipRect.right, edge->dy);
		
		x = clipRect.right;
	}
	
	rowstride = ddImage_getRowstride(image);
	
	if ( abs(endy - y) > abs(x - endx) )
	{
		// heading up/down
		
		int yi = INT_F(y);
		
		int clipleft = INT_F(clipRect.left);
		int clipright = INT_F(clipRect.right);
		
		UInt8* line = ddImage_getData(image) + INT_F(y) * rowstride;
		int endyi = INT_F(endy);
		
		// project to middle of the scan line
		
		x += (dx * ((FIXED_I(1) / 2) - FRACT_F(y))) / FIXED_I(1);
		y += (FIXED_I(1) / 2) - FRACT_F(y);
		
		if ( edge->flipped )
			dx = -dx;
		
		if ( y < endy )
		{
			if ( endyi > INT_F(clipRect.bottom) )
				endyi = INT_F(clipRect.bottom);

			while ( yi < /*<=*/ endyi && x > clipRect.left && x < clipRect.right )
			{
				doHorizontalWu((ddColor*)line, color, x, clipleft, clipright);
				
				line += rowstride;
				x += dx;
				++yi;
			}
		}
		else
		{
			while ( yi > /*>=*/ endyi && x > clipRect.left && x < clipRect.right )
			{
				if ( endyi < 0 )
					endyi = -1;
				
				doHorizontalWu((ddColor*)line, color, x, clipleft, clipright);
				
				line -= rowstride;
				x += dx;
				--yi;
			}
		}
	}
	else
	{
		// heading left/right
		
		fixed dy = edge->dy;
		
		int xi = INT_F(x);
		int endxi = INT_F(endx);
		
		int cliptop = INT_F(clipRect.top);
		int clipbottom = INT_F(clipRect.bottom);
		
		UInt8* data = ddImage_getData(image);
		
		// project to middle of this column
		
		if ( edge->flipped && x < endx )
			dy = -dy;
		
		y += (dy * ((FIXED_I(1) / 2) - FRACT_F(x))) / FIXED_I(1);
		x += (FIXED_I(1) / 2) - FRACT_F(x);
		
		if ( edge->flipped && x > endx )
			dy = -dy;
		
		if ( x < endx )
		{
			if ( endxi > INT_F(clipRect.right) )
				endxi = INT_F(clipRect.right) + 1;
			
			while ( xi < /*<=*/ endxi && y < clipRect.bottom && y > clipRect.top )
			{
				doVerticalWu(data, rowstride, xi, color, y, cliptop, clipbottom);
			
				y += dy;
				++xi;
			}
		}
		else
		{
			if ( endxi < INT_F(clipRect.left) )
				endxi = INT_F(clipRect.left) - 1;
			
			while ( xi > /*>=*/ endxi && y < clipRect.bottom && y > clipRect.top )
			{
				doVerticalWu(data, rowstride, xi, color, y, cliptop, clipbottom);
				
				y += dy;
				--xi;
			}
		}
	}
}


void
ddEdgeList_strokeEdges(ddEdgeList* edgeList, ddImage* image, ddMatrix matrix, ddCXform cXform, ddRect clipRect)
{
	int i;

#ifdef DD_RENDER_TOPDOWN
	if ( edgeList->next != NULL )
		ddEdgeList_strokeEdges(edgeList->next, image, matrix, cXform, clipRect);
#endif
	
	if ( ddRect_intersectsRect(edgeList->bounds, clipRect) )
	{
		for ( i = 0; i < edgeList->nEdges; ++i )
		{
			ddEdge* edge = &edgeList->edges[i];
			ddColor color;

			if ( edge->lineWidth == 0 )
				continue;
			
			// XXX - if edge->lineWidth * det(matrix) > one pixel,
			// generate an outline and fill it

			color = ddColor_premultiply(edge->lineColor);

			// XXX - ddCXform_apply also premultiplies
			color = ddCXform_apply(cXform, color);

			ddEdge_stroke(edge, color, image, clipRect);
		}
	}

#ifndef DD_RENDER_TOPDOWN
	if ( edgeList->next != NULL )
		ddEdgeList_strokeEdges(edgeList->next, image, matrix, cXform, clipRect);
#endif
}


static inline void
addAlpha(ddFillInstance* fill, int x, int clipleft, int clipright, fixed c)
{
	if ( fill == NULL || c == 0 )
		return;

	if ( x < clipleft )
	{
		fill->left = clipleft;
		return;
	}
	else if ( x >= clipright )
	{
		fill->right = clipright;
		return;
	}

	if ( x < 0 || x > ALPHABUF_SIZE )
		fprintf(stderr, "bad x!"); // XXX
	
#ifdef DD_LOG
	if ( x > ALPHABUF_SIZE )
		dd_log("x too big!");
#endif

	fill->alphaBuffer[x] += c;

	fill->left = dd_min(fill->left, x);
	fill->right = dd_max(fill->right, x + 1);
}


#ifdef DD_DEBUG
void
listEdges(ddEdge* edge)
{
	while ( edge != NULL )
	{
		fprintf(stderr, "(%i.%i, %i.%i) -> (%i.%i, %i.%i)\n",
		  edge->startx / 1024, edge->startx % 1024, edge->starty / 1024, edge->starty % 1024,
		  edge->endx / 1024, edge->endx % 1024, edge->endy / 1024, edge->endy % 1024);

		edge = edge->next;
	}
}
#endif


void
doFill(ddEdgeList* list, ddEdge* leftedge, ddEdge* rightedge, int endleft, int endright)
{
	// fill in solid area

	int x = INT_F(leftedge->x);
	int end = INT_F(rightedge->x);
	
	ddFillInstance* fill;
	
	if ( list->swapFills )
		fill = leftedge->leftfill;
	else
		fill = leftedge->rightfill;
	
	if ( fill == NULL || x >= endright || end <= endleft )
		return;

	if ( x < endleft )
		x = endleft;
	
	if ( end > endright )
		end = endright;

	dd_log("filling %p (#%X) from %i to %i", fill, fill->fill.data.color, x, end);
	
	while ( x < end )
		fill->alphaBuffer[x++] += ALPHABUF_MAX;
}


void
ddEdge_render(ddEdge* edge, ddEdgeList* list, int endleft, int endright, int y)
{
	ddFillInstance* leftfill = NULL;
	ddFillInstance* rightfill = NULL;
	fixed c;
	int x;
	
	if ( list->swapFills )
	{
		leftfill = edge->rightfill;
		rightfill = edge->leftfill;
	}
	else
	{
		leftfill = edge->leftfill;
		rightfill = edge->rightfill;
	}
	
	dd_log("edge %p: x = %i, left = %p, right = %p", edge, edge->x, leftfill, rightfill);
	
	// XXX - don't need to do anything if we're drawing top-down and pixels are already covered
	
	if ( edge->dx > 0 || edge->starty == edge->endy )
	{
		// edge heads right (edge: noun; heads: verb)
		
		int endx = edge->x + edge->dx;
		
		if ( edge->endx < endx || edge->starty == edge->endy )
			endx = edge->endx;
		
		if ( edge->starty >= FIXED_I(y) )
		{
			// edge starts in this scan line
			
			x = INT_F(edge->startx);
			
			if ( edge->endy < FIXED_I(y+1) && edge->endx < FIXED_I(x+1) )
			{
				// also ends in this pixel
				
				c = (edge->endx - edge->startx) * (FIXED_I(1) - ((FRACT_F(edge->endy) + FRACT_F(edge->starty)) / 2));
				
				addAlpha(leftfill, x, endleft, endright, c);
				addAlpha(rightfill, x, endleft, endright, ALPHABUF_MAX-c);
				
				return;
			}
			else
			{
				if ( edge->x + edge->dx > FIXED_I(x+1) )
				{
					// edge exits right, trapezoid
					
					edge->y += edge->dy;
					
					c = (FIXED_I(1) - FRACT_F(edge->startx)) * (FIXED_I(1) - ((FRACT_F(edge->starty) + edge->y) / 2));
					
					addAlpha(leftfill, x, endleft, endright, c);
					addAlpha(rightfill, x, endleft, endright, ALPHABUF_MAX-c);
				}
				else
				{
					edge->x += edge->dx;
					
					// edge exits bottom, triangle
					c = (FIXED_I(1) - FRACT_F(edge->starty)) * (FRACT_F(edge->x) - FRACT_F(edge->startx)) / 2;
					
					addAlpha(leftfill, x, endleft, endright, c);
					addAlpha(rightfill, x, endleft, endright, ALPHABUF_MAX-c);
					
					return;
				}
			}
		}
		else
		{
			// edge enters from top of pixel
			
			x = INT_F(edge->x);
			
			if ( edge->endx < FIXED_I(x+1) && FIXED_I(y+1) > edge->endy )
			{
				// edge ends in this pixel, coverage is rect minus triangle
				c = FIXED_I(1) * FRACT_F(edge->endx) - (FRACT_F(edge->endx) - FRACT_F(edge->x)) * FRACT_F(edge->endy) / 2;
				
				addAlpha(leftfill, x, endleft, endright, c);
				addAlpha(rightfill, x, endleft, endright, ALPHABUF_MAX-c);
				
				return;
			}
			else if ( FRACT_F(edge->x) + edge->dx < FIXED_I(1) )
			{
				// edge exits bottom of pixel, coverage is a trapezoid
				c = (FRACT_F(edge->x) + edge->dx / 2) * FIXED_I(1);
				
				addAlpha(leftfill, x, endleft, endright, c);
				addAlpha(rightfill, x, endleft, endright, ALPHABUF_MAX-c);
				
				edge->x += edge->dx;
				
				return;
			}
			else
			{
				// edge exits right, coverage is a triangle
				edge->y += edge->dy;
				
				c = ((FIXED_I(1) - FRACT_F(edge->x)) * edge->y) / 2;
				
				addAlpha(leftfill, x, endleft, endright, ALPHABUF_MAX-c);
				addAlpha(rightfill, x, endleft, endright, c);
			}
		}
		
		while ( ++x < INT_F(endx) )
		{
			// edge continues right, coverage is a trapezoid
			c = (edge->y + edge->dy / 2) * FIXED_I(1);
			
			addAlpha(leftfill, x, endleft, endright, ALPHABUF_MAX-c);
			addAlpha(rightfill, x, endleft, endright, c);
			
			edge->y += edge->dy;
		}
		
		edge->x += edge->dx;
		
		if ( edge->x >= edge->endx && edge->endy != FIXED_I(y+1) )
		{
			// edge ends in this pixel, coverage is rect plus triangle
			// (or rect minus trapezoid)
			
			c = FRACT_F(edge->endx) * (FIXED_I(1) - (FRACT_F(edge->endy) + edge->y) / 2);
			
			addAlpha(leftfill, x, endleft, endright, c);
			addAlpha(rightfill, x, endleft, endright, ALPHABUF_MAX-c);
			
			return;
		}
		
		if ( edge->y < FIXED_I(1) || edge->endy == FIXED_I(y+1) )
		{
			// last bit is a triangle
			c = FRACT_F(edge->x) * (FIXED_I(1) - edge->y) / 2;
			
			addAlpha(leftfill, x, endleft, endright, c);
			addAlpha(rightfill, x, endleft, endright, ALPHABUF_MAX-c);
		}
		else
			// make sure this last pixel is covered..
			addAlpha(rightfill, x, endleft, endright, ALPHABUF_MAX);
	}
	else if ( edge->dx < 0 )
	{
		// edge heads left
		
		int endx = edge->x + edge->dx;
		
		if ( edge->endx > endx )
			endx = edge->endx;
		
		if ( edge->starty >= FIXED_I(y) )
		{
			// edge starts in this scan line
			
			x = INT_F(edge->startx);
			
			if ( edge->endy < FIXED_I(y+1) && edge->endx > FIXED_I(x) )
			{
				// also ends in this pixel
				
				c = (edge->startx - edge->endx) * (FIXED_I(1) - ((FRACT_F(edge->endy) + FRACT_F(edge->starty)) / 2));
				
				addAlpha(leftfill, x, endleft, endright, ALPHABUF_MAX-c);
				addAlpha(rightfill, x, endleft, endright, c);
				
				return;
			}
			else
			{
				if ( edge->x + edge->dx < FIXED_I(x) )
				{
					// edge exits left, trapezoid
					
					edge->y += edge->dy;
					
					c = FRACT_F(edge->startx) * (FIXED_I(1) - ((FRACT_F(edge->starty) + edge->y) / 2));
					
					addAlpha(leftfill, x, endleft, endright, ALPHABUF_MAX-c);
					addAlpha(rightfill, x, endleft, endright, c);
				}
				else
				{
					edge->x += edge->dx;
					
					// edge exits bottom, triangle
					c = (FIXED_I(1) - FRACT_F(edge->starty)) * (FRACT_F(edge->startx) - FRACT_F(edge->x)) / 2;
					
					addAlpha(leftfill, x, endleft, endright, ALPHABUF_MAX-c);
					addAlpha(rightfill, x, endleft, endright, c);
					
					return;
				}
			}
		}
		else
		{
			// edge enters from top of pixel
			
			x = INT_F(edge->x);
			
			if ( edge->endx >= FIXED_I(x) && edge->endy < FIXED_I(y+1) )
			{
				// edge ends in this pixel, coverage is rect minus triangle
				c = FIXED_I(1) * (FIXED_I(1) - FRACT_F(edge->endx)) - (FRACT_F(edge->x) - FRACT_F(edge->endx)) * FRACT_F(edge->endy) / 2;
				
				addAlpha(leftfill, x, endleft, endright, ALPHABUF_MAX-c);
				addAlpha(rightfill, x, endleft, endright, c);
				
				return;
			}
			else if ( FRACT_F(edge->x) + edge->dx > 0 )
			{
				// edge exits bottom of pixel, coverage is a trapezoid
				c = (FRACT_F(edge->x) + edge->dx / 2) * FIXED_I(1);
				
				addAlpha(leftfill, x, endleft, endright, c);
				addAlpha(rightfill, x, endleft, endright, ALPHABUF_MAX-c);
				
				edge->x += edge->dx;
				
				return;
			}
			else
			{
				// edge exits left, coverage is a triangle
				edge->y += edge->dy;
				
				c = (FRACT_F(edge->x) * edge->y) / 2;
				
				addAlpha(leftfill, x, endleft, endright, c);
				addAlpha(rightfill, x, endleft, endright, ALPHABUF_MAX-c);
			}
		}
		
		while ( --x > INT_F(endx) )
		{
			// edge continues left, coverage is a trapezoid
			c = (edge->y + edge->dy / 2) * FIXED_I(1);
			
			addAlpha(leftfill, x, endleft, endright, c);
			addAlpha(rightfill, x, endleft, endright, ALPHABUF_MAX-c);
			
			edge->y += edge->dy;
		}
		
		edge->x += edge->dx;
		
		if ( edge->x <= edge->endx && edge->endy != FIXED_I(y+1) )
		{
			// edge ends in this pixel - coverage is rect plus triangle
			// (or rect minus trapezoid)
			
			c = (FIXED_I(1) - FRACT_F(edge->endx)) * (FIXED_I(1) - (FRACT_F(edge->endy) + edge->y) / 2);
			
			addAlpha(leftfill, x, endleft, endright, ALPHABUF_MAX-c);
			addAlpha(rightfill, x, endleft, endright, c);
			
			return;
		}
		
		if ( edge->y < FIXED_I(1) || edge->endy == FIXED_I(y+1) )
		{
			// last bit is a triangle
			c = (FIXED_I(1) - FRACT_F(edge->x)) * (FIXED_I(1) - edge->y) / 2;
			
			addAlpha(leftfill, x, endleft, endright, ALPHABUF_MAX-c);
			addAlpha(rightfill, x, endleft, endright, c);
		}
		else
			// make sure this last pixel is covered..
			addAlpha(leftfill, x, endleft, endright, ALPHABUF_MAX);
	}
	else // edge->dx == 0
	{
		// if edge starts in this scan line, ignore- another edge will take care of it
		
		x = INT_F(edge->x);
		
		if ( edge->starty > FIXED_I(y) )
		{
			if ( edge->endy >= FIXED_I(y+1) )
			{
				addAlpha(leftfill, x, endleft, endright, 0);
				addAlpha(rightfill, x, endleft, endright, 0);
			}
			
			return;
		}
		
		if ( INT_F(edge->x) < endleft || INT_F(edge->x) > endright )
		{
			addAlpha(leftfill, x, endleft, endright, ALPHABUF_MAX);
			addAlpha(rightfill, x, endleft, endright, ALPHABUF_MAX);
		}
		else
		{
			c = (FIXED_I(1) - FRACT_F(edge->x)) * FIXED_I(1);
		
			addAlpha(leftfill, x, endleft, endright, ALPHABUF_MAX-c);
			addAlpha(rightfill, x, endleft, endright, c);
		}
	}
}


static void
addFillToList(ddFillInstance** list, ddFillInstance* fill)
{
	if ( fill == NULL || fill->listed )
		return;

	fill->next = *list;
	*list = fill;
	fill->listed = DD_TRUE;
}


void
ddEdgeList_renderScanline(ddEdgeList* list, int y, fixed clipLeft, fixed clipRight,
						  ddImage* image, ddCXform cXform, UInt8* maskBuffer)
{
	// rasterize edges in current list
	static ddColor buffer[ALPHABUF_SIZE]; // XXX

	ddEdge* edge;
	int width = ddImage_getWidth(image);
	int left = width;
	int right = -1;

	int endleft = INT_F(clipLeft);
	int endright = INT_F(clipRight - 1) + 1;
	
	ddFillInstance* filllist = NULL;
	ddFillInstance* fill = NULL;

#ifdef DD_RENDER_TOPDOWN
	if ( list->next != NULL )
		ddEdgeList_renderScanline(list->next, y, clipLeft, clipRight, image, cXform, maskBuffer);
#endif
	
	ddEdgeList_advanceToScanline(list, y, clipLeft, clipRight);

	edge = list->start;

dd_log("rasterizing line at y = %i", y);

	// first edge should have correct leftfill for clipped area

	//if ( edge != NULL && edge->leftfill != NULL )
	//	edge->leftfill->lastEdge = clipLeft;
	
	// XXX - can start at first edge before clipLeft, finish after processing last edge after clipRight

	while ( edge != NULL )
	{
		ddEdge_render(edge, list, endleft, endright, y);
		
		// reset y to avoid drift
		
		if ( INT_F(edge->endy) > y )
		{
			if ( edge->dx > 0 )
				edge->y = -FRACT_F(edge->x) * edge->dy / FIXED_I(1);
			else
				edge->y = -(FIXED_I(1) - FRACT_F(edge->x)) * edge->dy / FIXED_I(1);
		}
		
		// make list of fills we're using this run
		
		addFillToList(&filllist, edge->rightfill);
		addFillToList(&filllist, edge->leftfill);
		
		edge = edge->next;
	}
	
	ddEdgeList_removeEdgesBeforeScanline(list, y+1, clipLeft, clipRight);

	// render fills' alphaBuffers
	
	edge = list->start;

	while ( edge != NULL )
	{
		if ( edge->next == NULL )
			break;
		
		doFill(list, edge, edge->next, endleft, endright);
		edge = edge->next;
	}
	
	// tell each fill we've used to draw into the temp line buffer
	
	memset(buffer, 0, sizeof(ddColor) * ALPHABUF_SIZE); // XXX
	
	fill = filllist;

	while ( fill != NULL )
	{
		ddFillInstance* next = fill->next;
		ddRasterMethod scan = ddFillInstance_getRasterMethod(fill);

		if ( fill->left < 0 || fill->right > width )
			fprintf(stderr, "bad fill bounds");
		
		if ( maskBuffer != NULL )
		{
			int i;
			
			for ( i = fill->left; i < fill->right; ++i )
			{
				if ( maskBuffer[i] == 0xff )
					continue;
				
				if ( maskBuffer[i] == 0 )
					fill->alphaBuffer[i] = 0;
				else
					fill->alphaBuffer[i] = (int)fill->alphaBuffer[i] * maskBuffer[i] / 0xff;
			}
		}

		scan(buffer, fill, y, cXform);

		left = dd_min(left, fill->left);
		right = dd_max(right, fill->right);

		if ( fill->left < fill->right )
			memset(fill->alphaBuffer + fill->left, 0, sizeof(UInt32) * (fill->right - fill->left));
		
		fill->left = width;
		fill->right = -1;

		fill->next = NULL;
		fill->listed = DD_FALSE;

		fill = next;
	}
	
	// finally, composite the temp buffer into the image
	
	ddImage_compositeTempBuffer(image, buffer, y, left, right);

#ifndef DD_RENDER_TOPDOWN
	if ( list->next != NULL )
		ddEdgeList_renderScanline(list->next, y, clipLeft, clipRight, image, cXform, maskBuffer);
#endif
}


void
ddEdgeList_renderMaskBuffer(ddEdgeList* list, int y, fixed clipLeft, fixed clipRight, ddImage* image,
							UInt8* maskBuffer)
{
	ddEdge* edge;
	
	int width = ddImage_getWidth(image);
	int left = width;
	int right = -1;

	int endleft = INT_F(clipLeft);
	int endright = INT_F(clipRight - 1) + 1;
	
	ddFillInstance* filllist = NULL;
	ddFillInstance* fill = NULL;
	
	if ( list->next != NULL )
		ddEdgeList_renderMaskBuffer(list->next, y, clipLeft, clipRight, image, maskBuffer);
	
	ddEdgeList_advanceToScanline(list, y, clipLeft, clipRight);
	
	edge = list->start;
	
	while ( edge != NULL )
	{
		ddEdge_render(edge, list, endleft, endright, y);
		
		// reset y to avoid drift
		
		if ( INT_F(edge->endy) > y )
		{
			if ( edge->dx > 0 )
				edge->y = -FRACT_F(edge->x) * edge->dy / FIXED_I(1);
			else
				edge->y = -(FIXED_I(1) - FRACT_F(edge->x)) * edge->dy / FIXED_I(1);
		}
		
		// make list of fills we're using this run
		
		addFillToList(&filllist, edge->rightfill);
		addFillToList(&filllist, edge->leftfill);
		
		edge = edge->next;
	}
	
	ddEdgeList_removeEdgesBeforeScanline(list, y+1, clipLeft, clipRight);
	
	// copy fills' alphaBuffers into maskBuffer
	
	edge = list->start;
	
	while ( edge != NULL )
	{
		if ( edge->next == NULL )
			break;
		
		doFill(list, edge, edge->next, endleft, endright);
		edge = edge->next;
	}

	fill = filllist;
	
	while ( fill != NULL )
	{
		ddFillInstance* next = fill->next;
		int i;
		
		if ( fill->left < 0 || fill->right > width )
			fprintf(stderr, "bad fill bounds");
		
		left = dd_min(left, fill->left);
		right = dd_max(right, fill->right);
		
		for ( i = left; i < right; ++i )
		{
			if ( maskBuffer[i] + fill->alphaBuffer[i] > 0xff )
				maskBuffer[i] = 0xff;
			else
				maskBuffer[i] += fill->alphaBuffer[i];
		}
		
		if ( fill->left < fill->right )
			memset(fill->alphaBuffer + fill->left, 0, sizeof(UInt32) * (fill->right - fill->left));
		
		fill->left = width;
		fill->right = -1;
		
		fill->next = NULL;
		fill->listed = DD_FALSE;
		
		fill = next;
	}
}


void
ddEdgeList_renderMaskedScanline(ddEdgeList* list, ddEdgeList* masklist, int y,
						  fixed clipLeft, fixed clipRight, ddImage* image, ddCXform cXform)
{
	static UInt8 maskBuffer[ALPHABUF_SIZE]; // XXX
	
	memset(maskBuffer, 0, sizeof(UInt8) * ALPHABUF_SIZE); // XXX

	ddEdgeList_renderMaskBuffer(masklist, y, clipLeft, clipRight, image, maskBuffer);
	
	ddEdgeList_renderScanline(list, y, clipLeft, clipRight, image, cXform, maskBuffer);
}


int
ddEdgeList_hitTest(ddEdgeList* list, fixed x, fixed y)
{
	int i = 0;
	
	fixed maxcrossing = -1;
	ddFillInstance* fill = NULL;
	
	if ( list->next != NULL && ddEdgeList_hitTest(list->next, x, y) )
		return DD_TRUE;
	
	if ( !ddRect_containsPoint(list->bounds, x, y) )
		return DD_FALSE;
	
	/* note: we expect s->vlist and s->bounds to be up to date.. */

	// find the edge closest to the left of the point

	for ( i = 0; i < list->nEdges; ++i )
	{
		ddEdge* edge = list->sortedEdges[i];

		if ( edge->starty > y )
			break;

		if ( edge->endy > y )
		{
			fixed crossing = edge->startx +
				(y - edge->starty) * (edge->endx - edge->startx) / (edge->endy - edge->starty);

			if ( crossing < x && crossing > maxcrossing )
			{
				maxcrossing = crossing;
				fill = edge->rightfill;
			}
		}
	}

	return (fill != NULL);
}


#ifdef DD_DEBUG

void
dd_logEdgeList(ddEdgeList* list)
{
	int i;

	for ( i = 0; i < list->nEdges; ++i )
	{
		ddEdge* edge = &list->edges[i];

		fprintf(stderr, "edge %i (%i, %i) -> (%i, %i)\n", i, edge->startx, edge->starty, edge->endx, edge->endy);
	}
}

#endif /* DD_DEBUG */
