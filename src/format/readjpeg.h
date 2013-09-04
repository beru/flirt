/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_FORMAT_READJPEG_H_INCLUDED
#define DD_FORMAT_READJPEG_H_INCLUDED

#include "../dd.h"

#include "../player/movieclip.h"
#include "read.h"

void readJpegTables(ddMovieClip* player, ddReader* r, int length);
void readDefineBitsJpeg(ddMovieClip* player, ddReader* r, int length);
void readDefineBitsJpeg2(ddMovieClip* player, ddReader* r, int length);
void readDefineBitsJpeg3(ddMovieClip* player, ddReader* r, int length);

#endif /* DD_FORMAT_READJPEG_H_INCLUDED */
