/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_CLASSES_OBJECT_H_INCLUDED
#define DD_ACTION_CLASSES_OBJECT_H_INCLUDED

#include "../../dd.h"
#include "../value.h"
#include "../class.h"
#include "../object.h"

/* Key, Math, Mouse, and Selection are objects-
   all others are functions */

/*
  new Object
  toString
  valueOf

 SWF6-
 addProperty(name, getFunc, setFunc)
 registerClass(name, class_ctor) - identify ctor as constructor for instances of library character "name" (instead of MovieClip)
 unwatch(name)
 watch(name, func[, userdata]) - func(id, oldval, newval, userData)
*/

extern ddActionClass* ddActionObjectClass;

ddActionFunction*
dd_initObjectClass();

#endif /* DD_ACTION_CLASSES_OBJECT_H_INCLUDED */
