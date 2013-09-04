/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "readshape.h"

#include "blocktypes.h"

#include "../render/shape.h"
#include "../render/fill.h"
#include "../render/rect.h"
#include "../player/drawable.h"
#include "../player/morphshape.h"

ddGradient*
readGradient(ddReader* r, int shapeType)
{
	int i, ratio;
	int numGrads = readUInt8(r);
	ddGradient* grad = dd_newGradient();

	for ( i = 0; i < numGrads; ++i )
	{
		ratio = readUInt8(r);

		if ( shapeType == DEFINESHAPE3 )
			ddGradient_add(grad, readRGBA(r), ratio);
		else
			ddGradient_add(grad, readRGB(r), ratio);
	}

	return grad;
}


void
readMorphGradient(ddReader* r, ddGradient** g1, ddGradient** g2)
{
	int i, ratio;
	int numGrads = readUInt8(r);
	ddGradient* grad1 = dd_newGradient();
	ddGradient* grad2 = dd_newGradient();

	for ( i = 0; i < numGrads; ++i )
	{
		ratio = readUInt8(r);
		ddGradient_add(grad1, readRGBA(r), ratio);

		ratio = readUInt8(r);
		ddGradient_add(grad2, readRGBA(r), ratio);
	}

	*g1 = grad1;
	*g2 = grad2;
}


struct _line
{
	int width;
	ddColor color;
};

void
readLineStyleArray(ddReader* r, int shapeType,
				   struct _line** lineStyles1, struct _line** lineStyles2)
{
	int count, i;
	
	struct _line* lines1 = *lineStyles1;
	struct _line* lines2 = NULL;

	if ( lineStyles2 != NULL )
		lines2 = *lineStyles2;

	if ( lines1 != NULL )
		dd_free(lines1);

	if ( lines2 != NULL )
		dd_free(lines2);
		
	count = readUInt8(r);

	if ( count == 0xff )
		count = readUInt16(r);

	lines1 = dd_malloc(count * sizeof(struct _line));
	*lineStyles1 = lines1;

	if  ( shapeType == DEFINEMORPHSHAPE )
	{
		lines2 = dd_malloc(count * sizeof(struct _line));
		*lineStyles2 = lines2;
	}

	for ( i = 0; i < count; ++i )
	{
		lines1[i].width = readUInt16(r);

		if ( shapeType == DEFINEMORPHSHAPE )
			lines2[i].width = readUInt16(r);

		if ( shapeType == DEFINESHAPE3 || shapeType == DEFINEMORPHSHAPE )
			lines1[i].color = readRGBA(r);
		else
			lines1[i].color = readRGB(r);

		if ( shapeType == DEFINEMORPHSHAPE )
			lines2[i].color = readRGBA(r);
	}
}


#define DD_FILL_SOLID 0x00
#define DD_FILL_LINEAR_GRADIENT 0x10
#define DD_FILL_RADIAL_GRADIENT 0x12
#define DD_FILL_TILED_BITMAP 0x40
#define DD_FILL_CLIPPED_BITMAP 0x41

