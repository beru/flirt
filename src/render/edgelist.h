/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_RENDER_EDGELIST_H_INCLUDED
#define DD_RENDER_EDGELIST_H_INCLUDED

#include "../dd.h"

typedef struct _ddEdgeList ddEdgeList;
typedef struct _ddEdge ddEdge;  // defined in edgelist.c

#include "rect.h"

struct _ddEdgeList
{
	struct _ddEdgeList* next; // next path in the shape
	
	ddRect bounds;
	
	ddEdge* edges;
	int nEdges;
	
	ddEdge** sortedEdges;
	
	int index; // current render index
	ddEdge* start; // render list

	boolean swapFills;
};

#include "shape.h"
#include "shapeinstance.h"
#include "raster.h"
#include "matrix.h"
#include "cxform.h"

ddEdgeList*
dd_newEdgeList(ddShapeInstance* s, ddMatrix matrix);

void
dd_destroyEdgeList(ddEdgeList* edgelist);

void
ddEdgeList_recalcSlopes(ddEdgeList* list, ddMatrix matrix, ddRect* bounds);

void
ddEdgeList_recalcCoords(ddEdgeList* list, ddMatrix matrix, ddRect* bounds);

void
ddEdgeList_displace(ddEdgeList* list, fixed dx, fixed dy, ddRect* bounds);

void
ddEdgeList_renderScanline(ddEdgeList* list, int y, fixed maskLeft, fixed maskRight,
							ddImage* image, ddCXform cXform, UInt8* maskBuffer);

void
ddEdgeList_renderMaskedScanline(ddEdgeList* list, ddEdgeList* maskList,
								int y, int maskLeft, int maskRight, ddImage* image,
								ddCXform cXform);

void
ddEdgeList_advanceToScanline(ddEdgeList* list, int y, fixed left, fixed right);

void
ddEdgeList_reset(ddEdgeList* list);

void
ddEdge_unlinkList(ddEdge* edge);

int
ddEdgeList_hitTest(ddEdgeList* list, fixed x, fixed y);

void
ddEdgeList_strokeEdges(ddEdgeList* edgeList, ddImage* image, ddMatrix matrix, ddCXform cXform, ddRect clipRect);

#ifdef DD_DEBUG
void
dd_logEdgeList(ddEdgeList* list);
#endif

#endif /* DD_RENDER_EDGELIST_H_INCLUDED */
