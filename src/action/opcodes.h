/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_OPCODES_H_INCLUDED
#define DD_ACTION_OPCODES_H_INCLUDED

typedef struct _ddActionOpcode ddActionOpcode;

#include "../dd.h"

#include "bytecodes.h"
#include "object.h"

struct _ddActionOpcode
{
	ddActionCode op;
	unsigned char data;

	char* arg;
	ddActionValue val;
	
#ifdef DD_INCLUDE_DEBUGGER
	boolean breakpoint;
#endif
};


void
dd_destroyActionOpcode(ddActionOpcode op);

ddActionOpcode*
ddAction_newOpcode(ddAction* action);

/*
 get var named: push 'a', get var => get var 'a'
 set var num: push 'a', push 0, set var => set a = 0
 set var str: push 'a', push 'foo', set var => set a = 'foo'
 get member named: push 'a', get member => get member 'a'
 set member num: push 'a', push 1, set mem => set obj.a = 1
 set member str: push 'a', push 'foo', set mem => set obj.a = 'foo'
 delete var named: push 'a', delete => delete 'a'
 local var named: push 'a', var => var 'a'
 local var assign: push 'a', push 0, var => var 'a' = 0
 new named: push 'Class', new => new 'Class'
 push value
 equals num: push 1, equals => equals 1
 less than num: push 1, less than => less than 1
 greater than num: push 1, greater than => greater than 1
 equals string: push 'a', equals => equals 'a'
 add num: push 10, add => add 10
 subtract num: push 10, subtract => subtract 10
 multiply num: push 10, multiply => multiply 10
 divide num: push 10, divide => divide 10
 modulo num: push 10, modulo => modulo 10
 shift left num: push 2, shift left => shift left 2
 shift right num: push 2, shift right => shift right 2
 call function named: push 'func', call function => call function 'func'
 call method named: push 'func', call method => call method 'func'
 */

#endif /* DD_ACTION_OPCODES_H_INCLUDED */
