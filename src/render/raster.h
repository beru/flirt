/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_RENDER_RASTER_H_INCLUDED
#define DD_RENDER_RASTER_H_INCLUDED

#include "../dd.h"

#include "image.h"
#include "fill.h"
#include "cxform.h"

typedef void (*ddRasterMethod)(ddColor* buffer, ddFillInstance* fill, int y, ddCXform cXform);

void
solidRaster(ddColor* buffer, ddFillInstance* fill, int y, ddCXform cXform);

void
tiledBitmapRaster(ddColor* buffer, ddFillInstance* fill, int y, ddCXform cXform);

void
clippedBitmapRaster(ddColor* buffer, ddFillInstance* fill, int y, ddCXform cXform);

void
linearGradientRaster(ddColor* buffer, ddFillInstance* fill, int y, ddCXform cXform);

void
radialGradientRaster(ddColor* buffer, ddFillInstance* fill, int y, ddCXform cXform);

ddRasterMethod
ddFillInstance_getRasterMethod(ddFillInstance* f);

#endif /* DD_RENDER_RASTER_H_INCLUDED */
