/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "blocktypes.h"
#include "read.h"

#include <zlib.h>


char* blockName(Blocktype type);
void skipBytes(ddReader* r, int length);


/*
void readLibrarySymbol(ddReader* r, int length)
{
  int mystery = readUInt16(r);
  int characterID = readUInt16(r);
  char* name = readString(r);
}
*/

void readPassword(ddReader* r, int length)
{
  int mystery = readUInt16(r);
  char* password = readString(r);
}

void skipBytes(ddReader* r, int length)
{
  for(; length>0; --length)
    readUInt8(r);
}

/*
void readMovie(ddReader* r)
{
  int fileOffset, version, filesize, frameRate, totalFrames;

  while(!ddReader_isEOF(r))
  {
    if(readUInt8(r)=='F' && readUInt8(r)=='W' && readUInt8(r)=='S')
      break;
  }

  if(ddReader_isEOF(r))
    error("Doesn't look like a swf file to me..\n");

  fileOffset = 3;

  version = readUInt8(r);

  filesize = readUInt32(r);

  readRect(r);

  frameRate = readUInt8(r)/256.0 + readUInt8(r);

  totalFrames = readUInt16(r);

  for(;;)
  {
    int block = readUInt16(r);
    int type = block>>6;

    int length = block & ((1<<6)-1);

    if(length == 63) // it's a long block.
      length = readUInt32(r);

    if(type == 0 || fileOffset >= filesize || length < 0)
      break;

    switch(type)
    {
      case DEFINESPRITE:        readSprite(r, length);         break;
      case DEFINESHAPE3:
      case DEFINESHAPE2:
      case DEFINESHAPE:         readShape(r, length, type);	break;
      case PLACEOBJECT:		readPlaceObject(r, length);	break;
      case PLACEOBJECT2:	readPlaceObject2(r, length);	break;
      case REMOVEOBJECT:	readRemoveObject(r);		break;
      case REMOVEOBJECT2:	readRemoveObject2(r);		break;
      case SETBACKGROUNDCOLOR:	readSetBackgroundColor(r);	break;
      case FRAMELABEL:		readFrameLabel(r);		break;
      case DEFINEMORPHSHAPE:	readMorphShape(r, length);	break; 
      case DEFINEFONT:		readDefineFont(r, length);	break;
      case DEFINEFONT2:		readDefineFont2(r, length);	break;
      case DEFINEFONTINFO:	readFontInfo(r, length);	break;
      case DEFINETEXT:		readDefineText(r, length, 1);	break;
      case DEFINETEXT2:		readDefineText(r, length, 2);	break;
      case DOACTION:		readAction(r, length);	break;
      case DEFINESOUND:         readDefineSound(r, length);    break;
      case SOUNDSTREAMHEAD:     readSoundStreamHead(r, 1);     break;
      case SOUNDSTREAMHEAD2:    readSoundStreamHead(r, 2);     break;
      case SOUNDSTREAMBLOCK:    readSoundStreamBlock(r, length); break;
      case DEFINEBUTTON:        readDefineButton(r, length);   break;
      case DEFINEBUTTON2:       readDefineButton2(r, length);  break;
      case JPEGTABLES:          readJpegStream(r, length);     break;
      case DEFINEBITS:
      case DEFINEBITSJPEG2:     readDefineBitsJpeg(r,length);  break;
      case DEFINEBITSJPEG3:     readDefineBitsJpeg3(r,length); break;
      case DEFINELOSSLESS:
      case DEFINELOSSLESS2:	readDefineBitsLossless(r,length); break;
      case TEXTFIELD:	        readTextField(r, length);	break;
      case LIBRARYSYMBOL:	readLibrarySymbol(r, length);	break;
      case PASSWORD:		readPassword(r, length);	break;

      default:                  dumpBytes(r, length);	        break;
    }
  }

  dumpBytes(r, filesize-fileOffset);
}
*/