void
readFillStyle(ddMovieClip* p, ddReader* r, int shapeType, ddFill* f1, ddFill* f2)
{
	int type = readUInt8(r);

	if ( type == DD_FILL_SOLID )
	{
		if ( shapeType == DEFINESHAPE3 || shapeType == DEFINEMORPHSHAPE )
			*f1 = dd_newSolidFill(readRGBA(r));
		else
			*f1 = dd_newSolidFill(readRGB(r));

		if ( shapeType==DEFINEMORPHSHAPE )
			*f2 = dd_newSolidFill(readRGBA(r));

		return;
	}
	else if ( type == DD_FILL_LINEAR_GRADIENT || type == DD_FILL_RADIAL_GRADIENT )
	{
		ddMatrix m1 = readMatrix(r);
		ddGradient* g1;
		ddGradient* g2;

#define GRAD_SCALE FIXED_D((16384 * FIXED_I(1) / TWIPS_I(1)) / 65280.0)

		m1 = ddMatrix_scale(m1, GRAD_SCALE, GRAD_SCALE);
		
		if ( shapeType==DEFINEMORPHSHAPE )
		{
			ddMatrix m2 = readMatrix(r);
			readMorphGradient(r, &g1, &g2);

			m2 = ddMatrix_scale(m2, GRAD_SCALE, GRAD_SCALE);

			if ( type == DD_FILL_LINEAR_GRADIENT )
			{
				*f1 = dd_newLinearGradientFill(g1, m1);
				*f2 = dd_newLinearGradientFill(g2, m2);
			}
			else
			{
				*f1 = dd_newRadialGradientFill(g1, m1);
				*f2 = dd_newRadialGradientFill(g2, m2);
			}

		}
		else
		{
			g1 = readGradient(r, shapeType);

			if ( type == DD_FILL_LINEAR_GRADIENT )
				*f1 = dd_newLinearGradientFill(g1, m1);
			else
				*f1 = dd_newRadialGradientFill(g1, m1);
		}

		return;
	}
	else if ( type == DD_FILL_TILED_BITMAP || type == DD_FILL_CLIPPED_BITMAP || type == 66 || type == 67 ) // XXX
	{
		int id = readSInt16(r);
		ddMatrix matrix = readMatrix(r);
		ddCharacter* bitmap;

		if ( id < 0 )
		{
			*f1 = dd_newSolidFill((ddColor)0);
			return;
		}

		bitmap = ddMovieClip_getCharacter(p, id);

		if ( bitmap == NULL || bitmap->type != BITMAP_CHAR )
		{
			dd_warn("Requested character id is not a bitmap");
			*f1 = dd_newSolidFill((ddColor)0);
			return;
		}

		/* fix scaling weirdness */
		matrix.a /= 20;
		matrix.b /= 20;
		matrix.c /= 20;
		matrix.d /= 20;

		if ( type == DD_FILL_TILED_BITMAP )
			*f1 = dd_newTiledBitmapFill(((ddBitmapCharacter*)bitmap)->bitmap, matrix);
		else
			*f1 = dd_newClippedBitmapFill(((ddBitmapCharacter*)bitmap)->bitmap, matrix);

		if ( shapeType == DEFINEMORPHSHAPE )
		{
			ddMatrix m2 = readMatrix(r);

			m2.a /= 20;
			m2.b /= 20;
			m2.c /= 20;
			m2.d /= 20;

			if ( type == DD_FILL_TILED_BITMAP )
				*f2 = dd_newTiledBitmapFill(((ddBitmapCharacter*)bitmap)->bitmap, m2);
			else
				*f2 = dd_newClippedBitmapFill(((ddBitmapCharacter*)bitmap)->bitmap, m2);
		}

		return;
	}
	else
		dd_error("Unknown fill type: %i", type);
}


void
readFillStyleArray(ddMovieClip* p, ddReader* r, Blocktype shapeType, ddShape* shape1, ddShape* shape2)
{
	int count, i;
	ddFill f1, f2;

	count = readUInt8(r);

	if ( count == 255 )
		count = readUInt16(r);

	for ( i = 0; i < count; ++i )
	{
		readFillStyle(p, r, shapeType, &f1, &f2);

		(void)ddShape_addFill(shape1, f1);

		if ( shapeType == DEFINEMORPHSHAPE )
			(void)ddShape_addFill(shape2, f2);
	}
}


