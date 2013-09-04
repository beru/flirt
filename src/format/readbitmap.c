/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "readbitmap.h"

#ifdef HAVE_ZLIB

#include <zlib.h>

void
readDefineBitsLossless(ddMovieClip* player, ddReader* r, int length, int version)
{
	int start = ddReader_getOffset(r);
	int tablesize = 0;
	ddColor* colortable;
	long size, bufsize;
	unsigned long datasize;
	unsigned char* buffer, *tmpdata, *p;

	int bitmapID = readUInt16(r);
	int format = readUInt8(r);
	int i, x, y, res;

	/*
	 case 3: puts("8 bpp\n"); break;
	 case 4: puts("16 bpp\n"); break;
	 case 5: puts("32 bpp\n"); break;
	 default: error("unknown bit format: %i", format); break;
		 */

	int width = readUInt16(r);
	int height = readUInt16(r);

	int rowwidth = ((width + 3) / 4) * 4; // row data are 32-bit aligned
			
	ddImage* image = dd_newImage(width, height);
	UInt8* data = ddImage_getData(image);

	if ( format == 3 )
	{
		tablesize = readUInt8(r) + 1;
		size = rowwidth * height + 4 * tablesize;
	}
	else
		size = 4 * width * height;

	size += 248; /* XXX - ??? */
	size += 1024;

	datasize = size;

	bufsize = start + length - ddReader_getOffset(r);
	buffer = readBlock(r, bufsize);

	tmpdata = dd_malloc(size);

	if ( (res = uncompress(tmpdata, &datasize, buffer, bufsize)) != Z_OK )
		dd_error("Couldn't uncompress bits! (err: %i)\n", res);

	p = tmpdata;

	if ( format == 3 )
	{
		int rowstride = ddImage_getRowstride(image);
		colortable = dd_malloc(tablesize * sizeof(ddColor));

		if ( version == 2 ) /* 4 bytes per entry */
		{
			for ( i = 0; i < tablesize; ++i )
			{
				colortable[i] = ddColor_scale(DDCOLOR(0xff, p[0], p[1], p[2]), p[3]);
				p += 4;
			}
		}
		else /* 3 bytes per entry */
		{
			for ( i = 0; i < tablesize; ++i )
			{
				colortable[i] = DDCOLOR(0xff, p[0], p[1], p[2]);
				p += 3;
			}
		}

		for ( y = 0; y < height; ++y )
		{
			ddColor* row = (ddColor*)(data + y * rowstride);

			for ( x = 0; x < width; ++x )
			{
				if ( *p < tablesize )
					row[x] = colortable[*p];
				else
					row[x] = colortable[0];

				++p;
			}

			for ( ; x < rowwidth; ++x )
				++p;
		}

		dd_free(colortable);
	}
	else
	{
		// XXX - need format = 4: 16bpp
		int rowstride = ddImage_getRowstride(image);
		
		for ( y = 0; y < height; ++y )
		{
			ddColor* row = (ddColor*)(data + y * rowstride);
			
			for ( x = 0; x < width; ++x )
			{
				unsigned char a = (version == 2) ? *p++ : 0xff;
				unsigned char r = *p++;
				unsigned char g = *p++;
				unsigned char b = *p++;

				row[x] = ddColor_scale(DDCOLOR(0xff, r, g, b), a);
			}

			for ( ; x < rowwidth; ++x )
				++p;
		}
	}

	dd_free(tmpdata);
	dd_free(buffer);

	ddMovieClip_addCharacter(player, bitmapID,
						  (ddCharacter*)dd_newBitmapCharacter(image));
}

#else

#include "readbitmap.h"

void
readDefineBitsLossless(ddMovieClip* player, ddReader* r, int length)
{
	skipBytes(r, length);
}

#endif /* HAVE_ZLIB */
