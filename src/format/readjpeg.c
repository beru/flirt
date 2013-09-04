/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include "readjpeg.h"

#ifdef HAVE_JPEGLIB

#include <stdio.h>

#define JPEG_STATIC 1

// toss this once we change "boolean" to ddBool
#define HAVE_BOOLEAN

#include <jpeglib.h>

#include <setjmp.h>

/* create jpeg source wrapper for ddReader* */

#define BUFFER_SIZE 1024

static unsigned char buffer[BUFFER_SIZE];

struct reader_src
{
	struct jpeg_source_mgr pub;   /* public fields */
	ddReader* reader;
	int length; /* length of jpeg block */
};


static void
init_source(j_decompress_ptr cinfo)
{
	//struct reader_src* src = (struct reader_src*)cinfo->src;
}


static boolean
fill_input_buffer(j_decompress_ptr cinfo)
{
	struct reader_src* src = (struct reader_src*)cinfo->src;
	size_t nbytes = dd_min(src->length, BUFFER_SIZE);

	readBlockIntoBuffer(src->reader, nbytes, buffer);
	
	if ( src->pub.next_input_byte == NULL && buffer[1] == 0xd9 && buffer[3] == 0xd8 )
	{
		buffer[1] = 0xd8;
		buffer[3] = 0xd9;
	}
	
	src->length -= nbytes;

	src->pub.next_input_byte = buffer;
	src->pub.bytes_in_buffer = nbytes;

	return DD_TRUE;
}


static void
skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
	struct reader_src* src = (struct reader_src*)cinfo->src;

	if ( num_bytes > 0 )
	{
		skipBytes(src->reader, num_bytes);

		src->pub.next_input_byte += (size_t) num_bytes;
		src->pub.bytes_in_buffer -= (size_t) num_bytes;
	}
}


static void
term_source(j_decompress_ptr cinfo)
{
	/* no work necessary here */
}


void
jpeg_reader_src(j_decompress_ptr cinfo, ddReader* r, int length)
{
	struct reader_src* src = dd_malloc(sizeof(struct reader_src));

	src->pub.init_source = init_source;
	src->pub.fill_input_buffer = fill_input_buffer;
	src->pub.skip_input_data = skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->pub.term_source = term_source;
	src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	src->pub.next_input_byte = NULL; /* until buffer loaded */

	src->reader = r;
	src->length = length;

	cinfo->src = (struct jpeg_source_mgr*)src;
}


struct my_error_mgr
{
	struct jpeg_error_mgr pub;	/* "public" fields */
	jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr* my_error_ptr;

void
my_error_exit(j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	longjmp(myerr->setjmp_buffer, 1);
}


ddImage*
readJpegImage(ddReader* r, int length)
{
	int res;

	UInt8* data;
	int rowstride;

	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;

	ddImage* image;

	JSAMPARRAY buffer;		/* Output row buffer */

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	if ( setjmp(jerr.setjmp_buffer) )
	{
		//struct reader_src* src = (struct reader_src *)cinfo.src;
		//my_error_ptr myerr = (my_error_ptr)cinfo.err;
		char errmsg[JMSG_LENGTH_MAX];

		cinfo.err->format_message((j_common_ptr)&cinfo, errmsg);
		dd_warn(errmsg);

		dd_free(cinfo.src);
		
		jpeg_destroy_decompress(&cinfo);

		return NULL;
	}

	jpeg_create_decompress(&cinfo);

	jpeg_reader_src(&cinfo, r, length);

	res = jpeg_read_header(&cinfo, DD_FALSE);

	if ( res != JPEG_HEADER_OK )
		res = jpeg_read_header(&cinfo, DD_TRUE);
	
	if ( res != JPEG_HEADER_OK )
	{
		dd_warn("empty jpeg image");
		return NULL;
	}

	jpeg_start_decompress(&cinfo);

	/* width and height available now */
	image = dd_newImage(cinfo.output_width, cinfo.output_height);

	data = ddImage_getData(image);

	/* JSAMPLEs per row in output buffer */
	rowstride = cinfo.output_width * cinfo.output_components;

	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, rowstride, 1);

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */

	while ( cinfo.output_scanline < cinfo.output_height )
	{
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could ask for
		* more than one scanline at a time if that's more convenient.
		*/

		unsigned int i;
		ddColor* row;

		(void)jpeg_read_scanlines(&cinfo, buffer, 1);

		row = (ddColor*)(data + (cinfo.output_scanline - 1) * image->rowstride);

		for ( i = 0; i < cinfo.output_width; ++i )
		{
			JSAMPLE *p = buffer[0] + 3 * i;
			row[i] = DDCOLOR(0xff, p[0], p[1], p[2]);
		}
	}

    /*
	 while(cinfo.output_scanline < cinfo.output_height)
	 {
		 jpeg_read_scanlines(&cinfo, data + cinfo.output_scanline*rowstride,
					   cinfo.output_height-cinfo.output_scanline);
	 }
	 */

	/* Step 7: Finish decompression */

	jpeg_finish_decompress(&cinfo);

	dd_free(cinfo.src);
	jpeg_destroy_decompress(&cinfo);

	return image;
}


struct jpeg_decompress_struct cinfo;

void
readJpegTables(ddMovieClip* player, ddReader* r, int length)
{
	int res;

	//struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	if ( setjmp(jerr.setjmp_buffer) )
	{
		//struct reader_src* src = (struct reader_src *)cinfo.src;
		//my_error_ptr myerr = (my_error_ptr)cinfo.err;
		char buffer[JMSG_LENGTH_MAX];

		cinfo.err->format_message((j_common_ptr)&cinfo, buffer);
		dd_warn(buffer);

		jpeg_destroy_decompress(&cinfo);
		
		return;
	}

	jpeg_create_decompress(&cinfo);

	jpeg_reader_src(&cinfo, r, length);

	res = jpeg_read_header(&cinfo, DD_FALSE);
}