int
readShapeRec(ddMovieClip* p, ddReader* r, ddShape* shape,
			 int* lineBits, int* fillBits, Blocktype shapeType,
			 int* fillIdx, struct _line** linesPtr)
{
	struct _line* lines = *linesPtr;
	int type = readBits(r, 1);

	if ( type == 0 ) /* state change */
	{
		int flags = readBits(r, 5);

		/* XXX - if record looks like:

		MoveTo (1) - (0,0)
FillStyle0: 0
FillStyle1: 0
LineStyle1: 0
		
		it indicates a completely new shape that may overlap the old one.. */
		

#define SHAPEREC_NEWSTYLES  (1<<4)
#define SHAPEREC_LINESTYLE  (1<<3)
#define SHAPEREC_FILLSTYLE1 (1<<2)
#define SHAPEREC_FILLSTYLE0 (1<<1)
#define SHAPEREC_MOVETO     (1<<0)

		if ( flags == 0 )
			return 0;

		if ( flags & SHAPEREC_MOVETO )
		{
			int moveBits = readBits(r, 5);

			r->x = readSBits(r, moveBits);
			r->y = readSBits(r, moveBits);
			
			if ( r->x == 0 && r->y == 0 &&
		(flags & SHAPEREC_FILLSTYLE0) && (flags & SHAPEREC_FILLSTYLE1) && (flags & SHAPEREC_LINESTYLE) )
				ddShape_newPath(shape);
			else
				ddShape_moveTo(shape, FIXED_T(r->x), FIXED_T(r->y));
		}

		if ( flags & SHAPEREC_FILLSTYLE0 )
			ddShape_setLeftFill(shape, *fillIdx, readBits(r, *fillBits));

		if ( flags & SHAPEREC_FILLSTYLE1 )
			ddShape_setRightFill(shape, *fillIdx, readBits(r, *fillBits));

		if ( flags & SHAPEREC_LINESTYLE )
		{
			int line = readBits(r, *lineBits);

			if ( line == 0 )
				ddShape_setLine(shape, 0, 0);
			else
				ddShape_setLine(shape, lines[line-1].width, lines[line-1].color);
		}

		if ( flags & SHAPEREC_NEWSTYLES )
		{
			*fillIdx = ddShape_getNFills(shape);
			readFillStyleArray(p, r, shapeType, shape, NULL);
			readLineStyleArray(r, shapeType, linesPtr, NULL);
			*fillBits = readBits(r, 4);
			*lineBits = readBits(r, 4);
		}
	}
	else /* it's an edge record */
	{
		int straight = readBits(r, 1);
		int numBits = readBits(r, 4) + 2;

		if ( straight == 1 )
		{
			if ( readBits(r, 1) ) /* general line */
			{
				r->x += readSBits(r, numBits);
				r->y += readSBits(r, numBits);
			}
			else
			{
				if ( readBits(r, 1) ) /* vert = 1 */
					r->y += readSBits(r, numBits);
				else
					r->x += readSBits(r, numBits);
			}

			ddShape_lineTo(shape, FIXED_T(r->x), FIXED_T(r->y));
		}
		else
		{
			int x1 = readSBits(r, numBits);
			int y1 = readSBits(r, numBits);
			int x2 = readSBits(r, numBits);
			int y2 = readSBits(r, numBits);

			ddShape_curveTo(shape,
				   FIXED_T(r->x + x1), FIXED_T(r->y + y1),
				   FIXED_T(r->x + x1 + x2), FIXED_T(r->y + y1 + y2));

			r->x += x1 + x2;
			r->y += y1 + y2;
		}
	}

	return 1;
}


