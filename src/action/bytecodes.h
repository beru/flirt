/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_ACTION_BYTECODES_H_INCLUDED
#define DD_ACTION_BYTECODES_H_INCLUDED

typedef enum
{
	ACTION_END        = 0x00,

	/* v3 actions */
	ACTION_NEXTFRAME     = 0x04,
	ACTION_PREVFRAME     = 0x05,
	ACTION_PLAY          = 0x06,
	ACTION_STOP          = 0x07,
	ACTION_TOGGLEQUALITY = 0x08,
	ACTION_STOPSOUNDS    = 0x09,
	ACTION_GOTOFRAME     = 0x81, /* >= 0x80 means record has args */
	ACTION_GETURL        = 0x83,
	ACTION_WAITFORFRAME  = 0x8A,
	ACTION_SETTARGET     = 0x8B,
	ACTION_GOTOLABEL     = 0x8C,

	/* v4 actions */
	ACTION_ADD                     = 0x0A,
	ACTION_SUBTRACT                = 0x0B,
	ACTION_MULTIPLY                = 0x0C,
	ACTION_DIVIDE                  = 0x0D,
	ACTION_EQUAL                   = 0x0E,
	ACTION_LESSTHAN                = 0x0F,
	ACTION_LOGICALAND              = 0x10,
	ACTION_LOGICALOR               = 0x11,
	ACTION_LOGICALNOT              = 0x12,
	ACTION_STRINGEQ                = 0x13,
	ACTION_STRINGLENGTH            = 0x14,
	ACTION_SUBSTRING               = 0x15,
	ACTION_POP                     = 0x17,
	ACTION_INT                     = 0x18,
	ACTION_GETVARIABLE             = 0x1C,
	ACTION_SETVARIABLE             = 0x1D,
	ACTION_SETTARGETEXPRESSION     = 0x20,
	ACTION_STRINGCONCAT            = 0x21,
	ACTION_GETPROPERTY             = 0x22,
	ACTION_SETPROPERTY             = 0x23,
	ACTION_DUPLICATECLIP           = 0x24,
	ACTION_REMOVECLIP              = 0x25,
	ACTION_TRACE                   = 0x26,
	ACTION_STARTDRAGMOVIE          = 0x27,
	ACTION_STOPDRAGMOVIE           = 0x28,
	ACTION_STRINGCOMPARE           = 0x29,
	ACTION_RANDOM                  = 0x30,
	ACTION_MBLENGTH                = 0x31,
	ACTION_ORD                     = 0x32,
	ACTION_CHR                     = 0x33,
	ACTION_GETTIMER                = 0x34,
	ACTION_MBSUBSTRING             = 0x35,
	ACTION_MBORD                   = 0x36,
	ACTION_MBCHR                   = 0x37,

	ACTION_WAITFORFRAMEEXPRESSION  = 0x8D,
	ACTION_PUSHDATA                = 0x96,
	ACTION_BRANCHALWAYS            = 0x99,
	ACTION_GETURL2                 = 0x9A,
	ACTION_BRANCHIFTRUE            = 0x9D,
	ACTION_CALLFRAME               = 0x9E,
	ACTION_GOTOEXPRESSION          = 0x9F,

	/* v5 actions */
	ACTION_DELETE                  = 0x3B,
	ACTION_VAREQUALS               = 0x3C,
	ACTION_CALLFUNCTION            = 0x3D,
	ACTION_RETURN                  = 0x3E,
	ACTION_MODULO                  = 0x3F,
	ACTION_NEW                     = 0x40,
	ACTION_VAR                     = 0x41,
	ACTION_INITARRAY               = 0x42,
	ACTION_INITOBJECT              = 0x43,
	ACTION_TYPEOF                  = 0x44,
	ACTION_ENUMERATE               = 0x46,
	ACTION_NEWADD                  = 0x47,
	ACTION_NEWLESSTHAN             = 0x48,
	ACTION_NEWEQUAL                = 0x49,
	ACTION_MAKENUMBER              = 0x4A,
	ACTION_MAKESTRING              = 0x4B,
	ACTION_DUP                     = 0x4C,
	ACTION_SWAP                    = 0x4D,
	ACTION_GETMEMBER               = 0x4E,
	ACTION_SETMEMBER               = 0x4F,
	ACTION_INCREMENT               = 0x50,
	ACTION_DECREMENT               = 0x51,
	ACTION_CALLMETHOD              = 0x52,
	ACTION_NEWMETHOD               = 0x53,
	ACTION_BITWISEAND              = 0x60,
	ACTION_BITWISEOR               = 0x61,
	ACTION_BITWISEXOR              = 0x62,
	ACTION_SHIFTLEFT               = 0x63,
	ACTION_SHIFTRIGHT              = 0x64,
	ACTION_SHIFTRIGHT2             = 0x65,
	
	ACTION_SETREGISTER             = 0x87,
	ACTION_DECLARENAMES            = 0x88,
	ACTION_WITH                    = 0x94,
	ACTION_DEFINEFUNCTION          = 0x9B,
	
	// v6 actions
	ACTION_INSTANCEOF			   = 0x54,
	ACTION_ENUMERATE2			   = 0x55,
	ACTION_STRICTEQ                = 0x66,
	ACTION_GREATERTHAN             = 0x67,
	ACTION_STRINGGREATER           = 0x68,

	// v7 actions
	ACTION_DEFINEFUNCTION2		   = 0x8E,
	ACTION_EXTENDS				   = 0x69,
	ACTION_CAST					   = 0x2B,
	ACTION_IMPLEMENTS			   = 0x2C,
	ACTION_TRY					   = 0x8F,
	ACTION_THROW				   = 0x2A,
	
	// transcoder additions

	ACTION_GETNAMEDVAR			   = 0xA0,
	ACTION_SETNAMEDVAR,
	ACTION_GETNAMEDMEMBER,
	ACTION_SETNAMEDMEMBER,
	ACTION_DELETENAMEDVAR,
	ACTION_NAMEDVAR,
	ACTION_NAMEDVAREQ,
	ACTION_NEWNAMED,
	ACTION_EQUALS_INT,
	ACTION_EQUALS_DOUBLE,
	ACTION_EQUALS_STRING,
	ACTION_LESSTHAN_INT,
	ACTION_LESSTHAN_DOUBLE,
	ACTION_LESSTHAN_STRING,
	ACTION_GREATERTHAN_INT,
	ACTION_GREATERTHAN_DOUBLE,
	ACTION_GREATERTHAN_STRING,
	ACTION_ADD_INT,
	ACTION_ADD_DOUBLE,
	ACTION_ADD_STRING,
	ACTION_SUBTRACT_INT,
	ACTION_SUBTRACT_DOUBLE,
	ACTION_MULTIPLY_INT,
	ACTION_MULTIPLY_DOUBLE,
	ACTION_DIVIDE_INT,
	ACTION_DIVIDE_DOUBLE,
	ACTION_MODULO_INT,
	ACTION_SHIFTLEFT_INT,
	ACTION_SHIFTRIGHT_INT,
	ACTION_CALLNAMEDFRAME,
	ACTION_CALLNAMEDFUNCTION,
	ACTION_CALLNAMEDFUNCTIONNARGS,
	ACTION_CALLNAMEDMETHOD,
	ACTION_PUSH_VALUE,
	ACTION_PUSH_REGISTER,
	ACTION_BRANCHIFFALSE

} ddActionCode;

#endif /* DD_ACTION_BYTECODES_H_INCLUDED */
