
#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <png.h>

#include "flirt-debug.h"

void
savePNG(char* filename, ddImage* image, int width, int height)
{
	FILE* f = fopen(filename, "wb");
	int i;
	png_bytep* row_pointers;
	char* imagebytes = ddImage_getData(image);
	int rowstride = ddImage_getRowstride(image);
	
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, png_voidp_NULL, png_voidp_NULL, png_voidp_NULL);
	
	png_infop info_ptr = png_create_info_struct(png_ptr);
	
	if ( setjmp(png_jmpbuf(png_ptr)) )
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(f);
		return;
	}
	
	png_init_io(png_ptr, f);
	png_set_write_status_fn(png_ptr, png_write_status_ptr_NULL);
	
	png_set_IHDR(png_ptr, info_ptr, width, height,
				 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
	row_pointers = png_malloc(png_ptr, height * sizeof(png_bytep));
	
	for ( i = 0; i < height; ++i )
		row_pointers[i] = (unsigned char*)imagebytes + i * rowstride;
	
	png_set_rows(png_ptr, info_ptr, row_pointers);
	
	png_set_swap_alpha(png_ptr);

	// XXX - should be endian check
#ifndef DD_PLATFORM_MAC_OS_X
	png_set_swap_alpha(png_ptr);
	png_set_bgr(png_ptr);
#endif
	
	png_write_png(png_ptr, info_ptr, 0, NULL);
	
	png_destroy_info_struct(png_ptr, &info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	
	fclose(f);
}


struct imageObject
{
	ddActionObject parent;
	ddDrawClip* clip;
	int width;
	int height;
};

typedef struct imageObject ImageObject;

ddActionValue
Image_constructor(ddActionObject* classref, ddActionContext* context, int nargs)
{
	ddActionValue value = ddActionContext_popValue(context);
	ddActionObject* object = ddActionValue_getObjectValue(value);
	ImageObject* image;
	
	if ( !ddActionObject_isKindOfClass(object, ddActionMovieClipClass) )
		return ddNullValue;
	
	image = malloc(sizeof(struct imageObject));
	
	ddActionObject_init((ddActionObject*)image);
	
	ddActionObject_setClass((ddActionObject*)image, (ddActionClass*)ddActionObject_getPrototype(classref));
	
	image->clip = ddActionMovieClip_getDrawClip((ddActionMovieClip*)ddActionObject_retain(object));
	image->width = 0;
	image->height = 0;
	
	return dd_newActionValue_object((ddActionObject*)image);
}


ddActionValue
Image_setWidth(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionContext_popValue(context);
	int width = ddActionValue_getIntValue(val);
	
	ImageObject* image = (ImageObject*)object;
	
	image->width = width;
	
	return ddNullValue;
}

ddActionValue
Image_setHeight(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionContext_popValue(context);
	int height = ddActionValue_getIntValue(val);
	
	ImageObject* image = (ImageObject*)object;
	
	image->height = height;
	
	return ddNullValue;
}

ddActionValue
Image_render(ddActionObject* object, ddActionContext* context, int nargs)
{
	ddActionValue val = ddActionContext_popValue(context);
	char* filename = ddActionValue_getStringValue(context, val);
	ImageObject* image = (ImageObject*)object;
	ddImage* bitmap = dd_newImage(image->width, image->height);
	
	if ( filename == NULL || image->width == 0 || image->height == 0 )
		return ddFalseValue;
	
	ddDrawClip_renderInImage(image->clip, bitmap, ddPlayer_getBackgroundColor(ddContext_getPlayer(context)));
	
	savePNG(filename, bitmap, image->width, image->height);
	
	return ddTrueValue;
}


void
setupImageClass(ddPlayer* player)
{
	ddActionClass* class = ddPlayer_addClass(player, ddActionObjectClass, "Image", Image_constructor, 1);
	
	ddActionClass_addNativeMethod(class, "render", Image_render, 1);
	ddActionClass_addNativeMethod(class, "setWidth", Image_setWidth, 1);
	ddActionClass_addNativeMethod(class, "setHeight", Image_setHeight, 1);
}
