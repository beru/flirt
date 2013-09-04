/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_RENDER_GRADIENT_H_INCLUDED
#define DD_RENDER_GRADIENT_H_INCLUDED

#include "../dd.h"

typedef struct _ddGradient ddGradient;

struct _ddGradEntry
{
	UInt16 ratio;
	UInt8 red;
	UInt8 green;
	UInt8 blue;
	UInt8 alpha;
	int redslope; /* scaled so that we can do x*slope>>shift instead of dividing */
	int greenslope;
	int blueslope;
	int alphaslope;
};

struct _ddGradient
{
	struct _ddGradEntry *entries;
	int nGrads;

	/* since we're usually looking for nearby ratios,
		we cache the band index for the found ratio */

	int band;
};


ddGradient*
dd_newGradient();

ddGradient*
dd_newGradient_copy(ddGradient* g);

void
dd_destroyGradient(ddGradient* g);

void
ddGradient_interpolate(ddGradient* g, ddGradient* g1, ddGradient* g2, UInt16 ratio);

void
ddGradient_add(ddGradient* g, ddColor color, UInt8 ratio);

ddColor
ddGradient_getColor(ddGradient* g, int ratio);

#endif /* DD_RENDER_GRADIENT_H_INCLUDED */