void readDefineBitsJpeg(ddMovieClip* player, ddReader* r, int length)
{
	int bitmapID = readUInt16(r);
	int res;

	ddImage* image;
	UInt8* data;
	int rowstride;

	JSAMPARRAY buffer;		/* Output row buffer */

	struct my_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	
	if ( setjmp(jerr.setjmp_buffer) )
	{
		//struct reader_src* src = (struct reader_src *)cinfo.src;
		//my_error_ptr myerr = (my_error_ptr)cinfo.err;
		char buffer[JMSG_LENGTH_MAX];

		cinfo.err->format_message((j_common_ptr)&cinfo, buffer);
		dd_warn(buffer);

		jpeg_destroy_decompress(&cinfo);

		return;
	}

	((struct reader_src*)cinfo.src)->length += (length - 2);

	res = jpeg_read_header(&cinfo, DD_TRUE);

	if ( res != JPEG_HEADER_OK )
	{
		dd_warn("empty jpeg image");
		return;
	}

	jpeg_start_decompress(&cinfo);

	/* width and height available now */
	image = dd_newImage(cinfo.output_width, cinfo.output_height);

	data = ddImage_getData(image);

	/* JSAMPLEs per row in output buffer */
	rowstride = cinfo.output_width * cinfo.output_components;

	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, rowstride, 1);

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */

	while ( cinfo.output_scanline < cinfo.output_height )
	{
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could ask for
		* more than one scanline at a time if that's more convenient.
		*/

		unsigned int i;
		ddColor* row;

		(void)jpeg_read_scanlines(&cinfo, buffer, 1);

		row = (ddColor*)(data + (cinfo.output_scanline - 1) * image->rowstride);

		for ( i = 0; i < cinfo.output_width; ++i )
		{
			JSAMPLE *p = buffer[0] + 3 * i;
			row[i] = DDCOLOR(0xff, p[0], p[1], p[2]);
		}
	}

    /*
	 while(cinfo.output_scanline < cinfo.output_height)
	 {
		 jpeg_read_scanlines(&cinfo, data + cinfo.output_scanline*rowstride,
					   cinfo.output_height-cinfo.output_scanline);
	 }
	 */

	/* Step 7: Finish decompression */

	jpeg_finish_decompress(&cinfo);

	ddMovieClip_addCharacter(player, bitmapID,
						  (ddCharacter*)dd_newBitmapCharacter(image));
}


void
readDefineBitsJpeg2(ddMovieClip* player, ddReader* r, int length)
{
	int bitmapID = readUInt16(r);
	int end = ddReader_getOffset(r) + length - 2;
	ddImage* image;
	
	//readUInt16(r); // EOI
	//readUInt16(r); // SOI
	
	image = readJpegImage(r, length - 2 /*6*/);

	if ( image != NULL )
	{
		ddMovieClip_addCharacter(player, bitmapID,
						   (ddCharacter*)dd_newBitmapCharacter(image));
	}
	else
		skipBytes(r, end - ddReader_getOffset(r));
	
	ddReader_setOffset(r, end);
}


#ifdef HAVE_ZLIB

#include <zlib.h>

void
readDefineBitsJpeg3(ddMovieClip* player, ddReader* r, int length)
{
	int bitmapID = readUInt16(r);
	int offset = readUInt32(r);
	int end = ddReader_getOffset(r) + length - 6;

	int datasize;
	unsigned long size;
	unsigned char* alpha;
	unsigned char* data;

	int i;
	int y;
	int x;

	int width;
	int height;

	int res;

	ddImage* image = readJpegImage(r, offset);

	if ( image == NULL )
	{
		skipBytes(r, end - ddReader_getOffset(r));
		return;
	}

	width = ddImage_getWidth(image);
	height = ddImage_getHeight(image);

	datasize = end - ddReader_getOffset(r);
	data = readBlock(r, datasize);

	size = width * height;

	size += 248; /* XXX - ??? */
	size += 1024;

	alpha = dd_malloc(size);

	if ( (res = uncompress(alpha, &size, data, datasize)) != Z_OK )
		dd_error("Couldn't uncompress bits! (err: %i)\n", res);

	dd_free(data);
	
	data = ddImage_getData(image);
	
	for ( i = 0, y = 0; y < height; ++y )
	{
		ddColor* row = (ddColor*)(data + y * image->rowstride);

		for ( x = 0; x < width; ++x, ++i )
		{
			if ( alpha[i] < 0xff )
				row[x] = ddColor_scale(row[x], alpha[i]);
		}
	}

	ddMovieClip_addCharacter(player, bitmapID,
						  (ddCharacter*)dd_newBitmapCharacter(image));

	ddReader_setOffset(r, end);
}

#else /* HAVE_ZLIB */

void
readDefineBitsJpeg3(ddMovieClip* player, ddReader* r, int length)
{
	skipBytes(r, length);
}

#endif /* HAVE_ZLIB */

#else /* HAVE_JPEGLIB */

#include "readjpeg.h"

void
readDefineBitsJpeg(ddMovieClip* player, ddReader* r, int length)
{
	skipBytes(r, length);
}

void
readDefineBitsJpeg2(ddMovieClip* player, ddReader* r, int length)
{
	skipBytes(r, length);
}

void
readDefineBitsJpeg3(ddMovieClip* player, ddReader* r, int length)
{
	skipBytes(r, length);
}

#endif /* HAVE_JPEGLIB */

