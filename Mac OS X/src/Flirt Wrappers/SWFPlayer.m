/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

/*
 //#include <CoreAudio/CoreAudio.h>
	//AudioDeviceID outputDevID;

 OSStatus
 waveIOProc(AudioDeviceID inDevice,
			const AudioTimeStamp* inNow,
			const AudioBufferList* inInputData,
			const AudioTimeStamp* inInputTime,
			AudioBufferList* outOutputData,
			const AudioTimeStamp* inOutputTime,
			void* inContext)
 {
	 float* buffer = outOutputData->mBuffers[0].mData;
	 UInt32 bufferSize = outOutputData->mBuffers[0].mDataByteSize;
	 UInt32 numChannels = outOutputData->mBuffers[0].mNumberChannels;
	 
	 ddSoundMixer* mixer = (ddSoundMixer*)inContext;
	 
	 if ( !dd_mixerCallback(buffer, bufferSize) )
	 {
		 dd_warn("audioCallback underrun!");
		 ddSoundMixer_stop(mixer);
		 return kAudioHardwareUnspecifiedError;
	 }
	 
	 return kAudioHardwareNoError;
 }
 
 {
	 AudioDeviceID outputDevID;
	 ddSoundMixer* mixer = dd_malloc(sizeof(ddSoundMixer));
	 
	 OSStatus status;
	 UInt32 size;
	 
	 size = sizeof(outputDevID);
	 status = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &size, &outputDevID);
	 
	 if ( status )
	 {
		 dd_warn("AudioHardwareGetProperty status %d", status);
		 return NULL;
	 }
	 
	 size = sizeof(bufferByteCount);
	 bufferByteCount = MIXER_BUFFER_SIZE * sizeof(float);
	 
	 status = AudioDeviceSetProperty(outputDevID, NULL, 0, DD_FALSE, kAudioDevicePropertyBufferSize, size, &bufferByteCount);
	 
	 if ( status )
	 {
		 dd_warn("AudioDeviceSetProperty setting buffer size status %d", status);
		 return NULL;
	 }
	 
	 status = AudioDeviceAddIOProc(outputDevID, waveIOProc, mixer);
	 
	 if ( status )
	 {
		 dd_warn("AudioDeviceAddIOProc status %d", status);
		 return NULL;
	 }
	 
	 mixer->outputDevID = outputDevID;
	 mixer->playing = DD_FALSE;
	 
	 return mixer;
 }

 void
 ddSoundMixer_start(ddSoundMixer* mixer)
 {
#ifdef DD_PLATFORM_MAC_OS_X
	 OSStatus status;
	 
	 status = AudioDeviceStart(mixer->outputDevID, waveIOProc);
	 
	 if ( status )
		 dd_warn("AudioDeviceStop status %d", status);
	 
	 mixer->playing = DD_TRUE;
#endif DD_PLATFORM_MAC_OS_X
 }
 
 
 void
 ddSoundMixer_stop(ddSoundMixer* mixer)
 {
#ifdef DD_PLATFORM_MAC_OS_X
	 OSStatus status;
	 
	 status = AudioDeviceStop(mixer->outputDevID, waveIOProc);
	 
	 if ( status )
		 dd_warn("AudioDeviceStop status %d", status);
	 
	 mixer->playing = DD_FALSE;
#endif DD_PLATFORM_MAC_OS_X
 }
 
 */


#import "FlashView.h"
#import "SWFAction.h"
#import "SWFActionObject.h"
#import "SWFDrawable.h"
#import "SWFMovieClip.h"
#import "SWFPlayer.h"

void
setupImageClass(ddPlayer* player);

@implementation SWFPlayer

+ (id)player
{
	return [[[self alloc] init] autorelease];
}


- (id)init
{
	if ( (self = [super init]) == nil )
		return nil;
	
	player = nil;

	return self;
}


- (void)dealloc
{
	[root release];
	[globals release];
	
	dd_destroyPlayer(player);

	[super dealloc];
}


