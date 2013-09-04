/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_FORMAT_READBUTTON_H_INCLUDED
#define DD_FORMAT_READBUTTON_H_INCLUDED

#include "../dd.h"

#include "read.h"

void
readDefineButton(ddMovieClip* p, ddReader* r, int length);

void
readDefineButton2(ddMovieClip* p, ddReader* r, int length);

void
readDefineButtonSound(ddMovieClip* p, ddReader* r, int length);

#endif /* DD_FORMAT_READBUTTON_H_INCLUDED */
