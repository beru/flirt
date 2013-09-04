/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "shape.h"
#include "edgelist.h"

/* this shape sits in the character list, is never used directly.. */

ddShape*
dd_newShape()
{
	ddShape* s = dd_malloc(sizeof(struct _ddShape));

	s->bounds = ddInvalidRect;
	s->nRecords = 0;
	s->records = NULL;
	s->nFills = 0;
	s->fills = NULL;
	s->hasCurve = DD_FALSE;
	s->lineWidth = 0;

	return s;
}


void
dd_destroyShape(ddShape* s)
{
	int i;

	if ( s->records != NULL )
		dd_free(s->records);

	for ( i = 0; i < s->nFills; ++i )
		dd_destroyFill(&s->fills[i]);

	if ( s->fills != NULL )
		dd_free(s->fills);

	dd_free(s);
}


ddRect
ddShape_getBounds(ddShape* s)
{
	return s->bounds;
}


void
ddShape_setBounds(ddShape* s, ddRect bounds)
{
	s->bounds = bounds;
}


void
ddShapeRecord_copyInto(ddShapeRecord target, ddShapeRecord source);

ddShape*
dd_newShape_copy(ddShape* shape)
{
	int i;
	ddShape* newshape = dd_malloc(sizeof(struct _ddShape));

	newshape->nFills = shape->nFills;
	newshape->fills = dd_malloc(newshape->nFills * sizeof(ddFill));

	for ( i = 0; i < newshape->nFills; ++i )
		newshape->fills[i] = ddFill_copy(shape->fills[i]);

	newshape->nRecords = shape->nRecords;
	newshape->records = dd_malloc(newshape->nRecords *
							   sizeof(struct _ddShapeRecord));

	for ( i = 0; i < newshape->nRecords; ++i )
		newshape->records[i] = shape->records[i];

	newshape->bounds = shape->bounds;

	return newshape;
}


static ddShapeRecord*
ddShape_addRecord(ddShape* s)
{
	s->records = dd_realloc(s->records,
						 (s->nRecords + 1) * sizeof(struct _ddShapeRecord));

	return &(s->records[s->nRecords++]);
}


void
ddShape_moveTo(ddShape* s, fixed x, fixed y)
{
	ddShapeRecord* r = ddShape_addRecord(s);

	r->type = SHAPERECORD_MOVETO;
	r->data.coords.x1 = x;
	r->data.coords.y1 = y;

	s->bounds = ddRect_containPoint(s->bounds, x - s->lineWidth / 2, y - s->lineWidth / 2);
	s->bounds = ddRect_containPoint(s->bounds, x + s->lineWidth / 2, y + s->lineWidth / 2);
}


void
ddShape_newPath(ddShape* s)
{
	ddShapeRecord* r = ddShape_addRecord(s);
	r->type = SHAPERECORD_NEWPATH;
}


void
ddShape_lineTo(ddShape* s, fixed x, fixed y)
{
	ddShapeRecord* r = ddShape_addRecord(s);

	r->type = SHAPERECORD_LINETO;
	r->data.coords.x1 = x;
	r->data.coords.y1 = y;

	s->bounds = ddRect_containPoint(s->bounds, x - s->lineWidth / 2, y - s->lineWidth / 2);
	s->bounds = ddRect_containPoint(s->bounds, x + s->lineWidth / 2, y + s->lineWidth / 2);
}


void
ddShape_curveTo(ddShape* s, fixed x1, fixed y1, fixed x2, fixed y2)
{
	ddShapeRecord* r = ddShape_addRecord(s);

	r->type = SHAPERECORD_CURVETO;
	r->data.coords.x1 = x1;
	r->data.coords.y1 = y1;
	r->data.coords.x2 = x2;
	r->data.coords.y2 = y2;

	// somewhat sloppy, but fast and easy
	s->bounds = ddRect_containPoint(s->bounds, x1 - s->lineWidth / 2, y1 - s->lineWidth / 2);
	s->bounds = ddRect_containPoint(s->bounds, x1 + s->lineWidth / 2, y1 + s->lineWidth / 2);
	s->bounds = ddRect_containPoint(s->bounds, x2 - s->lineWidth / 2, y2 - s->lineWidth / 2);
	s->bounds = ddRect_containPoint(s->bounds, x2 + s->lineWidth / 2, y2 + s->lineWidth / 2);

	s->hasCurve = DD_TRUE;
}