void
readDefineShape3(ddMovieClip* p, ddReader* r, int length)
{
	struct _line* lines = NULL;

	int start = ddReader_getOffset(r);
	int fillBits, lineBits;

	int fillIdx; /* current starting index for fill numbers-
		incremented when we get new fills */

	int shapeID = readUInt16(r);
	ddRect bounds;
	
	ddShape* shape = dd_newShape();

	r->x = 0;
	r->y = 0;

	bounds = readRect(r);

	readFillStyleArray(p, r, DEFINESHAPE3, shape, NULL);
	readLineStyleArray(r, DEFINESHAPE3, &lines, NULL);

	byteAlign(r);

	fillBits = readBits(r, 4);
	lineBits = readBits(r, 4);
	fillIdx = 0;

	while ( ddReader_getOffset(r) < start+length )
	{
		if ( readShapeRec(p, r, shape, &lineBits, &fillBits, DEFINESHAPE3, &fillIdx, &lines) == 0 )
			break;
	}

	ddShape_end(shape);
	ddShape_setBounds(shape, bounds);

	if ( lines != NULL )
		dd_free(lines);

	ddMovieClip_addCharacter(p, shapeID,
						  (ddCharacter*)dd_newShapeCharacter(shape));
}


void
readDefineShape2(ddMovieClip* p, ddReader* r, int length)
{
	struct _line* lines = NULL;

	int start = ddReader_getOffset(r);
	int fillBits, lineBits, fillIdx;
	int shapeID = readUInt16(r);
	ddRect bounds;

	ddShape* shape = dd_newShape();

	r->x = 0;
	r->y = 0;

	bounds = readRect(r);

	readFillStyleArray(p, r, DEFINESHAPE2, shape, NULL);
	readLineStyleArray(r, DEFINESHAPE2, &lines, NULL);

	byteAlign(r);

	fillBits = readBits(r, 4);
	lineBits = readBits(r, 4);
	fillIdx = 0;

	while ( ddReader_getOffset(r) < start+length )
	{
		if ( readShapeRec(p, r, shape, &lineBits, &fillBits, DEFINESHAPE2, &fillIdx, &lines) == 0 )
			break;
	}

	ddShape_end(shape);
	ddShape_setBounds(shape, bounds);

	if ( lines != NULL )
		dd_free(lines);

	ddMovieClip_addCharacter(p, shapeID, (ddCharacter*)dd_newShapeCharacter(shape));
}


void
readDefineShape(ddMovieClip* p, ddReader* r, int length)
{
	struct _line* lines = NULL;

	int start = ddReader_getOffset(r);
	int fillBits, lineBits, fillIdx;
	int shapeID = readUInt16(r);
	ddRect bounds;

	ddShape* shape = dd_newShape();

	r->x = 0;
	r->y = 0;
	
	bounds = readRect(r);

	readFillStyleArray(p, r, DEFINESHAPE, shape, NULL);
	readLineStyleArray(r, DEFINESHAPE, &lines, NULL);

	byteAlign(r);

	fillBits = readBits(r, 4);
	lineBits = readBits(r, 4);
	fillIdx = 0;

	while ( ddReader_getOffset(r) < start + length )
	{
		if ( readShapeRec(p, r, shape, &lineBits, &fillBits, DEFINESHAPE, &fillIdx, &lines) == 0 )
			break;
	}

	ddShape_end(shape);
	ddShape_setBounds(shape, bounds);

	if ( lines != NULL )
		dd_free(lines);

	ddMovieClip_addCharacter(p, shapeID, (ddCharacter*)dd_newShapeCharacter(shape));
}


