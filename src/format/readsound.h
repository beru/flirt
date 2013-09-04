/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_FORMAT_READSOUND_H_INCLUDED
#define DD_FORMAT_READSOUND_H_INCLUDED

#include "../dd.h"

#include "read.h"
#include "../player/movieclip.h"
#include "../player/sound.h"

ddSoundInstance*
readSoundInfo(ddReader* r, int characterid);

void
readDefineSound(ddMovieClip* p, ddReader* r, int length);

void
readSoundStreamHead(ddMovieClip* p, ddReader* r, int length);

void
readSoundStreamBlock(ddMovieClip* p, ddReader* r, int length);

void
readStartSound(ddMovieClip* p, ddReader* r, int length);

#endif /* DD_FORMAT_READSOUND_H_INCLUDED */
