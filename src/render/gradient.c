/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>

#include "gradient.h"

ddGradient*
dd_newGradient()
{
	ddGradient* g = dd_malloc(sizeof(ddGradient));
	g->entries = NULL;
	g->nGrads = 0;
	g->band = 0;
	return g;
}


void
dd_destroyGradient(ddGradient* g)
{
	if ( g->entries != NULL )
		dd_free(g->entries);

	dd_free(g);
}


ddGradient*
dd_newGradient_copy(ddGradient* g)
{
	ddGradient* newg = dd_malloc(sizeof(ddGradient));
	newg->nGrads = g->nGrads;
	newg->entries = dd_malloc(newg->nGrads * sizeof(struct _ddGradEntry));

	memcpy(newg->entries, g->entries, newg->nGrads * sizeof(struct _ddGradEntry));

	newg->band = 0;
	return newg;
}


void
ddGradient_set(ddGradient* g, int idx, ddColor color, UInt8 ratio)
{
	struct _ddGradEntry* e = &(g->entries[idx]);

	e->ratio = ratio << 8;
	e->alpha = COLOR_ALPHA(color);
	e->red = COLOR_RED(color);
	e->green = COLOR_GREEN(color);
	e->blue = COLOR_BLUE(color);

	if ( idx > 0 )
	{
		/* compute slopes for last entry */

		struct _ddGradEntry *l = &(g->entries[idx-1]);

		if ( e->ratio != l->ratio )
		{
			l->alphaslope = ((e->alpha - l->alpha) << 16) / (e->ratio - l->ratio);
			l->redslope = ((e->red - l->red) << 16) / (e->ratio - l->ratio);
			l->greenslope = ((e->green - l->green) << 16) / (e->ratio - l->ratio);
			l->blueslope = ((e->blue - l->blue) << 16) / (e->ratio - l->ratio);
		}

		// (and if they ARE the same, we won't ever be looking at the slope anyway..)
	}
}


void
ddGradient_interpolate(ddGradient* g, ddGradient* g1, ddGradient* g2, UInt16 ratio)
{
	int i;
	unsigned int r1 = 0x10000 - ratio, r2 = ratio;

	for ( i = 0; i < g->nGrads; ++i )
	{
		struct _ddGradEntry *e1 = &(g1->entries[i]);
		struct _ddGradEntry *e2 = &(g2->entries[i]);

		ddColor c1 = DDCOLOR(e1->alpha, e1->red, e1->green, e1->blue);
		ddColor c2 = DDCOLOR(e2->alpha, e2->red, e2->green, e2->blue);

		ddGradient_set(g, i,
				 ddColor_scale(c1, (r1 + 127) >> 8) + ddColor_scale(c2, (r2 + 127) >> 8),
				 (r1 * e1->ratio + r2 * e2->ratio) >> 24);
	}
}


void
ddGradient_add(ddGradient* g, ddColor color, UInt8 ratio)
{
	if ( g->nGrads > 0 && ratio == g->entries[g->nGrads-1].ratio )
	{
		/* flash is sloppy here sometimes.. */
		ddGradient_set(g, g->nGrads-1, color, ratio);
		return;
	}

	g->entries = dd_realloc(g->entries, sizeof(struct _ddGradEntry) * (g->nGrads + 1));

	ddGradient_set(g, g->nGrads, color, ratio);
	++g->nGrads;
}


/* get the color for the specified ratio from the gradient. */

ddColor
ddGradient_getColor(ddGradient* g, int ratio)
{
	struct _ddGradEntry *e;

	while ( g->band > 0 && g->entries[g->band].ratio > ratio )
		--g->band;

	while ( g->band < g->nGrads-1 && g->entries[g->band+1].ratio < ratio )
		++g->band;

	e = &(g->entries[g->band]);

	if ( g->band == g->nGrads-1 )
		return DDCOLOR(e->alpha, e->red, e->green, e->blue);

	ratio -= e->ratio;

	if ( ratio <= 0 )
		return DDCOLOR(e->alpha, e->red, e->green, e->blue);
	else
		return DDCOLOR(e->alpha + ((e->alphaslope * ratio) >> 16), 
					   e->red + ((e->redslope * ratio) >> 16),
					   e->green + ((e->greenslope * ratio) >> 16),
					   e->blue + ((e->blueslope * ratio) >> 16));
}