int
readMorphShapeRec(ddMovieClip* p, ddReader* r, ddShape* shape1,
				  ddShape* shape2, int* lineBits, int* fillBits,
				  int* fillIdx, struct _line** lines1Ptr, struct _line** lines2Ptr)
{
	struct _line* lines1 = *lines1Ptr;
	struct _line* lines2 = *lines2Ptr;
		
	int type = readBits(r, 1);

	if ( type == 0 ) /* state change */
	{
		int flags = readBits(r, 5);

		if ( flags == 0 )
			return 0;

		if ( flags & SHAPEREC_MOVETO )
		{
			int moveBits = readBits(r, 5);

			r->x = readSBits(r, moveBits);
			r->y = readSBits(r, moveBits);
			ddShape_moveTo(shape1, FIXED_T(r->x), FIXED_T(r->y));
			ddShape_moveTo(shape2, FIXED_T(r->x), FIXED_T(r->y));
		}

		if ( flags & SHAPEREC_FILLSTYLE1 ) /* note backwards fills.. */
		{
			int fill = readBits(r, *fillBits);
			ddShape_setLeftFill(shape1, *fillIdx, fill);
			ddShape_setLeftFill(shape2, *fillIdx, fill);
		}

		if ( flags & SHAPEREC_FILLSTYLE0 )
		{
			int fill = readBits(r, *fillBits);
			ddShape_setRightFill(shape1, *fillIdx, fill);
			ddShape_setRightFill(shape2, *fillIdx, fill);
		}

		if ( flags & SHAPEREC_LINESTYLE )
		{
			int line = readBits(r, *lineBits);

			if ( line == 0 )
			{
				ddShape_setLine(shape1, 0, 0);
				ddShape_setLine(shape2, 0, 0);
			}
			else
			{
				ddShape_setLine(shape1, lines1[line-1].width, lines1[line-1].color);
				ddShape_setLine(shape2, lines2[line-1].width, lines2[line-1].color);
			}
		}

		if ( flags & SHAPEREC_NEWSTYLES )
		{
			*fillIdx = ddShape_getNFills(shape1);
			readFillStyleArray(p, r, DEFINEMORPHSHAPE, shape1, shape2);
			readLineStyleArray(r, DEFINEMORPHSHAPE, lines1Ptr, lines2Ptr);
			*fillBits = readBits(r, 4);
			*lineBits = readBits(r, 4);
		}
	}
	else /* it's an edge record */
	{
		int straight = readBits(r, 1);
		int numBits = readBits(r, 4) + 2;

		if ( straight == 1 )
		{
			if ( readBits(r, 1) ) /* general line */
			{
				r->x += readSBits(r, numBits);
				r->y += readSBits(r, numBits);
			}
			else
			{
				int vert = readBits(r,1);

				if ( vert )
					r->y += readSBits(r, numBits);
				else
					r->x += readSBits(r, numBits);
			}

			ddShape_lineTo(shape1, FIXED_T(r->x), FIXED_T(r->y));
			ddShape_lineTo(shape2, FIXED_T(r->x), FIXED_T(r->y));
		}
		else
		{
			int x1 = readSBits(r, numBits);
			int y1 = readSBits(r, numBits);
			int x2 = readSBits(r, numBits);
			int y2 = readSBits(r, numBits);

			ddShape_curveTo(shape1,
				   FIXED_T(r->x + x1), FIXED_T(r->y + y1),
				   FIXED_T(r->x + x1 + x2), FIXED_T(r->y + y1 + y2));

			ddShape_curveTo(shape2,
				   FIXED_T(r->x + x1), FIXED_T(r->y + y1),
				   FIXED_T(r->x + x1 + x2), FIXED_T(r->y + y1 + y2));

			r->x += x1 + x2;
			r->y += y1 + y2;
		}
	}

	return 1;
}


