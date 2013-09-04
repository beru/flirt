/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_READPLACE_H_INCLUDED
#define DD_PLAYER_READPLACE_H_INCLUDED

#include "../dd.h"

#include "../player/movieclip.h"
#include "read.h"

void readLibrarySymbol(ddMovieClip* p, ddReader* r, int length);
void readClipAction(ddMovieClip* p, ddReader* r, int length);
void readInitClipAction(ddMovieClip* p, ddReader* r, int length);
void readFrameLabel(ddMovieClip* p, ddReader* r, int length);
void readPlaceObject(ddMovieClip* p, ddReader* r, int length);
void readPlaceObject2(ddMovieClip* p, ddReader* r, int length);
void readRemoveObject(ddMovieClip* p, ddReader* r, int length);
void readRemoveObject2(ddMovieClip* p, ddReader* r, int length);

#endif /* DD_PLAYER_READPLACE_H_INCLUDED */
