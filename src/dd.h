/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_H_INCLUDED
#define DD_H_INCLUDED

#include <stdlib.h>

#include "config.h"
#include "types.h"

#ifdef DD_LOG
#include <stdio.h>
extern FILE* logfile;
void dd_log(const char* msg, ...);
#else
static inline void dd_log() {}
#endif

#define dd_malloc(l)     malloc(l)
#define dd_realloc(p,l)  realloc((p),(l))
#define dd_free(p)       free(p)

extern long dd_startTime;
extern void (*dd_error)(const char* msg, ...);
extern void (*dd_warn)(const char* msg, ...);

boolean
dd_init();

void
dd_setErrorFunction(void (*error)(const char* msg, ...));

void
dd_setWarnFunction(void (*warn)(const char* msg, ...));

unsigned long dd_getTicks();

#endif /* DD_H_INCLUDED */
