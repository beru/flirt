/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "mp3decoder.h"

#ifdef HAVE_MAD

#include "mad.h"

ddMP3Decoder*
dd_newMP3Decoder()
{
	ddMP3Decoder* decoder = dd_malloc(sizeof(ddMP3Decoder));
	
	mad_stream_init(&decoder->stream);
	mad_frame_init(&decoder->frame);
	mad_synth_init(&decoder->synth);
	
	return decoder;
}

void
dd_destroyMP3Decoder(ddMP3Decoder* decoder)
{
	mad_synth_finish(&decoder->synth);
	mad_frame_finish(&decoder->frame);
	mad_stream_finish(&decoder->stream);

	dd_free(decoder);
}


boolean
ddMP3Decoder_decodeFrame(ddMP3Decoder* decoder, ddSoundTransform xform, unsigned char** dataPtr, int datalength, int delay, ddAudioSample** bufferPtr, int size)
{
	struct mad_pcm* pcm;
	ddAudioSample* buffer = *bufferPtr;
	unsigned char* data = *dataPtr;
	
	mad_stream_buffer(&decoder->stream, data, datalength);
	
	if ( mad_frame_decode(&decoder->frame, &decoder->stream) < 0 )
		return DD_FALSE;
	
	mad_synth_frame(&decoder->synth, &decoder->frame);
	
	pcm = &decoder->synth.pcm;
	
	int samplerate = pcm->samplerate;
	int length = pcm->length;
	
	int i = 0;
	int stereo = (pcm->channels == 2);
	
	for ( i = 0; i < length; ++i )
	{
		int left = pcm->samples[0][i];
		int right = pcm->samples[1][i];
		
		buffer->left += left * xform.leftToLeft + right * xform.rightToLeft;
		buffer->right += left * xform.leftToRight + right * xform.rightToRight;
		++buffer;
	}

	*dataPtr = (unsigned char*)decoder->stream.next_frame;
	*bufferPtr = buffer;
	
	return DD_TRUE;
}

#else

ddMP3Decoder*
dd_newMP3Decoder()
{
	return NULL;
}

void
dd_destroyMP3Decoder(ddMP3Decoder* decoder)
{
}

int
ddMP3Decoder_decodeFrame(ddMP3Decoder* decoder, unsigned char* data, int offset, ddAudioSample* buffer, int size)
{
	return 0;
}

#endif /* HAVE_MAD */
