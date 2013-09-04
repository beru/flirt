/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_CLASSES_KEY_H_INCLUDED
#define DD_ACTION_CLASSES_KEY_H_INCLUDED

#include "../../dd.h"

typedef struct _ddActionKeyObject ddActionKeyObject;

#include "../object.h"

struct _ddActionKeyObject
{
	ddActionObject parent;

	int nListeners;
	ddActionObject** listeners;
};


ddActionKeyObject*
dd_initKeyObject();

#define KEY_BACKSPACE 8
#define KEY_CAPSLOCK 20
#define KEY_CONTROL 17
#define KEY_DELETEKEY 46
#define KEY_DOWN 40
#define KEY_END 35
#define KEY_ENTER 13
#define KEY_ESCAPE 27
#define KEY_HOME 36
#define KEY_INSERT 45
#define KEY_LEFT 37
#define KEY_PGDN 34
#define KEY_PGUP 33
#define KEY_RIGHT 39
#define KEY_SHIFT 16
#define KEY_SPACE 32
#define KEY_TAB 9
#define KEY_UP 38

#endif /* DD_ACTION_CLASSES_KEY_H_INCLUDED */
