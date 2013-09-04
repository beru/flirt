/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "../dd.h"

#include "../player/movieclip.h"
#include "read.h"

void
readDefineBitsLossless(ddMovieClip* player, ddReader* r, int length, int version);
