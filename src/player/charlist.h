/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_PLAYER_CHARLIST_H_INCLUDED
#define DD_PLAYER_CHARLIST_H_INCLUDED

/* each player instance has a character list.
   anything in the movie that refers to characters- movie clips, buttons, etc-
   gets a pointer to the character list. */

#include "../dd.h"

typedef struct _ddCharacterList ddCharacterList;

#include "character.h"

struct _ddCharacterList
{
	int nChars;
	ddCharacter** chars;

	/* name list for symbols.. */
};

ddCharacterList* dd_newCharacterList();
void dd_destroyCharacterList(ddCharacterList* list);

void ddCharacterList_addCharacter(ddCharacterList* charList,
								  int id, ddCharacter* c);

ddCharacter* ddCharacterList_getCharacter(ddCharacterList* charList, int id);

#endif /* DD_PLAYER_CHARLIST_H_INCLUDED */
