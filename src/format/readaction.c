/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>

#include "readaction.h"
#include "../action/bytecodes.h"

struct opcodeWithOffset
{
	ddActionOpcode opcode;
	int offset;
	boolean target;
};

struct opcodeList
{
	struct opcodeWithOffset* opcodes;
	int opcount;
	char** dictionary;
	int dictionarySize;
	unsigned char* ptr;
	unsigned char* start;
	boolean needsArguments;
};


ddAction*
readActionBlock(ddReader* r, unsigned char* block, int length);

void
readActionOp(ddReader* r, struct opcodeList* list);


static struct opcodeWithOffset*
addOpcode(struct opcodeList* list)
{
	struct opcodeWithOffset* op;
#ifdef _MSC_VER
	ddActionOpcode code;
	code.op = ACTION_END;
	code.data = 0;
	code.arg = NULL;
	code.val = ddNullValue;
#else
	ddActionOpcode code = { ACTION_END, 0, NULL, ddNullValue };
#endif
	
	list->opcodes = dd_realloc(list->opcodes,
							(list->opcount+1) * sizeof(struct opcodeWithOffset));

	op = &list->opcodes[list->opcount++];
	op->opcode = code;
	op->offset = 0;
	op->target = DD_FALSE;
	
	return op;
}


static ddAction*
recodeAction(struct opcodeList* list)
{
	int i;
	int* offsets = dd_malloc(list->opcount * sizeof(int));
	ddAction* newaction;
	
	for ( i = 0; i < list->opcount; ++i )
		offsets[i] = -1;
	
	// translate bytecode format into faster opcode format

	newaction = dd_newAction();
	
	newaction->dictionary = list->dictionary;
	newaction->dictionarySize = list->dictionarySize;
	
	// fix for obfuscator nonsense
	
	if ( list->opcodes[0].opcode.op == ACTION_LOGICALNOT )
	{
		list->opcodes[0].opcode.op = ACTION_PUSH_VALUE;
		list->opcodes[0].opcode.val = ddTrueValue;
	}

	for ( i = 0; i < list->opcount; ++i )
	{
		ddActionOpcode* newcode;
		ddActionOpcode* opcode = &list->opcodes[i].opcode;
		ddActionOpcode* next = &list->opcodes[i + 1].opcode;

		offsets[i] = newaction->length;

		if ( opcode->op == ACTION_DECLARENAMES )
			continue;
		
		if ( opcode->op == ACTION_LOGICALNOT &&
			 list->opcodes[i + 1].target != DD_TRUE && next->op == ACTION_LOGICALNOT )
		{
			++i;
			continue;
		}

		newcode = ddAction_newOpcode(newaction);

		if ( opcode->op == ACTION_LOGICALNOT && next->op == ACTION_BRANCHIFTRUE )
		{
			newcode->op = ACTION_BRANCHIFFALSE;
			newcode->val = next->val;
			++i;
			continue;
		}

		if ( opcode->op != ACTION_PUSH_VALUE || list->opcodes[i + 1].target == DD_TRUE )
			goto defaultOp;
		
		if ( opcode->val.type == VALUE_STRING )
		{
			ddActionOpcode* nextnext = &list->opcodes[i + 2].opcode;
			char* str = opcode->val.data.stringValue;

			if ( next->op == ACTION_PUSH_VALUE && next->val.type == VALUE_STRING )
			{
				if ( list->opcodes[i + 2].target == DD_TRUE )
					goto defaultOp;
				
				if ( nextnext->op == ACTION_SETVARIABLE )
				{
					newcode->op = ACTION_SETNAMEDVAR;
					newcode->arg = str;
					newcode->val = next->val;
					i += 2;
				}
				else if ( nextnext->op == ACTION_SETMEMBER )
				{
					newcode->op = ACTION_SETNAMEDMEMBER;
					newcode->arg = str;
					newcode->val = next->val;
					i += 2;
				}
				else if ( nextnext->op == ACTION_VAREQUALS )
				{
					newcode->op = ACTION_NAMEDVAREQ;
					newcode->arg = str;
					newcode->val = next->val;
					i += 2;
				}
				else goto defaultOp;
			}
			else if ( next->op == ACTION_PUSH_VALUE && next->val.type == VALUE_INTEGER )
			{
				if ( list->opcodes[i + 2].target == DD_TRUE )
					goto defaultOp;

				if ( nextnext->op == ACTION_SETVARIABLE )
				{
					newcode->op = ACTION_SETNAMEDVAR;
					newcode->arg = str;
					newcode->val = next->val;
					i += 2;
				}
				else if ( nextnext->op == ACTION_SETMEMBER )
				{
					newcode->op = ACTION_SETNAMEDMEMBER;
					newcode->arg = str;
					newcode->val = next->val;
					i += 2;
				}
				else if ( nextnext->op == ACTION_VAREQUALS )
				{
					newcode->op = ACTION_NAMEDVAREQ;
					newcode->arg = str;
					newcode->val = next->val;
					i += 2;
				}
				else goto defaultOp;
			}
			else if ( next->op == ACTION_PUSH_VALUE && next->val.type == VALUE_DOUBLE )
			{
				if ( list->opcodes[i + 2].target == DD_TRUE )
					goto defaultOp;

				if ( nextnext->op == ACTION_SETVARIABLE )
				{
					newcode->op = ACTION_SETNAMEDVAR;
					newcode->arg = str;
					newcode->val = next->val;
					i += 2;
				}
				else if ( nextnext->op == ACTION_SETMEMBER )
				{
					newcode->op = ACTION_SETNAMEDMEMBER;
					newcode->arg = str;
					newcode->val = next->val;
					i += 2;
				}
				else if ( nextnext->op == ACTION_VAREQUALS )
				{
					newcode->op = ACTION_NAMEDVAREQ;
					newcode->arg = str;
					newcode->val = next->val;
					i += 2;
				}
				else goto defaultOp;
			}
			else if ( next->op == ACTION_GETVARIABLE )
			{
				newcode->op = ACTION_GETNAMEDVAR;
				newcode->arg = str;
				++i;
			}
			else if ( next->op == ACTION_GETMEMBER )
			{
				newcode->op = ACTION_GETNAMEDMEMBER;
				newcode->arg = str;
				++i;
			}
			else if ( next->op == ACTION_DELETE )
			{
				newcode->op = ACTION_DELETENAMEDVAR;
				newcode->arg = str;
				++i;
			}
			else if ( next->op == ACTION_VAR )
			{
				newcode->op = ACTION_NAMEDVAR;
				newcode->arg = str;
				++i;
			}
			else if ( next->op == ACTION_NEW )
			{
				newcode->op = ACTION_NEWNAMED;
				newcode->arg = str;
				++i;
			}
			else if ( next->op == ACTION_NEWEQUAL )
			{
				newcode->op = ACTION_EQUALS_STRING;
				newcode->val = opcode->val;
				++i;
			}
			else if ( next->op == ACTION_NEWLESSTHAN )
			{
				newcode->op = ACTION_LESSTHAN_STRING;
				newcode->val = opcode->val;
				++i;
			}
			else if ( next->op == ACTION_GREATERTHAN )
			{
				newcode->op = ACTION_GREATERTHAN_STRING;
				newcode->val = opcode->val;
				++i;
			}
			else if ( next->op == ACTION_NEWADD )
			{
				newcode->op = ACTION_ADD_STRING;
				newcode->val = opcode->val;
				++i;
			}
			else if ( next->op == ACTION_CALLFUNCTION )
			{
				newcode->op = ACTION_CALLNAMEDFUNCTION;
				newcode->arg = str;
				++i;
			}
			else if ( next->op == ACTION_CALLMETHOD )
			{
				newcode->op = ACTION_CALLNAMEDMETHOD;
				newcode->arg = str;
				++i;
			}
			else if ( next->op == ACTION_CALLFRAME )
			{
				newcode->op = ACTION_CALLNAMEDFRAME;
				newcode->arg = str;
				++i;
			}
			else goto defaultOp;
		}
		else if ( opcode->val.type == VALUE_INTEGER )
		{
			int num = opcode->val.data.intValue;

			// XXX - if next is push string and nextnext is callfunction/method
			// ACTION_CALLNAMEDFUNCTIONNARGS or something like..

			if ( next->op == ACTION_PUSH_VALUE && next->val.type == VALUE_STRING )
			{
				ddActionOpcode* nextnext = &list->opcodes[i + 2].opcode;

				if ( list->opcodes[i + 2].target == DD_TRUE )
					goto defaultOp;

				if ( nextnext->op == ACTION_CALLFUNCTION )
				{
					newcode->op = ACTION_CALLNAMEDFUNCTIONNARGS;
					newcode->arg = next->val.data.stringValue;
					newcode->val.data.intValue = opcode->val.data.intValue;
					i += 2;
				}
				else
					goto defaultOp;
			}
			else if ( next->op == ACTION_NEWEQUAL )
			{
				newcode->op = ACTION_EQUALS_INT;
				newcode->val.data.intValue = num;
				++i;
			}
			else if ( next->op == ACTION_NEWLESSTHAN )
			{
				newcode->op = ACTION_LESSTHAN_INT;
				newcode->val.data.intValue = num;
				++i;
			}
			else if ( next->op == ACTION_GREATERTHAN )
			{
				newcode->op = ACTION_GREATERTHAN_INT;
				newcode->val.data.intValue = num;
				++i;
			}
			else if ( next->op == ACTION_NEWADD )
			{
				newcode->op = ACTION_ADD_INT;
				newcode->val.data.intValue = num;
				++i;
			}
			else if ( next->op == ACTION_SUBTRACT )
			{
				newcode->op = ACTION_SUBTRACT_INT;
				newcode->val.data.intValue = num;
				++i;
			}
			else if ( next->op == ACTION_MULTIPLY )
			{
				newcode->op = ACTION_MULTIPLY_INT;
				newcode->val.data.intValue = num;
				++i;
			}
			else if ( next->op == ACTION_DIVIDE )
			{
				newcode->op = ACTION_DIVIDE_INT;
				newcode->val.data.intValue = num;
				++i;
			}
			else if ( next->op == ACTION_MODULO )
			{
				newcode->op = ACTION_MODULO_INT;
				newcode->val.data.intValue = num;
				++i;
			}
			else if ( next->op == ACTION_SHIFTLEFT )
			{
				newcode->op = ACTION_SHIFTLEFT_INT;
				newcode->val.data.intValue = num;
				++i;
			}
			else if ( next->op == ACTION_SHIFTRIGHT )
			{
				newcode->op = ACTION_SHIFTRIGHT_INT;
				newcode->val.data.intValue = num;
				++i;
			}
			else goto defaultOp;
		}
		else if ( opcode->val.type == VALUE_DOUBLE )
		{
			double num = opcode->val.data.doubleValue;

			if ( next->op == ACTION_NEWEQUAL )
			{
				newcode->op = ACTION_EQUALS_DOUBLE;
				newcode->val.data.doubleValue = num;
				++i;
			}
			else if ( next->op == ACTION_NEWLESSTHAN )
			{
				newcode->op = ACTION_LESSTHAN_DOUBLE;
				newcode->val.data.doubleValue = num;
				++i;
			}
			else if ( next->op == ACTION_GREATERTHAN )
			{
				newcode->op = ACTION_GREATERTHAN_DOUBLE;
				newcode->val.data.doubleValue = num;
				++i;
			}
			else if ( next->op == ACTION_NEWADD )
			{
				newcode->op = ACTION_ADD_DOUBLE;
				newcode->val.data.doubleValue = num;
				++i;
			}
			else if ( next->op == ACTION_SUBTRACT )
			{
				newcode->op = ACTION_SUBTRACT_DOUBLE;
				newcode->val.data.doubleValue = num;
				++i;
			}
			else if ( next->op == ACTION_MULTIPLY )
			{
				newcode->op = ACTION_MULTIPLY_DOUBLE;
				newcode->val.data.doubleValue = num;
				++i;
			}
			else if ( next->op == ACTION_DIVIDE )
			{
				newcode->op = ACTION_DIVIDE_DOUBLE;
				newcode->val.data.doubleValue = num;
				++i;
			}
			else goto defaultOp;
		}
		else goto defaultOp;

		continue;

	  defaultOp:
		newcode->op = opcode->op;
		newcode->data = opcode->data;
		newcode->arg = opcode->arg;
		newcode->val = opcode->val;
	}

	for ( i = 0; i < newaction->length; ++i )
	{
		ddActionOpcode* op = &newaction->opcodes[i];
		
		if ( op->op == ACTION_BRANCHIFTRUE || op->op == ACTION_BRANCHIFFALSE ||
			 op->op == ACTION_BRANCHALWAYS || op->op == ACTION_WITH )
		{
			int offset = offsets[op->val.data.intValue];

			if ( offset == -1 )
			{
				dd_warn("Couldn't find recoded offset!");
				op->val.data.intValue = 0;
			}
			else
				op->val.data.intValue = offset - (i + 1);
		}
	}

	dd_free(offsets);

	return newaction;
}