int
readMorphEndShapeRec(ddMovieClip* p, ddReader* r, ddShape* shape, int* recIdx)
{
	int type;

	while ( shape->records[*recIdx].type != SHAPERECORD_MOVETO &&
		 shape->records[*recIdx].type != SHAPERECORD_LINETO &&
		 shape->records[*recIdx].type != SHAPERECORD_END &&
		 shape->records[*recIdx].type != SHAPERECORD_CURVETO )
	{
		++*recIdx;
	}

	type = readBits(r, 1);

	if ( type == 0 ) /* state change */
	{
		int flags = readBits(r, 5);

		if ( flags == 0 )
			return 0;

		if ( flags & SHAPEREC_MOVETO )
		{
			int moveBits = readBits(r, 5);
			r->x = readSBits(r, moveBits);
			r->y = readSBits(r, moveBits);
			shape->records[*recIdx].data.coords.x1 = FIXED_T(r->x);
			shape->records[*recIdx].data.coords.y1 = FIXED_T(r->y);
		}

		if ( flags & SHAPEREC_FILLSTYLE0 || flags & SHAPEREC_FILLSTYLE1 ||
			 flags & SHAPEREC_LINESTYLE || flags & SHAPEREC_NEWSTYLES )
		{
			dd_error("Unexpected records in morph end shape!");
		}
	}
	else /* it's an edge record */
	{
		int straight = readBits(r, 1);
		int numBits = readBits(r, 4) + 2;

		if ( straight == 1 )
		{
			if ( readBits(r, 1) ) /* general line */
			{
				r->x += readSBits(r, numBits);
				r->y += readSBits(r, numBits);
			}
			else
			{
				if ( readBits(r, 1) ) /* vert = 1 */
					r->y += readSBits(r, numBits);
				else
					r->x += readSBits(r, numBits);
			}

			shape->records[*recIdx].data.coords.x1 = FIXED_T(r->x);
			shape->records[*recIdx].data.coords.y1 = FIXED_T(r->y);
		}
		else
		{
			r->x += readSBits(r, numBits);
			r->y += readSBits(r, numBits);
			
			shape->records[*recIdx].data.coords.x1 = FIXED_T(r->x);
			shape->records[*recIdx].data.coords.y1 = FIXED_T(r->y);
			
			r->x += readSBits(r, numBits);
			r->y += readSBits(r, numBits);
			
			shape->records[*recIdx].data.coords.x2 = FIXED_T(r->x);
			shape->records[*recIdx].data.coords.y2 = FIXED_T(r->y);
		}
	}

	++*recIdx;
	
	return 1;
}


void
readDefineMorphShape(ddMovieClip* p, ddReader* r, int length)
{
	struct _line* lines1 = NULL;
	struct _line* lines2 = NULL;

	int offset, start = ddReader_getOffset(r);
	int fillBits, lineBits, here;
	int idx;

	ddShape* shape1 = dd_newShape();
	ddShape* shape2 = dd_newShape();

	int shapeID = readUInt16(r);

	ddRect rect = readRect(r); /* bounds 1 */
	
	r->x = 0;
	r->y = 0;
	
	if ( rect.left == 0 && rect.right == 0 &&
		 rect.top == 0 && rect.bottom == 0 )
	{
		skipBytes(r, start + length - ddReader_getOffset(r));
		return;
	}

	rect = readRect(r); /* bounds 2 */

	if ( rect.left == 0 && rect.right == 0 &&
		 rect.top == 0 && rect.bottom == 0 )
	{
		skipBytes(r, start + length - ddReader_getOffset(r));
		return;
	}

	offset = readUInt32(r);

	here = ddReader_getOffset(r);

	readFillStyleArray(p, r, DEFINEMORPHSHAPE, shape1, shape2);
	readLineStyleArray(r, DEFINEMORPHSHAPE, &lines1, &lines2);

	fillBits = readBits(r, 4);
	lineBits = readBits(r, 4);

	idx = 0;

	while ( ddReader_getOffset(r) < here+offset )
	{
		readMorphShapeRec(p, r, shape1, shape2, &lineBits, &fillBits,
					&idx, &lines1, &lines2);
	}

	ddShape_end(shape1);
	ddShape_end(shape2);

	byteAlign(r);

	/* ??? */
	fillBits = readBits(r, 4);
	lineBits = readBits(r, 4);

	/* kinda sloppy, we're using idx for two different purposes.. */
	idx = 0;

	while ( ddReader_getOffset(r) < start + length )
		readMorphEndShapeRec(p, r, shape2, &idx);

	if ( lines1 != NULL )
		dd_free(lines1);

	if ( lines2 != NULL )
		dd_free(lines2);

	ddMovieClip_addCharacter(p, shapeID,
						  (ddCharacter*)dd_newMorphShape(shape1, shape2));
}
