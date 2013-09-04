/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "readbutton.h"
#include "readaction.h"
#include "readsound.h"
#include "../player/button.h"

int
readButtonRecord(ddMovieClip* p, ddReader* r, ddButton* b, int recordType)
{
	int character, layer;
	int flags = readUInt8(r);
	ddMatrix matrix;
	ddCXform cXform;

	if ( flags == 0 )
		return 0;

	character = readUInt16(r);
	layer = readUInt16(r);
	matrix = readMatrix(r);

	if ( recordType == 2 )
		cXform = readCXform(r, DD_TRUE); /* XXX - should be true? */

	ddButton_addCharacter(b, ddMovieClip_getCharacter(p, character),
					   layer, matrix, cXform, flags);

	return 1;
}


void
readDefineButton(ddMovieClip* p, ddReader* r, int length)
{
	int offset = ddReader_getOffset(r);
	int buttonID = readUInt16(r);
	int alen;

	ddButton* button = dd_newButton();
	button->flags = 0;

	while ( readButtonRecord(p, r, button, 1) )
		;

	alen = length - (ddReader_getOffset(r) - offset);

	ddButton_addAction(button, readAction(r, alen), DDBUTTON_OVERDOWNTOOVERUP);

	ddMovieClip_addCharacter(p, buttonID, (ddCharacter*)button);
}


int
readButton2ActionCondition(ddButton* b, ddReader* r, int end)
{
	int offset = readUInt16(r);
	int flags = readUInt16(r);
	int len;

	if ( offset == 0 )
		len = end - ddReader_getOffset(r);
	else
		len = offset - 4;

	ddButton_addAction(b, readAction(r, len), flags);

	return offset;
}


void
readDefineButtonSound(ddMovieClip* p, ddReader* r, int length)
{
	int buttonID = readUInt16(r);
	ddButton* button = (ddButton*)ddMovieClip_getCharacter(p, buttonID);

	int overUpToIdleSoundID;
	int idleToOverUpSoundID;
	int overUpToOverDownSoundID;
	int overDownToOverUpSoundID;
	
	if ( button == NULL )
	{
		dd_warn("readDefineButtonSound(): button %i not defined\n", buttonID);
		return;
	}

	if ( button->parent.type != BUTTON_CHAR )
	{
		dd_warn("readDefineButtonSound(): button %i not defined\n", buttonID);
		return;
	}
	
	overUpToIdleSoundID = readUInt16(r);
	
	if ( overUpToIdleSoundID != 0 )
	{
		ddSoundInstance* sound = readSoundInfo(r, overUpToIdleSoundID);
		
		if ( sound != NULL )
			ddButton_addSoundInstance(button, (ddSoundInstance*)sound, DDBUTTON_OVERUPTOIDLE);
	}
	
	idleToOverUpSoundID = readUInt16(r);
	
	if ( idleToOverUpSoundID != 0 )
	{
		ddSoundInstance* sound = readSoundInfo(r, idleToOverUpSoundID);
		
		if ( sound != NULL )
			ddButton_addSoundInstance(button, (ddSoundInstance*)sound, DDBUTTON_IDLETOOVERUP);
	}
	
	overUpToOverDownSoundID = readUInt16(r);

	if ( overUpToOverDownSoundID != 0 )
	{
		ddSoundInstance* sound = readSoundInfo(r, overUpToOverDownSoundID);
		
		if ( sound != NULL )
			ddButton_addSoundInstance(button, (ddSoundInstance*)sound, DDBUTTON_OVERUPTOOVERDOWN);
	}
	
	overDownToOverUpSoundID = readUInt16(r);

	if ( overDownToOverUpSoundID != 0 )
	{
		ddSoundInstance* sound = readSoundInfo(r, overDownToOverUpSoundID);
		
		if ( sound != NULL )
			ddButton_addSoundInstance(button, (ddSoundInstance*)sound, DDBUTTON_OVERDOWNTOOVERUP);
	}
}


void
readDefineButton2(ddMovieClip* p, ddReader* r, int length)
{
	int offset, end = ddReader_getOffset(r) + length;

	ddButton* button = dd_newButton();

	int buttonID = readUInt16(r);
	button->flags = readUInt8(r);

	offset = readUInt16(r);

	while ( readButtonRecord(p, r, button, 2) )
		;

	if ( offset > 0 )
	{
		while ( readButton2ActionCondition(button, r, end) )
			;
	}

	ddMovieClip_addCharacter(p, buttonID, (ddCharacter*)button);
}