static inline void
readDeclareNames(ddReader* r, struct opcodeList* list)
{
	int i;
	unsigned char* p = list->ptr + 2;

	// XXX - these leak

	// XXX - probably possible to specify > 256
	int dictionarySize = (unsigned char)list->ptr[0];

	list->dictionary = dd_malloc(dictionarySize * sizeof(char*));

	for ( i = 0; i < dictionarySize; ++i )
	{
		list->dictionary[i] = strdup((char*)p);

		while ( *p != '\0' )
			++p;

		++p;
	}

	list->dictionarySize = dictionarySize;
	list->ptr = p;

	ddReader_addDictionary(r, list->dictionary, dictionarySize);
}


static inline void
readPushData(struct opcodeList* list, int offset, int length)
{
	unsigned char* p = list->ptr;
	unsigned char* end = p + length;

	while ( p < end )
	{
		struct opcodeWithOffset* opcode = addOpcode(list);
		
		int type = *p;
		++p;

		opcode->offset = offset++;  // first push should be only one with this offset

		switch ( type )
		{
			case 0: /* string */
			{
				opcode->opcode.op = ACTION_PUSH_VALUE;
				opcode->opcode.val = dd_newActionValue_string((char*)p);

				if ( strcasecmp((char*)p, "arguments") == 0 )
					list->needsArguments = DD_TRUE;
				
				p += strlen((char*)p) + 1;

				break;
			}

			case 1: /* 4-byte float */
			{
				float f;
				unsigned char* t = (unsigned char* )&f;

#ifdef DD_LITTLE_ENDIAN
				t[0] = *p++;
				t[1] = *p++;
				t[2] = *p++;
				t[3] = *p++;
#else
				t[3] = *p++;
				t[2] = *p++;
				t[1] = *p++;
				t[0] = *p++;
#endif
				opcode->opcode.op = ACTION_PUSH_VALUE;
				opcode->opcode.val = dd_newActionValue_double(f);
				break;
			}

			case 2: /* null */
				opcode->opcode.op = ACTION_PUSH_VALUE;
				opcode->opcode.val = ddNullValue;
				break;

			case 3: /* ??? */
				opcode->opcode.op = ACTION_PUSH_VALUE;
				opcode->opcode.val = ddUndefValue;
				break;

			case 4: /* register */
			{
				int n = *p;

				//if ( n >= 4 )
				//	dd_warn("Bad register number in pushdata op: %i", n);
				
				opcode->opcode.op = ACTION_PUSH_REGISTER;
				opcode->opcode.data = n;
				++p;

				break;
			}

			case 5: /* boolean */
				opcode->opcode.op = ACTION_PUSH_VALUE;
				opcode->opcode.val = (*p++) ? ddTrueValue : ddFalseValue;
				break;

			case 6: /* double */
			{
				double d;
				unsigned char* t = (unsigned char*)&d;

#ifdef DD_LITTLE_ENDIAN
				t[4] = *p++;
				t[5] = *p++;
				t[6] = *p++;
				t[7] = *p++;
				t[0] = *p++;
				t[1] = *p++;
				t[2] = *p++;
				t[3] = *p++;
#else
				t[3] = *p++;
				t[2] = *p++;
				t[1] = *p++;
				t[0] = *p++;
				t[7] = *p++;
				t[6] = *p++;
				t[5] = *p++;
				t[4] = *p++;
#endif

				opcode->opcode.op = ACTION_PUSH_VALUE;

				if ( d == 0 )
					opcode->opcode.val = dd_newActionValue_int(0);
				else
					opcode->opcode.val = dd_newActionValue_double(d);
				
				break;
			}

			case 7: /* int */
			{
				int d;
				unsigned char* t = (unsigned char* )&d;

#ifdef DD_LITTLE_ENDIAN
				t[0] = *p++;
				t[1] = *p++;
				t[2] = *p++;
				t[3] = *p++;
#else
				t[3] = *p++;
				t[2] = *p++;
				t[1] = *p++;
				t[0] = *p++;
#endif

				opcode->opcode.op = ACTION_PUSH_VALUE;
				opcode->opcode.val = dd_newActionValue_int(d);
				break;
			}

			case 8: /* dictionary entry */
			{
				int n = *p;

				if ( n < list->dictionarySize )
				{
					opcode->opcode.op = ACTION_PUSH_VALUE;

					// XXX - could avoid the copy, but cleanup wants unique vals
					opcode->opcode.val = dd_newActionValue_string(list->dictionary[n]);

					if ( strcasecmp(list->dictionary[n], "arguments") == 0 )
						list->needsArguments = DD_TRUE;
				}
				else
					dd_warn("n >= dictionarySize in push constant");

				++p;
				break;
			}

			default:
				dd_error("unexpected pushdata type: %i", type);
				break;
		}
	}

	list->ptr = p;
}