void
ddShape_end(ddShape* s)
{
	ddShapeRecord* r;

	if ( s->records == NULL || s->records[s->nRecords-1].type != SHAPERECORD_END )
	{
		r = ddShape_addRecord(s);
		r->type = SHAPERECORD_END;
	}
}


int
ddShape_addFill(ddShape* s, ddFill fill)
{
	s->fills = dd_realloc(s->fills, sizeof(ddFill) * (s->nFills + 1));
	s->fills[s->nFills] = fill;

	return s->nFills++;
}


int
ddShape_getNFills(ddShape* s)
{
	return s->nFills;
}


void
ddShape_setLeftFill(ddShape* s, int offset, int idx)
{
	ddShapeRecord* r = ddShape_addRecord(s);
	r->type = SHAPERECORD_NEWFILL0;

	if ( idx == 0 )
		r->data.fill = -1;
	else
		r->data.fill = offset + idx - 1;
}


void
ddShape_setRightFill(ddShape* s, int offset, int idx)
{
	ddShapeRecord* r = ddShape_addRecord(s);
	r->type = SHAPERECORD_NEWFILL1;

	if ( idx == 0 )
		r->data.fill = -1;
	else
		r->data.fill = offset + idx - 1;
}


void
ddShape_setLine(ddShape* s, fixed width, ddColor color)
{
	ddShapeRecord* r = ddShape_addRecord(s);
	r->type = SHAPERECORD_NEWLINE;
	s->lineWidth = width;
	r->data.line.width = width;
	r->data.line.color = color;
}


#ifdef DD_DEBUG

void
dd_logShapeRecord(ddShapeRecord record)
{
	switch ( record.type )
	{
		case SHAPERECORD_END:
			fprintf(stderr, "SHAPERECORD_END\n");
			break;

		case SHAPERECORD_MOVETO:
			fprintf(stderr, "SHAPERECORD_MOVETO: (%i, %i)\n", record.data.coords.x1, record.data.coords.y1);
			break;
		
		case SHAPERECORD_LINETO:
			fprintf(stderr, "SHAPERECORD_LINETO: (%i, %i)\n", record.data.coords.x1, record.data.coords.y1);
			break;

		case SHAPERECORD_CURVETO:
			fprintf(stderr, "SHAPERECORD_CURVETO: (%i, %i), (%i, %i)\n", record.data.coords.x1, record.data.coords.y1, record.data.coords.x2, record.data.coords.y2);
			break;

		case SHAPERECORD_NEWFILL0:
			fprintf(stderr, "SHAPERECORD_NEWFILL0: 0x%x\n", record.data.fill);
			break;

		case SHAPERECORD_NEWFILL1:
			fprintf(stderr, "SHAPERECORD_NEWFILL1: 0x%x\n", record.data.fill);
			break;
			
		case SHAPERECORD_NEWLINE:
			fprintf(stderr, "SHAPERECORD_NEWLINE: %i #%x\n", record.data.line.width, record.data.line.color);
			break;
		
		case SHAPERECORD_NEWPATH:
			fprintf(stderr, "SHAPERECORD_NEWPATH\n");
			break;
		
		default:
			dd_error("Unknown shaperecord type: %i", record.type);
	}
}


void
dd_logShape(ddShape* shape)
{
	int i;

	for ( i = 0; i < shape->nRecords; ++i )
	{
		fprintf(stderr, "%02i: ", i);
		dd_logShapeRecord(shape->records[i]);
	}
}

#endif
