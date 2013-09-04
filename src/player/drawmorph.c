/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "drawmorph.h"

static void
ddDrawMorph_getUpdateList(ddDrawable* d, ddUpdateList* list, ddMatrix matrix)
{
	ddMatrix m;

	ddDrawMorph* morph = (ddDrawMorph*)d;

	m = ddMatrix_multiply(matrix, d->matrix);
	ddShapeInstance_getUpdateList(morph->instance, list, m);

	d->bounds = morph->instance->bounds;
}


static void
ddDrawMorph_drawInImage(ddDrawable* d, ddImage* image, ddUpdateList* list,
						ddMatrix matrix, ddCXform cXform, ddRect clipRect)
{
	ddDrawMorph* morph = (ddDrawMorph*)d;

	if ( ddUpdateList_intersectsRect(list, d->bounds) )
	{
		ddMatrix m = ddMatrix_multiply(matrix, d->matrix);
		ddCXform c = ddCXform_compose(cXform, d->cXform);
		ddShapeInstance_drawInImage(morph->instance, image, m, c, clipRect);
	}
}


static void
dd_destroyDrawMorph(ddDrawable* d)
{
	ddDrawMorph* m = (ddDrawMorph*)d;
	dd_destroyShapeInstance(m->instance);
	dd_destroyShape(m->shape);
	dd_free(d);
}


ddDrawMorph*
dd_newDrawMorph(ddMorphShape* m)
{
	ddDrawMorph* morph = dd_malloc(sizeof(ddDrawMorph));
	ddDrawable_init((ddDrawable*)morph);

	morph->parent.type = MORPH_CHAR;
	morph->parent.getUpdateList = ddDrawMorph_getUpdateList;
	morph->parent.drawInImage = ddDrawMorph_drawInImage;
	morph->parent.destroy = dd_destroyDrawMorph;
	morph->parent.bounds = ddInvalidRect;
	morph->parent.character = (ddCharacter*)m;

	morph->shape = dd_newShape_copy(m->shape1);
	morph->instance = NULL;
	morph->ratio = -1;

	return morph;
}


int
ddDrawMorph_hitTest(ddDrawMorph* m, fixed x, fixed y)
{
	if ( m->instance != NULL )
		return ddShapeInstance_hitTest(m->instance, x, y);
	else
		return DD_FALSE;
}


void
ddDrawMorph_setRatio(ddDrawMorph* morph, UInt16 ratio)
{
	ddShape* shape = morph->shape;
	ddShape* shape1 = ((ddMorphShape*)morph->parent.character)->shape1;
	ddShape* shape2 = ((ddMorphShape*)morph->parent.character)->shape2;

	int i, r1, r2, fill;

	if ( ratio == morph->ratio )
		return;

	r1 = (0x10000 - ratio) >> 6;
	r2 = ratio >> 6;

	morph->ratio = ratio;

	/* scan through shape edges, fills, lines, and interpolate */

	for ( i = 0; i < shape->nRecords; ++i )
	{
		ddShapeRecord* record = &(shape->records[i]);
		ddShapeRecord* record1 = &(shape1->records[i]);
		ddShapeRecord* record2 = &(shape2->records[i]);

		ddShapeRecordType type = record->type;

		switch( type )
		{
			case SHAPERECORD_END:
				break;

			case SHAPERECORD_MOVETO:
			case SHAPERECORD_LINETO:

				record->data.coords.x1 =
					(r1 * record1->data.coords.x1 + r2 * record2->data.coords.x1) >> 10;

				record->data.coords.y1 =
					(r1 * record1->data.coords.y1 + r2 * record2->data.coords.y1) >> 10;

				break;

			case SHAPERECORD_CURVETO:

				record->data.coords.x1 =
					(r1 * record1->data.coords.x1 + r2 * record2->data.coords.x1) >> 10;

				record->data.coords.y1 =
					(r1 * record1->data.coords.y1 + r2 * record2->data.coords.y1) >> 10;

				record->data.coords.x2 =
					(r1 * record1->data.coords.x2 + r2 * record2->data.coords.x2) >> 10;

				record->data.coords.y2 =
					(r1 * record1->data.coords.y2 + r2 * record2->data.coords.y2) >> 10;

				break;

			case SHAPERECORD_NEWFILL0:
			case SHAPERECORD_NEWFILL1:

				fill = record->data.fill;

				ddFill_interpolate(&shape->fills[fill],
					   &shape1->fills[fill], &shape2->fills[fill], ratio);
				break;

			case SHAPERECORD_NEWLINE:

				record->data.line.width =
					(r1 * record1->data.line.width + r2 * record2->data.line.width) >> 10;

				record->data.line.color =
					ddColor_scale(record1->data.line.color, r1 >> 8) +
					ddColor_scale(record2->data.line.color, r2 >> 8);

				break;

			case SHAPERECORD_NEWPATH:
				break;
			
			default:
				dd_error("Unknown shaperecord type: %i", type);
		}
	}

	if ( morph->instance != NULL )
		dd_destroyShapeInstance(morph->instance);

	morph->instance = dd_newShapeInstance(morph->shape);

	if ( morph->parent.maskShape != NULL )
		// XXX - make sure mask is a shape
		ddShapeInstance_setMaskShape(morph->instance, ((ddDrawShape*)morph->parent.maskShape)->shape);
}