static void
fixJumps(struct opcodeList* list)
{
	int i;
	
	for ( i = 0; i < list->opcount; ++i )
	{
		struct opcodeWithOffset* op = list->opcodes + i;

		if ( op->opcode.op == ACTION_BRANCHALWAYS || op->opcode.op == ACTION_BRANCHIFTRUE ||
			 op->opcode.op == ACTION_BRANCHIFFALSE || op->opcode.op == ACTION_WITH )
		{
			int j;
			int jump = op->opcode.val.data.intValue;
			int target = list->opcodes[i + 1].offset + jump;

			if ( jump >= 0 )
			{
				for ( j = i; j < list->opcount; ++j )
				{
					if ( list->opcodes[j].offset == target )
					{
						list->opcodes[i].opcode.val.data.intValue = j;
						list->opcodes[j].target = DD_TRUE;
						break;
					}
				}

				if ( j == list->opcount )
					dd_warn("Couldn't find branch target!");
			}
			else
			{
				for ( j = i; j >= 0; --j )
				{
					if ( list->opcodes[j].offset == target )
					{
						list->opcodes[i].opcode.val.data.intValue = j;
						list->opcodes[j].target = DD_TRUE;
						break;
					}
				}

				if ( j == -1 )
					dd_warn("Couldn't find branch target!");
			}
		}
	}
}


