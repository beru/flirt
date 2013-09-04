/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_RENDER_SHAPE_H_INCLUDED
#define DD_RENDER_SHAPE_H_INCLUDED

#include "../dd.h"

typedef struct _ddShape ddShape;
typedef struct _ddShapeRecord ddShapeRecord;

#include "fill.h"
#include "matrix.h"
#include "rect.h"

typedef enum
{
	SHAPERECORD_END      = 0,
	SHAPERECORD_MOVETO   = 1,
	SHAPERECORD_NEWFILL0 = 2,
	SHAPERECORD_NEWFILL1 = 3,
	SHAPERECORD_NEWLINE  = 4,
	SHAPERECORD_NEWPATH  = 5,
	SHAPERECORD_LINETO   = 8,
	SHAPERECORD_CURVETO  = 9
} ddShapeRecordType;

#define SHAPERECORD_ISEDGE(r) (((r)->type & 0x08) != 0 )

struct _ddShapeRecord
{
	ddShapeRecordType type;

	union
	{
		struct
		{
			fixed x1;
			fixed y1;
			fixed x2;
			fixed y2;
		} coords;

		struct
		{
			fixed width;
			ddColor color;
		} line;

		int fill;

	} data;
};

struct _ddShape
{
	ddFill* fills;
	int nFills;

	ddShapeRecord* records;
	int nRecords;

	fixed lineWidth;
	ddRect bounds;
	boolean hasCurve; /* if no curves, we can scale w/out rebuilding vlist */
};


ddShape*
dd_newShape();

void
dd_destroyShape(ddShape* s);

ddShape*
dd_newShape_copy(ddShape* shape);

ddRect
ddShape_getBounds(ddShape* s);

void
ddShape_setBounds(ddShape* s, ddRect bounds);

void
ddShape_moveTo(ddShape* s, fixed x, fixed y);

void
ddShape_newPath(ddShape* s);

void
ddShape_lineTo(ddShape* s, fixed x, fixed y);

void
ddShape_curveTo(ddShape* s, fixed x1, fixed y1, fixed x2, fixed y2);

void
ddShape_close(ddShape* s);

void
ddShape_setLeftFill(ddShape* s, int offset, int idx);

void
ddShape_setRightFill(ddShape* s, int offset, int idx);

void
ddShape_setLine(ddShape* s, fixed width, ddColor color);

void
ddShape_end(ddShape* s);

int
ddShape_addFill(ddShape* s, ddFill fill);

int
ddShape_getNFills(ddShape* s);

#ifdef DD_DEBUG
void
dd_logShapeRecord(ddShapeRecord record);

void
dd_logShape(ddShape* shape);
#endif

#endif /* DD_SHAPE_H_INCLUDED */
