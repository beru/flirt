/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "charlist.h"
#include "drawshape.h"

ddCharacterList*
dd_newCharacterList()
{
	ddCharacterList* charList = dd_malloc(sizeof(struct _ddCharacterList));

	charList->nChars = 0;
	charList->chars = NULL;

	return charList;
}


void
dd_destroyCharacterList(ddCharacterList* list)
{
	int i;

	if ( list->chars != NULL )
	{
		for ( i = 0; i < list->nChars; ++i )
		{
			if ( list->chars[i] != NULL )
				dd_destroyCharacter(list->chars[i]);
		}

		dd_free(list->chars);
	}
	
	dd_free(list);
}


void
ddCharacterList_addCharacter(ddCharacterList* charList, int id, ddCharacter* c)
{
	int i = charList->nChars;

	if ( id >= charList->nChars )
	{
		charList->chars = dd_realloc(charList->chars, (id + 1) * sizeof(ddCharacter));
		charList->nChars = id + 1;

		while ( i < id )
			charList->chars[i++] = NULL;
	}

	charList->chars[id] = c;
}


ddCharacter*
ddCharacterList_getCharacter(ddCharacterList* charList, int id)
{
	if ( id < charList->nChars )
		return charList->chars[id];
	else
		return NULL;
}
