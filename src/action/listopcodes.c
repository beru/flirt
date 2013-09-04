/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "opcodes.h"

#ifdef DD_INCLUDE_DEBUGGER

void
printValue(ddActionValue val)
{
	if ( val.type == VALUE_INTEGER )
		fprintf(stderr, "%i", val.data.intValue);
	else if ( val.type == VALUE_DOUBLE )
		fprintf(stderr, "%f", val.data.doubleValue);
	else if ( val.type == VALUE_STRING )
		fprintf(stderr, "'%s'", val.data.stringValue);
	else if ( val.type == VALUE_NULL )
		fprintf(stderr, "null");
	else if ( val.type == VALUE_UNDEF )
		fprintf(stderr, "undef");
	else if ( val.type == VALUE_BOOLEAN )
		fprintf(stderr, val.data.intValue ? "true" : "false");
	else
		fprintf(stderr, "(val type %i)", val.type);
}


char*
listOpcode(ddActionOpcode* op)
{
	static char buffer[1024];

	switch ( op->op )
	{
		case ACTION_END:
			sprintf(buffer, "end\n");
			break;
		case ACTION_NEXTFRAME:
			sprintf(buffer, "nextframe\n");
			break;
		case ACTION_PREVFRAME:
			sprintf(buffer, "prevframe\n");
			break;
		case ACTION_PLAY:
			sprintf(buffer, "play\n");
			break;
		case ACTION_STOP:
			sprintf(buffer, "stop\n");
			break;
		case ACTION_TOGGLEQUALITY:
			sprintf(buffer, "togglequality\n");
			break;
		case ACTION_STOPSOUNDS:
			sprintf(buffer, "stopsounds\n");
			break;
		case ACTION_GOTOFRAME:
			sprintf(buffer, "gotoframe %i\n", op->val.data.intValue);
			break;
		case ACTION_GETURL:
			sprintf(buffer, "geturl '%s'\n", op->val.data.stringValue);
			break;
		case ACTION_WAITFORFRAME:
			sprintf(buffer, "waitforframe %i\n", op->data);
			break;
		case ACTION_SETTARGET:
			sprintf(buffer, "settarget '%s'\n", op->arg);
			break;
		case ACTION_GOTOLABEL:
			sprintf(buffer, "gotolabel '%s'\n", op->arg);
			break;
		case ACTION_ADD:
			sprintf(buffer, "add\n");
			break;
		case ACTION_SUBTRACT:
			sprintf(buffer, "subtract\n");
			break;
		case ACTION_MULTIPLY:
			sprintf(buffer, "multiply\n");
			break;
		case ACTION_DIVIDE:
			sprintf(buffer, "divide\n");
			break;
		case ACTION_EQUAL:
			sprintf(buffer, "equal\n");
			break;
		case ACTION_LESSTHAN:
			sprintf(buffer, "lessthan\n");
			break;
		case ACTION_LOGICALAND:
			sprintf(buffer, "logicaland\n");
			break;
		case ACTION_LOGICALOR:
			sprintf(buffer, "logicalor\n");
			break;
		case ACTION_LOGICALNOT:
			sprintf(buffer, "logicalnot\n");
			break;
		case ACTION_STRINGEQ:
			sprintf(buffer, "stringeq\n");
			break;
		case ACTION_STRINGLENGTH:
			sprintf(buffer, "stringlength\n");
			break;
		case ACTION_SUBSTRING:
			sprintf(buffer, "substring\n");
			break;
		case ACTION_POP:
			sprintf(buffer, "pop\n");
			break;
		case ACTION_INT:
			sprintf(buffer, "int\n");
			break;
		case ACTION_GETVARIABLE:
			sprintf(buffer, "getvariable\n");
			break;
		case ACTION_SETVARIABLE:
			sprintf(buffer, "setvariable\n");
			break;
		case ACTION_SETTARGETEXPRESSION:
			sprintf(buffer, "settargetexpression\n");
			break;
		case ACTION_STRINGCONCAT:
			sprintf(buffer, "stringconcat\n");
			break;
		case ACTION_GETPROPERTY:
			sprintf(buffer, "getproperty\n");
			break;
		case ACTION_SETPROPERTY:
			sprintf(buffer, "setproperty\n");
			break;
		case ACTION_DUPLICATECLIP:
			sprintf(buffer, "duplicateclip\n");
			break;
		case ACTION_REMOVECLIP:
			sprintf(buffer, "removeclip\n");
			break;
		case ACTION_TRACE:
			sprintf(buffer, "trace\n");
			break;
		case ACTION_STARTDRAGMOVIE:
			sprintf(buffer, "startdragmovie\n");
			break;
		case ACTION_STOPDRAGMOVIE:
			sprintf(buffer, "stopdragmovie\n");
			break;
		case ACTION_STRINGCOMPARE:
			sprintf(buffer, "stringcompare\n");
			break;
		case ACTION_RANDOM:
			sprintf(buffer, "random\n");
			break;
		case ACTION_MBLENGTH:
			sprintf(buffer, "mblength\n");
			break;
		case ACTION_ORD:
			sprintf(buffer, "ord\n");
			break;
		case ACTION_CHR:
			sprintf(buffer, "chr\n");
			break;
		case ACTION_GETTIMER:
			sprintf(buffer, "gettimer\n");
			break;
		case ACTION_MBSUBSTRING:
			sprintf(buffer, "mbsubstring\n");
			break;
		case ACTION_MBORD:
			sprintf(buffer, "mbord\n");
			break;
		case ACTION_MBCHR:
			sprintf(buffer, "mbchr\n");
			break;
		case ACTION_WAITFORFRAMEEXPRESSION:
			sprintf(buffer, "waitforframeexpression\n");
			break;
		case ACTION_PUSHDATA:
			sprintf(buffer, "pushdata - XXX\n");
			break;
		case ACTION_BRANCHALWAYS:
			sprintf(buffer, "branchalways %i\n", op->val.data.intValue);
			break;
		case ACTION_GETURL2:
			sprintf(buffer, "geturl2 (%i)\n", op->data);
			break;
		case ACTION_BRANCHIFTRUE:
			sprintf(buffer, "branchiftrue %i\n", op->val.data.intValue);
			break;
		case ACTION_CALLFRAME:
			sprintf(buffer, "callframe '%s'\n", op->arg);
			break;
		case ACTION_GOTOEXPRESSION:
			sprintf(buffer, "gotoexpression (%i)\n", op->data);
			break;
		case ACTION_DELETE:
			sprintf(buffer, "delete\n");
			break;
		case ACTION_VAREQUALS:
			sprintf(buffer, "varequals\n");
			break;
		case ACTION_CALLFUNCTION:
			sprintf(buffer, "callfunction\n");
			break;
		case ACTION_RETURN:
			sprintf(buffer, "return\n");
			break;
		case ACTION_MODULO:
			sprintf(buffer, "modulo\n");
			break;
		case ACTION_NEW:
			sprintf(buffer, "new\n");
			break;
		case ACTION_NEWMETHOD:
			sprintf(buffer, "newmethod\n");
			break;
		case ACTION_VAR:
			sprintf(buffer, "var\n");
			break;
		case ACTION_INITARRAY:
			sprintf(buffer, "initarray\n");
			break;
		case ACTION_INITOBJECT:
			sprintf(buffer, "initobject\n");
			break;
		case ACTION_TYPEOF:
			sprintf(buffer, "typeof\n");
			break;
		case ACTION_ENUMERATE:
			sprintf(buffer, "enumerate\n");
			break;
		case ACTION_NEWADD:
			sprintf(buffer, "newadd\n");
			break;
		case ACTION_NEWLESSTHAN:
			sprintf(buffer, "newlessthan\n");
			break;
		case ACTION_NEWEQUAL:
			sprintf(buffer, "newequal\n");
			break;
		case ACTION_MAKENUMBER:
			sprintf(buffer, "makenumber\n");
			break;
		case ACTION_MAKESTRING:
			sprintf(buffer, "makestring\n");
			break;
		case ACTION_DUP:
			sprintf(buffer, "dup\n");
			break;
		case ACTION_SWAP:
			sprintf(buffer, "swap\n");
			break;
		case ACTION_GETMEMBER:
			sprintf(buffer, "getmember\n");
			break;
		case ACTION_SETMEMBER:
			sprintf(buffer, "setmember\n");
			break;
		case ACTION_INCREMENT:
			sprintf(buffer, "increment\n");
			break;
		case ACTION_DECREMENT:
			sprintf(buffer, "decrement\n");
			break;
		case ACTION_CALLMETHOD:
			sprintf(buffer, "callmethod\n");
			break;
		case ACTION_ENUMERATE2:
			sprintf(buffer, "enumerate2\n");
			break;
		case ACTION_BITWISEAND:
			sprintf(buffer, "bitwiseand\n");
			break;
		case ACTION_BITWISEOR:
			sprintf(buffer, "bitwiseor\n");
			break;
		case ACTION_BITWISEXOR:
			sprintf(buffer, "bitwisexor\n");
			break;
		case ACTION_SHIFTLEFT:
			sprintf(buffer, "shiftleft\n");
			break;
		case ACTION_SHIFTRIGHT:
			sprintf(buffer, "shiftright\n");
			break;
		case ACTION_SHIFTRIGHT2:
			sprintf(buffer, "shiftright2\n");
			break;
		case ACTION_STRICTEQ:
			sprintf(buffer, "stricteq\n");
			break;
		case ACTION_GREATERTHAN:
			sprintf(buffer, "greaterthan\n");
			break;
		case ACTION_SETREGISTER:
			sprintf(buffer, "setregister %i\n", op->data);
			break;
		case ACTION_DECLARENAMES:
			sprintf(buffer, "declarenames\n");
			break;
		case ACTION_WITH:
			sprintf(buffer, "with %i\n", op->val.data.intValue);
			break;
		case ACTION_DEFINEFUNCTION:
			sprintf(buffer, "definefunction '%s'\n", op->arg);
			break;

		// recoder additions

		case ACTION_GETNAMEDVAR:
			sprintf(buffer, "getnamedvar '%s'\n", op->arg);
			break;
		case ACTION_SETNAMEDVAR:
			sprintf(buffer, "setnamedvar '%s' = %s\n", op->arg, ddActionValue_getStringValue(NULL, op->val));
			break;
		case ACTION_GETNAMEDMEMBER:
			sprintf(buffer, "getnamedmember '%s'\n", op->arg);
			break;
		case ACTION_SETNAMEDMEMBER:
			sprintf(buffer, "setnamedmember '%s' = %s\n", op->arg, ddActionValue_getStringValue(NULL, op->val));
			//printValue(op->val);
			//sprintf(buffer, "\n");
			break;
		case ACTION_DELETENAMEDVAR:
			sprintf(buffer, "deletenamedvar '%s'\n", op->arg);
			break;
		case ACTION_NAMEDVAR:
			sprintf(buffer, "namedvar '%s'\n", op->arg);
			break;
		case ACTION_NAMEDVAREQ:
			sprintf(buffer, "namedvareq '%s' = %s\n", op->arg, ddActionValue_getStringValue(NULL, op->val));
			//printValue(op->val);
			//sprintf(buffer, "\n");
			break;
		case ACTION_NEWNAMED:
			sprintf(buffer, "newnamed '%s'\n", op->arg);
			break;
		case ACTION_EQUALS_INT:
			sprintf(buffer, "equalsint %i\n", op->val.data.intValue);
			break;
		case ACTION_EQUALS_DOUBLE:
			sprintf(buffer, "equalsdouble %f\n", op->val.data.doubleValue);
			break;
		case ACTION_EQUALS_STRING:
			sprintf(buffer, "equalsstring '%s'\n", op->val.data.stringValue);
			break;
		case ACTION_LESSTHAN_INT:
			sprintf(buffer, "lessthanint %i\n", op->val.data.intValue);
			break;
		case ACTION_LESSTHAN_DOUBLE:
			sprintf(buffer, "lessthandouble %f\n", op->val.data.doubleValue);
			break;
		case ACTION_LESSTHAN_STRING:
			sprintf(buffer, "lessthanstring '%s'\n", op->val.data.stringValue);
			break;
		case ACTION_GREATERTHAN_INT:
			sprintf(buffer, "greaterthanint %i\n", op->val.data.intValue);
			break;
		case ACTION_GREATERTHAN_DOUBLE:
			sprintf(buffer, "greaterthandouble %f\n", op->val.data.doubleValue);
			break;
		case ACTION_GREATERTHAN_STRING:
			sprintf(buffer, "greaterthanstring '%s'\n", op->val.data.stringValue);
			break;
		case ACTION_ADD_INT:
			sprintf(buffer, "addint %i\n", op->val.data.intValue);
			break;
		case ACTION_ADD_DOUBLE:
			sprintf(buffer, "adddouble %f\n", op->val.data.doubleValue);
			break;
		case ACTION_ADD_STRING:
			sprintf(buffer, "addstring '%s'\n", op->val.data.stringValue);
			break;
		case ACTION_SUBTRACT_INT:
			sprintf(buffer, "subtractint %i\n", op->val.data.intValue);
			break;
		case ACTION_SUBTRACT_DOUBLE:
			sprintf(buffer, "subtractdouble %f\n", op->val.data.doubleValue);
			break;
		case ACTION_MULTIPLY_INT:
			sprintf(buffer, "multiplyint %i\n", op->val.data.intValue);
			break;
		case ACTION_MULTIPLY_DOUBLE:
			sprintf(buffer, "multiplydouble %f\n", op->val.data.doubleValue);
			break;
		case ACTION_DIVIDE_INT:
			sprintf(buffer, "divideint %i\n", op->val.data.intValue);
			break;
		case ACTION_DIVIDE_DOUBLE:
			sprintf(buffer, "dividedouble %f\n", op->val.data.doubleValue);
			break;
		case ACTION_MODULO_INT:
			sprintf(buffer, "moduloint %i\n", op->val.data.intValue);
			break;
		case ACTION_SHIFTLEFT_INT:
			sprintf(buffer, "shiftleftint %i\n", op->val.data.intValue);
			break;
		case ACTION_SHIFTRIGHT_INT:
			sprintf(buffer, "shiftrightint %i\n", op->val.data.intValue);
			break;
		case ACTION_CALLNAMEDFRAME:
			sprintf(buffer, "callnamedframe '%s'\n", op->arg);
			break;
		case ACTION_CALLNAMEDFUNCTION:
			sprintf(buffer, "callnamedfunction '%s'\n", op->arg);
			break;
		case ACTION_CALLNAMEDFUNCTIONNARGS:
			sprintf(buffer, "callnamedfunctionnargs '%s' %i\n", op->arg, op->val.data.intValue);
			break;
		case ACTION_CALLNAMEDMETHOD:
			sprintf(buffer, "callnamedmethod '%s'\n", op->arg);
			break;
		case ACTION_PUSH_VALUE:
			sprintf(buffer, "pushvalue '%s'\n", ddActionValue_getStringValue(NULL, op->val));
			break;
		case ACTION_PUSH_REGISTER:
			sprintf(buffer, "pushregister %i\n", op->data);
			break;
		case ACTION_BRANCHIFFALSE:
			sprintf(buffer, "branchiffalse %i\n", op->val.data.intValue);
			break;
		default:
			sprintf(buffer, "Unknown opcode: %i\n", op->op);
	}

	return buffer;
}


char*
ddAction_opcodeAtOffset(ddAction* action, int index)
{
	while ( index > action->length && action != NULL )
	{
		index -= action->length;
		action = action->next;
	}

	if ( action == NULL )
		return "requested index past end of action!";

	return listOpcode(&action->opcodes[index]);
}


ddAction*
ddAction_functionAtOffset(ddAction* action, int offset)
{
	while ( offset > action->length && action->next != NULL )
		action = action->next;
	
	if ( action->opcodes[offset].op == ACTION_DEFINEFUNCTION )
	{
		ddActionFunction* function = action->opcodes[offset].val.data.functionValue;

		if ( function->type == FUNCTION_BYTECODE )
			return function->data.bytecode.action;
	}
	
	return NULL;
}

/*
void
listAction(ddAction* action)
{
	ddActionOpcode* ops = action->opcodes;
	int length = action->length;
	int i;
	
	for ( i = 0; i < length; ++i )
	{
		fprintf(stderr, "%03i: ", i);
		listOpcode(ops++);
	}
}
*/

#endif
