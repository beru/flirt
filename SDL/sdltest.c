
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include "flirt-debug.h"

#include "SDL/SDL.h"
#include "SDL/SDL_audio.h"

#include "ringbuffer.h"

#define WIDTH 320
#define HEIGHT 240

static SDL_Surface *screen;

void doNothing(char *msg, ...)
{
}


void sdlerror(char* msg)
{
	fputs(msg, stderr);
	fputs(": ", stderr);
	fputs(SDL_GetError(), stderr);
	fputs("\n", stderr);
	
	exit(1);
}


int playing = 0;
struct ringBuffer* ring = NULL;

void
mixaudio(void* unused, Uint8* stream, int len)
{
	ringBuffer_writeSamplesToStream(ring, stream, len / 4);
}


void
dd_initSDL()
{
	/* Initialize the SDL library */
	if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE) < 0 )
		sdlerror("Couldn't initialize SDL");
	
	/* Set video mode */
	screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_HWSURFACE);
	
	if ( screen == NULL )
		sdlerror("Couldn't set SDL video mode");

	SDL_AudioSpec fmt;
	
	fmt.freq = 22100; //44000;
	fmt.format = AUDIO_S16MSB; //AUDIO_S16;
	fmt.channels = 2;
	fmt.samples = 2048;
	fmt.callback = mixaudio;
	fmt.userdata = NULL;

	ring = newRingBuffer(44000);
	
	/* Open the audio device and start playing sound! */
	if ( SDL_OpenAudio(&fmt, NULL) < 0 )
		sdlerror("Unable to open audio");
}


void
dd_runSDL(ddPlayer* player, ddImage* image)
{
	SDL_Event event;
	
	int done = 0;
	float framerate;
	int usecPerFrame;
	struct timeval tv;
	long lastupdate;
	long delay;
	
	int firstFrame = 1;

	// XXX - readMovie() clears image with background color. Ick.
	if ( SDL_LockSurface(screen) < 0)
		sdlerror("Couldn't lock the display surface");

	ddImage_setBuffer(image, screen->pixels);
	
	ddPlayer_readMovie(player);
	
	SDL_UnlockSurface(screen);

	framerate = ddPlayer_getFrameRate(player);
	usecPerFrame = 1000000 / framerate;
	
	gettimeofday(&tv, NULL);
	lastupdate = tv.tv_sec * 1000000 + tv.tv_usec;

	while ( !done )
	{
		int i;
		ddRect* rects;
		int count;
		long now;
		
		if ( SDL_LockSurface(screen) < 0)
			sdlerror("Couldn't lock the display surface");
		
		ddImage_setBuffer(image, screen->pixels);
		
		ddPlayer_step(player);

		while ( ddPlayer_executeFrameActions(player, STEP_FRAME) )
			;
		
		ddPlayer_updateDisplay(player, &rects, &count);
		
		SDL_UnlockSurface(screen);
		
		int framesize = 44000 / framerate; // XXX - slippage
		int length = framesize;
		
		printf("render audio: %i\n", framesize);
		
		ddAudioSample* buffer = ringBuffer_getWriteBuffer(ring);

		ddPlayer_renderFrameSoundIntoBuffer(player, buffer, &length);
		
		ringBuffer_advanceWriteOffset(ring, framesize);
		ringBuffer_setSurplus(ring, length - framesize);
		
		if ( ringBuffer_samplesRemaining(ring) < framesize )
			ringBuffer_loop(ring);
		
		printf("render audio returned: %i\n", length);
		
		if ( !playing )
		{
			SDL_PauseAudio(0);
			playing = 1;
		}
		
		if ( firstFrame )
		{
			SDL_UpdateRect(screen, 0, 0, WIDTH, HEIGHT);
			firstFrame = 0;
		}
		else
		{
			for ( i = 0; i < count; ++i )
			{
				ddRect rect = rects[i];
				
				SDL_UpdateRect(screen, INT_F(rect.left), INT_F(rect.top),
							   INT_F(rect.right - rect.left),
							   INT_F(rect.bottom - rect.top));
			}
		}
		
		/* XXX - prolly want to toss extra mouse moves */
		
		while ( SDL_PollEvent(&event) )
		{
			switch ( event.type )
			{
				case SDL_MOUSEMOTION:
					ddPlayer_doMouseMove(player, event.motion.x, event.motion.y);
					break;
					
				case SDL_MOUSEBUTTONDOWN:
					/* button number in event.button.button */
					ddPlayer_doMouseDown(player, event.button.x, event.button.y);
					break;
					
				case SDL_MOUSEBUTTONUP:
					ddPlayer_doMouseUp(player, event.button.x, event.button.y);
					break;
					
				case SDL_KEYDOWN:
					if ( event.key.keysym.sym == SDLK_ESCAPE )
						done = 1;
					
					ddPlayer_doKeyDown(player, event.key.which);
					break;
					
				case SDL_KEYUP:
					ddPlayer_doKeyUp(player, event.key.which);
					break;
				
				case SDL_QUIT:
					done = 1;
					break;
				
				//default:
				//	printf("unhandled event type %i\n", event.type);
			}
		}
		
		gettimeofday(&tv, NULL);
		now = tv.tv_sec * 1000000 + tv.tv_usec;
		delay = lastupdate + usecPerFrame - now;
		
		static int frame = 0;
		printf("frame %i: %i.%06i\n", ++frame, (int)tv.tv_sec, tv.tv_usec);
		
		if ( delay > 0 )
			usleep(delay);
		else
			delay = 0;
		
		lastupdate = now + delay;
	}
}


int
main(int argc, char *argv[])
{
	FILE* f;
	ddPlayer* player;
	ddImage* image = dd_newImage(WIDTH, HEIGHT);
	
	if ( argc == 1 )
	{
		printf("usage: test <foo.swf>\nAttempts to parse the named file\n");
		exit(-1);
	}
	
	f = fopen(argv[1], "rb");
	
	if ( f == NULL )
	{
		printf("couldn't open file %s\n", argv[1]);
		exit(-1);
	}
	
	if ( !dd_init() )
	{
		printf("dd_init() failed\n");
		exit(-1);
	}
	
	//dd_setWarnFunction(doNothing);
	
	dd_initSDL();
	
	player = dd_newPlayer_file(f, image);
	dd_setActionTraceFunction(player, (void *)puts, NULL);

	dd_runSDL(player, image);
	
	dd_destroyPlayer(player);

	SDL_CloseAudio();	
	SDL_Quit();
	
	return 0;
}
