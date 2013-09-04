/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

/*
 new Date
 
 getDate
 getDay
 getFullYear
 getHours
 getMilliseconds
 getMinutes
 getMonth
 getSeconds
 getTime
 getTimezoneOffset
 getUTCDate
 getUTCDay
 getUTCFullYear
 getUTCHours
 getUTCMilliseconds
 getUTCMinutes
 getUTCMonth
 getUTCSeconds
 getYear
 setDate
 setFullYear
 setHours
 setMilliseconds
 setMinutes
 setMonth
 setSeconds
 setTime
 setUTCDate
 setUTCFullYear
 setUTCHours
 setUTCMilliseconds
 setUTCMinutes
 setUTCMonth
 setUTCSeconds
 setYear
 toString
 UTC
 */

#ifndef DD_ACTION_CLASSES_DATE_H_INCLUDED
#define DD_ACTION_CLASSES_DATE_H_INCLUDED

typedef struct _ddActionDate ddActionDate;

#include "../../dd.h"
#include "../value.h"
#include "../class.h"
#include "../object.h"

#include <sys/time.h>

struct _ddActionDate
{
	ddActionObject parent;
	struct timeval* time;
};

extern ddActionClass* ddActionDateClass;

ddActionDate*
dd_newActionDate();

ddActionDate*
dd_newActionDate_args(int year, int month, int date, int hour, int minute, int second, int millis);

ddActionFunction*
dd_initDateClass();

#endif /* DD_ACTION_CLASSES_DATE_H_INCLUDED */