- (ddPlayer*)ddPlayer
{
	return player;
}


- (SWFMovieClip*)movieClip
{
	return [SWFMovieClip clipWithDDMovieClip:ddPlayer_getMovieClip(player)];
}


- (SWFDrawable*)drawClip
{
	return [SWFDrawable drawableWithDDDrawable:(ddDrawable*)ddPlayer_getPlayClip(player)];
}


- (float)frameRate
{
	return ddPlayer_getFrameRate(player);
}


- (void)keyDown:(unichar)c
{
	ddPlayer_doKeyDown(player, c);
}


- (void)keyUp:(unichar)c
{
	ddPlayer_doKeyUp(player, c);
}


- (BOOL)loadMovieAtPath:(NSString*)path forFlashView:(FlashView*)view
{
	ddImage* image = [view image];
	FILE* f = fopen([path cString], "r");
	player = dd_newPlayer_file(f, image);

	if ( ddPlayer_readMovie(player) < 0 )
		return NO;

	setupImageClass(player);
	
	ddImage_clear(image, ddPlayer_getBackgroundColor(player));

	return YES;
}


- (void)mouseDown:(NSPoint)point
{
	ddPlayer_doMouseDown(player, point.x, point.y);
}


- (void)mouseUp:(NSPoint)point
{
	ddPlayer_doMouseUp(player, point.x, point.y);
}


- (ddCursorType)mouseMoved:(NSPoint)point
{
	return ddPlayer_doMouseMove(player, point.x, point.y);
}


- (void)replaceImage:(ddImage*)image
{
	ddPlayer_setImage(player, image);
}


- (void)setTraceFunction:(void*)func userData:(void*)data
{
	dd_setActionTraceFunction(player, func, data);
}


- (SWFActionObject*)globals
{
	if ( globals == nil )
		globals = [[SWFActionObject objectWithDDActionObject:ddPlayer_getGlobals(player)] retain];
	
	return globals;
}


- (SWFActionObject*)root
{
	if ( root == nil )
		root = [[SWFActionObject objectWithDDActionObject:(ddActionObject*)ddPlayer_getRootClip(player)] retain];

	return root;
}


- (SWFAction*)currentAction
{
	return [self actionAtFrameDepth:0];
}


- (int)actionOffsetAtFrameDepth:(int)depth
{
	ddActionContext* context = ddPlayer_actionContextAtDepth(player, depth);
	return ddActionContext_getActionOffset(context);
}


- (int)actionStackDepth
{
	return ddPlayer_actionStackDepth(player);
}


- (NSString*)actionNameAtFrameDepth:(int)depth
{
	ddActionContext* context = ddPlayer_actionContextAtDepth(player, depth);
	char* name = ddActionContext_getLabel(context);
	
	if ( name != NULL )
		return [NSString stringWithCString:name];
	else
		return @"no name";
}


- (SWFAction*)actionAtFrameDepth:(int)depth
{
	ddActionContext* context = ddPlayer_actionContextAtDepth(player, depth);
	
	if ( context != NULL )
		return [SWFAction actionWithDDAction:ddActionContext_getAction(context)];
	else
		return nil;
}


- (BOOL)stepFrame:(ddRect**)rectsPtr nRects:(int*)countPtr mode:(ddStepMode)mode
{
	if ( !hitBreakpoint )
		ddPlayer_step(player);
	
	if ( ddPlayer_executeFrameActions(player, mode) )
	{
		// hit a breakpoint
		hitBreakpoint = YES;
		
		return NO;
	}
	else
	{
		ddPlayer_updateDisplay(player, rectsPtr, countPtr);

		[globals release];
		globals = nil;

		[root release];
		root = nil;

		hitBreakpoint = NO;
		
		SInt16 soundbuffer[2 * 44000];
		int size = 2 * 44000;
		
		ddPlayer_renderFrameSoundIntoBuffer(player, soundbuffer, &size);
		
		return YES;
	}
}

@end