static inline void
readDefineFunction(ddReader* r, struct opcodeList* list, int offset)
{
	struct opcodeWithOffset* op = addOpcode(list);
	struct opcodeWithOffset* endop;
	int i, nargs, len;
	unsigned char* data = list->ptr;
	char** args;
	ddAction* action;
	ddActionFunction* func;

	struct opcodeList funclist =
		{ NULL, 0, list->dictionary, list->dictionarySize, data, data, DD_FALSE };

	op->offset = offset;
	
	if ( *data != '\0' )
	{
		op->opcode.arg = strdup((char*)data); // name
		op->opcode.op = ACTION_DEFINEFUNCTION;
	}
	else
	{
		// anonymous function, left on stack
		op->opcode.arg = NULL;
		op->opcode.op = ACTION_PUSH_VALUE;
	}

	data += strlen((char*)data) + 1;

	nargs = data[0] + (data[1] << 8); // more than 256 args!?

	data += 2;

	args = dd_malloc(sizeof(char*) * nargs);

	for ( i = 0; i < nargs; ++i )
	{
		args[i] = strdup((char*)data);
		data += strlen(args[i]) + 1;
	}

	len = data[0] + (data[1] << 8);

	data += 2;
	funclist.ptr = data;

	while ( funclist.ptr < data + len )
		readActionOp(r, &funclist);

	endop = addOpcode(&funclist);
	endop->opcode.op = ACTION_END;
	endop->offset = funclist.ptr - funclist.start;

	fixJumps(&funclist);

	action = recodeAction(&funclist);
	--action->length;

	dd_free(funclist.opcodes);

	func = dd_newActionBytecodeFunction(action, nargs, args);

	if ( funclist.needsArguments )
		ddActionFunction_setNeedsArgumentsArray(func);
	
	op->opcode.val = dd_newActionValue_function(func);

	data += len;
	list->ptr = data;
}


