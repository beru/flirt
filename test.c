
#include <stdio.h>
#include <math.h>
#include <png.h>

#include "flirt-debug.h"

#define WIDTH 1024
#define HEIGHT 768

int frames = 0;

void
bail(char* msg)
{
  printf("%s\n", msg);
  exit(-1);
}

void
doNothing(char* msg, ...)
{
}

void
traceFunction(const char* msg, void* data)
{
  puts(msg);
}


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

  if (setjmp(png_jmpbuf(png_ptr)))
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
    row_pointers[i] = imagebytes + i * rowstride;

  png_set_rows(png_ptr, info_ptr, row_pointers);

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

int main(int argc, const char* argv[])
{
  FILE* f;
  ddPlayer* player;
  ddImage* image;
  int i;

  if ( argc == 1 )
    bail("usage: test <foo.swf>\nAttempts to parse the named file");

  f = fopen(argv[1], "rb");

  if ( f == NULL )
    bail("couldn't open input file");

  if ( !dd_init() )
    bail("dd_init() failed!");

  image = dd_newImage(WIDTH, HEIGHT);

  player = dd_newPlayer_file(f, image);

  dd_setActionTraceFunction(player, traceFunction, NULL);

  if ( ddPlayer_readMovie(player) != 0 )
    bail("ddPlayer_readMovie() failed");

  for ( i = 0; i < 100; ++i )
  {
    ddPlayer_step(player);

    while ( ddPlayer_executeFrameActions(player, STEP_FRAME) )
      ;

    ddPlayer_updateDisplay(player, NULL, NULL);
  }

  savePNG("test.png", image, WIDTH, HEIGHT);

  dd_destroyPlayer(player);
  dd_destroyImage(image);

  fclose(f);

  return 0;
}
