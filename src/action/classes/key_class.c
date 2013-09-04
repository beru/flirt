/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

/*
 BACKSPACE
 CAPSLOCK
 CONTROL
 DELETEKEY
 DOWN
 END
 ENTER
 ESCAPE
 HOME
 INSERT
 LEFT
 PGDN
 PGUP
 RIGHT
 SHIFT
 SPACE
 TAB
 UP

 getAscii
 getCode
 isDown
 isToggled
 addListener(obj) - calls back to obj.onKeyDown, obj.onKeyUp
 removeListener(obj)
 */

#include <string.h>

#include "key_class.h"

#ifdef DD_PLATFORM_MAC_OS_X
#include <Carbon/Carbon.h>
#endif

static ddActionValue
Key_isDown(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionContext_popValue(context);
	int keycode = ddActionValue_getIntValue(val);
	boolean isDown = DD_FALSE;

#ifdef DD_PLATFORM_MAC_OS_X
	KeyMap map;
	GetKeys(map);

	if ( --keycode < 128 )
	{
		unsigned long mask = map[keycode / 32];
		keycode %= 32;
		isDown = ((mask & (1 << (31 - (keycode % 32)))) != 0);
	}
#endif

	return isDown ? ddTrueValue : ddFalseValue;
}


static ddActionValue
Key_getAscii(ddActionObject* object, ddActionContext* context, int nargs)
{
	UInt8 key = 0;

#ifdef DD_PLATFORM_MAC_OS_X
	key = LMGetKbdLast();
#endif

	return dd_newActionValue_int(key);
}


static ddActionValue
Key_getCode(ddActionObject* object, ddActionContext* context, int nargs)
{
	UInt8 key = 0;

#ifdef DD_PLATFORM_MAC_OS_X
	key = LMGetKbdLast();
#endif
	
	return dd_newActionValue_int(key);
}


static ddActionValue
Key_isToggled(ddActionObject* object, ddActionContext* context, int nargs)
{
	// XXX
	return ddFalseValue;
}


static ddActionValue
Key_addListener(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionKeyObject* key = (ddActionKeyObject*)object;
	ddActionValue val = ddActionContext_popValue(context);
	
	if ( val.type == VALUE_OBJECT )
	{
		// XXX - look for empty slot, also avoid duplicates
		
		key->listeners = dd_realloc(key->listeners, (key->nListeners + 1) * sizeof(ddActionObject*));
		key->listeners[key->nListeners] = ddActionValue_getObjectValue(val);
	}
	
	return ddNullValue;
}


static ddActionValue
Key_removeListener(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionKeyObject* key = (ddActionKeyObject*)object;
	ddActionValue val = ddActionContext_popValue(context);
	ddActionObject* obj;
	int i;

	if ( val.type != VALUE_OBJECT )
		return ddNullValue;

	obj = val.data.objectValue;

	for ( i = 0; i < key->nListeners; ++i )
	{
		if ( key->listeners[i] == obj )
			key->listeners[i] = NULL;

		ddActionObject_release(obj);
	}

	return ddNullValue;
}


ddActionKeyObject*
dd_initKeyObject()
{
	ddActionKeyObject* key = dd_malloc(sizeof(ddActionKeyObject));
	ddActionObject* obj = (ddActionObject*)key;

	ddActionObject_init(obj);

	key->nListeners = 0;
	key->listeners = NULL;
	
	ddActionObject_addNativeMethod(obj, strdup("getAscii"), Key_getAscii, 1);
	ddActionObject_addNativeMethod(obj, strdup("getCode"), Key_getCode, 1);
	ddActionObject_addNativeMethod(obj, strdup("isDown"), Key_isDown, 1);
	ddActionObject_addNativeMethod(obj, strdup("isToggled"), Key_isToggled, 1);
	ddActionObject_addNativeMethod(obj, strdup("addListener"), Key_addListener, 1);
	ddActionObject_addNativeMethod(obj, strdup("removeListener"), Key_removeListener, 1);

	ddActionHash_setValue(obj->properties, strdup("BACKSPACE"), dd_newActionValue_int(KEY_BACKSPACE));
	ddActionHash_setValue(obj->properties, strdup("CAPSLOCK"), dd_newActionValue_int(KEY_CAPSLOCK));
	ddActionHash_setValue(obj->properties, strdup("CONTROL"), dd_newActionValue_int(KEY_CONTROL));
	ddActionHash_setValue(obj->properties, strdup("DELETEKEY"), dd_newActionValue_int(KEY_DELETEKEY));
	ddActionHash_setValue(obj->properties, strdup("DOWN"), dd_newActionValue_int(KEY_DOWN));
	ddActionHash_setValue(obj->properties, strdup("END"), dd_newActionValue_int(KEY_END));
	ddActionHash_setValue(obj->properties, strdup("ENTER"), dd_newActionValue_int(KEY_ENTER));
	ddActionHash_setValue(obj->properties, strdup("ESCAPE"), dd_newActionValue_int(KEY_ESCAPE));
	ddActionHash_setValue(obj->properties, strdup("HOME"), dd_newActionValue_int(KEY_HOME));
	ddActionHash_setValue(obj->properties, strdup("INSERT"), dd_newActionValue_int(KEY_INSERT));
	ddActionHash_setValue(obj->properties, strdup("LEFT"), dd_newActionValue_int(KEY_LEFT));
	ddActionHash_setValue(obj->properties, strdup("PGDN"), dd_newActionValue_int(KEY_PGDN));
	ddActionHash_setValue(obj->properties, strdup("PGUP"), dd_newActionValue_int(KEY_PGUP));
	ddActionHash_setValue(obj->properties, strdup("RIGHT"), dd_newActionValue_int(KEY_RIGHT));
	ddActionHash_setValue(obj->properties, strdup("SHIFT"), dd_newActionValue_int(KEY_SHIFT));
	ddActionHash_setValue(obj->properties, strdup("SPACE"), dd_newActionValue_int(KEY_SPACE));
	ddActionHash_setValue(obj->properties, strdup("TAB"), dd_newActionValue_int(KEY_TAB));
	ddActionHash_setValue(obj->properties, strdup("UP"), dd_newActionValue_int(KEY_UP));

	return key;
}