void
readActionOp(ddReader* r, struct opcodeList* list)
{
	unsigned char* ptr = list->ptr;
	ddActionCode code = *ptr;
	unsigned int len = 0;

	++ptr;

	if ( (code & 0x80) == 0 )
	{
		struct opcodeWithOffset* op = addOpcode(list);
		
		op->opcode.op = code;
		op->offset = list->ptr - list->start;
		list->ptr = ptr;
		return;
	}
	
	len = ptr[0] + (ptr[1] << 8);
	ptr += 2;
	
	// some obfuscator out there puts a declare names block with fake length
	if ( code == ACTION_DECLARENAMES && ptr[0] == 0 && ptr[1] == 0 )
		len = 2;
	
	list->ptr = ptr;

	if ( code == ACTION_PUSHDATA )
		readPushData(list, list->ptr - 3 - list->start, len);

	else if ( code == ACTION_DEFINEFUNCTION )
		readDefineFunction(r, list, list->ptr - 3 - list->start);
	
	else
	{
		struct opcodeWithOffset* op = addOpcode(list);
		op->opcode.op = code;
		op->offset = list->ptr - 3 - list->start;
		
		switch ( code )
		{
			case ACTION_DECLARENAMES:
				readDeclareNames(r, list);
				break;
			case ACTION_WAITFORFRAME:
				op->opcode.data = ptr[2];
			case ACTION_GOTOFRAME:
			case ACTION_WITH:
				op->opcode.val = dd_newActionValue_int(ptr[0] + (ptr[1] << 8));
				break;
			case ACTION_GETURL2:
			case ACTION_GOTOEXPRESSION:
			case ACTION_SETREGISTER:
				op->opcode.data = *ptr;
				break;
			case ACTION_BRANCHIFTRUE:
			case ACTION_BRANCHALWAYS:
				op->opcode.val = dd_newActionValue_int(((char)ptr[1] << 8) + ptr[0]);
				break;
			case ACTION_GETURL:
				op->opcode.val = dd_newActionValue_string((char*)ptr + strlen((char*)ptr) + 1);
			case ACTION_SETTARGET:
			case ACTION_CALLFRAME:
			case ACTION_GOTOLABEL:
				op->opcode.arg = strdup((char*)ptr);
				break;
			default:
				dd_warn("unhandled opcode 0x%x in readActionOp()", code);
				break;
		}

		ptr += len;
		list->ptr = ptr;
	}
}


ddAction*
readActionBlock(ddReader* r, unsigned char* block, int length)
{
	ddAction* action;
	
	// build opcode structures for bytecodes
	unsigned char* end = block + length;
	struct opcodeList opcodes = { NULL, 0, NULL, 0, block, block, DD_FALSE };
	
	// read opcodes from block
	
	while ( opcodes.ptr < end )
		readActionOp(r, &opcodes);

	// translate jump relative offsets to absolute

	fixJumps(&opcodes);

	// shorten list by using extra bytecodes

	action = recodeAction(&opcodes);

	dd_free(opcodes.opcodes);
	
	return action;
}


ddAction*
readAction(ddReader* r, int length)
{
	ddAction* action;
	unsigned char* block = readBlock(r, length);

	action = readActionBlock(r, block, length);

	dd_free(block);
	
	return action;
}
