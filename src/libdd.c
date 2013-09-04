/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <stdio.h>
#include <stdarg.h>

#include <sys/time.h>
#include <sys/timeb.h>

#include "dd.h"

#ifdef DD_LOG
FILE* logfile = NULL;
static int enable_log = 0;
#endif

long dd_startTime = 0;

boolean
dd_init()
{
#ifdef DD_LOG
	logfile = fopen("/tmp/dd.log", "w");

	if ( logfile == NULL )
		return DD_FALSE;

	setlinebuf(logfile);
#endif

	return DD_TRUE;
}


unsigned long
dd_getTicks()
{
	struct timeval t;
	gettimeofday(&t, NULL);

	return (t.tv_sec - dd_startTime) * 1000 + t.tv_usec / 1000;
}

#ifdef DD_LOG
void
dd_log(const char* msg, ...)
{
	if ( enable_log == 0 )
		return;
	
	va_list args;
	va_start(args, msg);
	vfprintf(logfile, msg, args);
	fprintf(logfile, "\n");
	va_end(args);
}
#endif

static void dd_error_default(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	putchar('\n');
	va_end(args);
	exit(-1);
}

static void dd_warn_default(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	putchar('\n');
	va_end(args);
}

void (*dd_error)(const char* msg, ...) = dd_error_default;
void (*dd_warn)(const char* msg, ...) = dd_warn_default;

void dd_setErrorFunction(void (*error)(const char* msg, ...))
{
	dd_error = error;
}

void dd_setWarnFunction(void (*warn)(const char* msg, ...))
{
	dd_warn = warn;
}
