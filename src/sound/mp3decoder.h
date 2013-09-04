/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_SOUND_MP3DECODE_H_INCLUDED
#define DD_SOUND_MP3DECODE_H_INCLUDED

#include "../dd.h"

typedef struct _ddMP3Decoder ddMP3Decoder;

#ifdef HAVE_MAD
#include "mad.h"
#endif

#include "mixer.h"
#include "../player/sound.h"

struct _ddMP3Decoder
{
#ifdef HAVE_MAD
	struct mad_stream stream;
	struct mad_frame frame;
	struct mad_synth synth;
#else
	int fubar;
#endif
};


ddMP3Decoder*
dd_newMP3Decoder();

void
dd_destroyMP3Decoder(ddMP3Decoder* decoder);

boolean
ddMP3Decoder_decodeFrame(ddMP3Decoder* decoder, ddSoundTransform xform, unsigned char** dataPtr, int datalength, int delay, ddAudioSample** bufferPtr, int size);

#endif /* DD_SOUND_MP3DECODE_H_INCLUDED */
