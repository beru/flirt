/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_RENDER_FILL_H_INCLUDED
#define DD_RENDER_FILL_H_INCLUDED

#include "../dd.h"

typedef struct _ddFill ddFill;
typedef struct _ddFillInstance ddFillInstance;

#include "gradient.h"
#include "image.h"
#include "matrix.h"

typedef enum
{
	FILL_NONE,
	FILL_SOLID,
	FILL_RADIAL_GRADIENT,
	FILL_LINEAR_GRADIENT,
	FILL_CLIPPED_BITMAP,
	FILL_TILED_BITMAP
} ddFillType;

struct _ddFill
{
	ddFillType type;

	union
	{
		ddColor color;

		struct
		{
			ddImage* image;
			ddMatrix matrix;
		} bitmap;

		struct
		{
			ddGradient* gradient;
			ddMatrix matrix;
		} gradient;

	} data;
};

#define ALPHABUF_SHIFT (2*FIXED_SHIFT)
#define ALPHABUF_MAX ((1 << ALPHABUF_SHIFT) - 1)
#define ALPHABUF_SIZE 2048

struct _ddFillInstance
{
	ddFill fill;
	ddMatrix inverse;

	UInt32* alphaBuffer; // line-by-line storage of scan coverage
	
	int size;
	int left;
	int right;
	struct _ddFillInstance* next; // list for when rendering
	boolean listed;
};


ddFill
dd_newSolidFill(ddColor color);

ddFill
dd_newLinearGradientFill(ddGradient* gradient, ddMatrix matrix);

ddFill
dd_newRadialGradientFill(ddGradient* gradient, ddMatrix matrix);

ddFill
dd_newTiledBitmapFill(ddImage* bitmap, ddMatrix matrix);

ddFill
dd_newClippedBitmapFill(ddImage* bitmap, ddMatrix matrix);

ddFill
ddFill_copy(ddFill fill);

void
dd_destroyFill(ddFill* fill);

void
ddFill_interpolate(ddFill* fill, ddFill* fill1, ddFill* fill2, UInt16 ratio);

ddFillInstance
dd_newFillInstance(ddFill fill);

void
dd_destroyFillInstance(ddFillInstance* fill);

#endif /* DD_RENDER_FILL_H_INCLUDED */
