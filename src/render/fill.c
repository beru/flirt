/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <stdlib.h>
#include <string.h>

#include "fill.h"

ddFill
dd_newSolidFill(ddColor color)
{
	ddFill f;
	f.type = FILL_SOLID;
	f.data.color = color;
	return f;
}


static ddFill
dd_newGradientFill(ddFillType type, ddGradient* gradient, ddMatrix matrix)
{
	ddFill f;

	f.type = type;
	f.data.gradient.gradient = gradient;
	f.data.gradient.matrix = matrix;

	return f;
}


ddFill
dd_newLinearGradientFill(ddGradient* gradient, ddMatrix matrix)
{
	return dd_newGradientFill(FILL_LINEAR_GRADIENT, gradient, matrix);
}


ddFill
dd_newRadialGradientFill(ddGradient* gradient, ddMatrix matrix)
{
	return dd_newGradientFill(FILL_RADIAL_GRADIENT, gradient, matrix);
}


static ddFill
dd_newBitmapFill(ddFillType type, ddImage* image, ddMatrix matrix)
{
	ddFill f;

	f.type = type;
	f.data.bitmap.image = image;
	f.data.bitmap.matrix = matrix;

	return f;
}


ddFill
dd_newTiledBitmapFill(ddImage* image, ddMatrix matrix)
{
	return dd_newBitmapFill(FILL_TILED_BITMAP, image, matrix);
}


ddFill
dd_newClippedBitmapFill(ddImage* image, ddMatrix matrix)
{
	return dd_newBitmapFill(FILL_CLIPPED_BITMAP, image, matrix);
}


ddFill
ddFill_copy(ddFill fill)
{
	if ( fill.type == FILL_RADIAL_GRADIENT || fill.type == FILL_LINEAR_GRADIENT )
		fill.data.gradient.gradient = dd_newGradient_copy(fill.data.gradient.gradient);

	return fill;
}


void
dd_destroyFill(ddFill* fill)
{
	if ( fill->type == FILL_RADIAL_GRADIENT || fill->type == FILL_LINEAR_GRADIENT )
		dd_destroyGradient(fill->data.gradient.gradient);
}


void
ddFill_interpolate(ddFill* fill, ddFill* fill1, ddFill* fill2, UInt16 ratio)
{
	if ( fill1->type != fill2->type )
	{
		dd_warn("mismatched fill types in ddFill_interpolate");
		return;
	}

	switch ( fill1->type )
	{
		case FILL_NONE:
			break;

		case FILL_SOLID:
		{
			fill->data.color =
				ddColor_scale(fill1->data.color, (0x10000 - ratio) >> 8) +
				ddColor_scale(fill2->data.color, ratio >> 8);

			break;
		}
		case FILL_RADIAL_GRADIENT:
		case FILL_LINEAR_GRADIENT:
		{
			ddGradient_interpolate(fill->data.gradient.gradient,
						  fill1->data.gradient.gradient,
						  fill2->data.gradient.gradient, ratio);

			fill->data.gradient.matrix =
				ddMatrix_interpolate(fill1->data.gradient.matrix,
						 fill2->data.gradient.matrix, ratio);
			break;
		}
		case FILL_CLIPPED_BITMAP:
		case FILL_TILED_BITMAP:
		{
			fill->data.bitmap.matrix =
				ddMatrix_interpolate(fill1->data.bitmap.matrix,
						fill2->data.bitmap.matrix, ratio);
			break;
		}
		default:
			dd_error("unknown fill type in ddFill_interpolate: %i", fill1->type);
	}
}


ddFillInstance
dd_newFillInstance(ddFill fill)
{
	ddFillInstance i;
	i.fill = fill;
	i.inverse = ddEmptyMatrix;
	i.left = ALPHABUF_SIZE;
	i.right = -1;
	i.alphaBuffer = dd_malloc(sizeof(UInt32) * ALPHABUF_SIZE);
	memset(i.alphaBuffer, 0, sizeof(UInt32) * ALPHABUF_SIZE);
	i.size = ALPHABUF_SIZE;
	i.next = NULL;
	i.listed = DD_FALSE;
	return i;
}


void
dd_destroyFillInstance(ddFillInstance* fill)
{
	if ( fill->alphaBuffer != NULL )
		dd_free(fill->alphaBuffer);
}
