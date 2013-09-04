/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "readsprite.h"
#include "readplace.h"
#include "readsound.h"
#include "blocktypes.h"

void readDefineSprite(ddMovieClip* p, ddReader* r, int length)
{
	int start = ddReader_getOffset(r);
	int block, l;
	Blocktype type;

	ddMovieClip* clip = dd_newMovieClip(ddMovieClip_getCharList(p));

	int clipID = readUInt16(r);

	clip->framesTotal = readUInt16(r);

	while ( ddReader_getOffset(r) < start + length )
	{
		//int offset = ddReader_getOffset(r);
		int nextoffset;

		block = readUInt16(r);
		type = block>>6;

		l = block & ((1 << 6) - 1);

		if ( l == 63 ) /* it's a long block. */
			l = readUInt32(r);

		nextoffset = ddReader_getOffset(r) + l;

		switch ( type )
		{
			case PLACEOBJECT:
				readPlaceObject(clip, r, l);
				break;
				
			case PLACEOBJECT2:
				readPlaceObject2(clip, r, l);
				break;
				
			case REMOVEOBJECT:
				readRemoveObject(clip, r, l);
				break;
				
			case REMOVEOBJECT2:
				readRemoveObject2(clip, r, l);
				break;
				
			case SHOWFRAME:
				ddMovieClip_nextFrame(clip);
				//readShowFrame(clip, r, l);
				break;
				
			case FRAMELABEL:
				readFrameLabel(clip, r, l);
				break;
				
			case DOACTION:
				readClipAction(clip, r, l);
				break;
				
			case STARTSOUND:
				readStartSound(clip, r, l);
				break;
				
			case SOUNDSTREAMHEAD:
			case SOUNDSTREAMHEAD2:
				readSoundStreamHead(clip, r, l);
				break;
			
			case SOUNDSTREAMBLOCK:
				readSoundStreamBlock(clip, r, l);
				break;

			default:
			{
				if ( length > 0 )
					skipBytes(r, l);
			}
		}
		
		ddReader_setOffset(r, nextoffset);
	}

	ddMovieClip_addCharacter(p, clipID, (ddCharacter*)clip);
}
