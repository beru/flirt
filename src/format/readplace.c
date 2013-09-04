/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "read.h"
#include "readaction.h"
#include "../action/action.h"

void
readLibrarySymbol(ddMovieClip* p, ddReader* r, int length)
{
	int id;
	char* name;

	readUInt16(r); /* ??? - mystery number */

	id = readUInt16(r);
	name = readString(r);

	ddMovieClip_addLibrarySymbol(p, id, name);
}


void
readClipAction(ddMovieClip* p, ddReader* r, int length)
{
	ddTimeline_addAction(p->timeline, readAction(r, length));
}


void
readInitClipAction(ddMovieClip* p, ddReader* r, int length)
{
	int id = readUInt16(r);
	ddCharacter* clip = ddMovieClip_getCharacter(p, id);

	if ( clip == NULL )
	{
		dd_warn("Couldn't find character id %i", id);
		return;
	}
	else if ( clip->type != MOVIECLIP_CHAR )
	{
		dd_warn("Clip action character isn't a movie clip!");
		return;
	}

	ddMovieClip_addInitAction((ddMovieClip*)clip, readAction(r, length-2));
}


void
readFrameLabel(ddMovieClip* p, ddReader* r, int length)
{
	ddMovieClip_addFrameLabel(p, readString(r));
}


void
readPlaceObject(ddMovieClip* p, ddReader* r, int length)
{
	int start = ddReader_getOffset(r);

	int id = readUInt16(r);
	int depth = readUInt16(r);

	ddMatrix matrix = readMatrix(r);
	ddCXform cXform = ddEmptyCXform;

	if ( ddReader_getOffset(r) < start+length )
		cXform = readCXform(r, DD_FALSE); /* XXX - no alpha? */

	ddTimeline_placeCharacter(p->timeline, ddMovieClip_getCharacter(p, id),
						   depth, matrix, cXform, 0, -1, NULL);
}


#define PLACE_HASACTION		(1<<7)
#define PLACE_HASMASK		(1<<6)
#define PLACE_HASNAME		(1<<5)
#define PLACE_HASRATIO		(1<<4)
#define PLACE_HASCXFORM		(1<<3)
#define PLACE_HASMATRIX		(1<<2)
#define PLACE_HASCHARACTER	(1<<1)
#define PLACE_HASMOVE		(1<<0)

void
readPlaceObject2(ddMovieClip* p, ddReader* r, int length)
{
	/* int start = ddReader_getOffset(r); */
	int flags = readUInt8(r);
	int depth = readUInt16(r);

	ddTimelineItem* item = ddTimeline_findItem(p->timeline, depth);

	if ( item == NULL )
		item = ddTimeline_addItem(p->timeline, depth, p->framesLoaded);

	if ( flags & PLACE_HASMOVE )
		; /* ??? */

	/*
	 if the HASMOVE flag is set, we're moving a character that's already
	 been placed.  if HASMOVE and HASCHARACTER are set, it replaces the char..?
	 */

	if ( flags & PLACE_HASCHARACTER )
		item->character = ddMovieClip_getCharacter(p, readUInt16(r));

	if ( flags & PLACE_HASMATRIX )
		item->matrix = readMatrix(r);

	if ( flags & PLACE_HASCXFORM )
		item->cXform = readCXform(r, DD_TRUE);

	if ( flags & PLACE_HASRATIO )
		item->ratio = readUInt16(r);

	if ( flags & PLACE_HASNAME )
		item->name = readString(r);

	if ( flags & PLACE_HASMASK )
		item->maskdepth = readUInt16(r);

	if ( flags & PLACE_HASACTION )
	{
		readUInt16(r); /* mystery number - always seems to be 0 */

		/* logical OR of all clip action flags -
			we don't bother with this because clips will
			register for event callbacks */

		if ( r->version < 6 )
			flags = readUInt16(r);
		else
			flags = readUInt32(r);
		
		for ( ;; )
		{
			int i;
			ddAction* action;
			int length;

			if ( r->version < 6 )
				flags = readUInt16(r);
			else
				flags = readUInt32(r);
		
			if ( flags == 0 )
				break;
			
			length = readUInt32(r);
			action = readAction(r, length);

			for ( i = 0; i < DD_NUMEVENTHANDLERS; ++i )
			{
				if ( (flags & (1 << i)) != 0 )
				{
					if ( item->actions[i] != NULL )
						action->next = item->actions[i];

					item->actions[i] = action;
				}
			}
		}
	}
}


void
readRemoveObject(ddMovieClip* p, ddReader* r, int length)
{
	int id = readUInt16(r);
	int depth = readUInt16(r);

	ddTimeline_removeCharacter(p->timeline, ddMovieClip_getCharacter(p, id), depth);
}


void
readRemoveObject2(ddMovieClip* p, ddReader* r, int length)
{
	ddTimeline_removeItem(p->timeline, readUInt16(r));
}
