/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

  if(condition & 0xfe00) println("condition: keyPress(%c)", (condition&0xfe00)>>9);
  if(condition & 0x100) println("condition: overDownToIdle");
  if(condition & 0x80)  println("condition: idleToOverDown");
  if(condition & 0x40)  println("condition: outDownToIdle");
  if(condition & 0x20)  println("condition: outDownToOverDown");
  if(condition & 0x10)  println("condition: overDownToOutDown");
  if(condition & 0x08)  println("condition: overDownToOverUp");
  if(condition & 0x04)  println("condition: overUpToOverDown");
  if(condition & 0x02)  println("condition: overUpToIdle");
  if(condition & 0x01)  println("condition: idleToOverUp");
