/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "raster.h"

ddRasterMethod
ddFillInstance_getRasterMethod(ddFillInstance* f)
{
	if ( f == NULL )
		return NULL;

	switch ( f->fill.type )
	{
		case FILL_NONE:
			return NULL;

		case FILL_SOLID:
			return solidRaster;

		case FILL_RADIAL_GRADIENT:
			return radialGradientRaster;

		case FILL_LINEAR_GRADIENT:
			return linearGradientRaster;

		case FILL_CLIPPED_BITMAP:
			return clippedBitmapRaster;

		case FILL_TILED_BITMAP:
			return tiledBitmapRaster;

		default:
			dd_warn("Unknown fill type: %i", f->fill.type);
			return NULL;
	}
}
