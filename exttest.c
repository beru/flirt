
#include <stdlib.h>
#include <stdio.h>
//#include <math.h>

#include "flirt-debug.h"

#define WIDTH 400
#define HEIGHT 300

void
setupImageClass(ddPlayer* player);

int frames = 0;

void
bail(char* msg)
{
	printf("%s\n", msg);
	exit(-1);
}

void
logmsg(const char* msg, void* data)
{
	printf("%s\n", msg);
}


int main(int argc, const char* argv[])
{
	FILE* f;
	ddPlayer* player;
	ddImage* image;

	if ( argc == 1 )
		bail("usage: test <foo.swf>\nAttempts to parse the named file");

	f = fopen(argv[1], "rb");

	if ( f == NULL )
		bail("couldn't open input file");

	if ( !dd_init() )
		bail("dd_init() failed!");
	
	image = dd_newImage(WIDTH, HEIGHT);

	player = dd_newPlayer_file(f, image);

	setupImageClass(player);

	dd_setActionTraceFunction(player, logmsg, NULL);

	if ( ddPlayer_readMovie(player) != 0 )
		bail("ddPlayer_readMovie() failed");

	ddPlayer_step(player);

	while ( ddPlayer_executeFrameActions(player, STEP_FRAME) )
		;

	ddPlayer_updateDisplay(player, NULL, NULL);

	dd_destroyPlayer(player);
	dd_destroyImage(image);

	fclose(f);

	return 0;
}
