/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#import <Foundation/Foundation.h>

#import <flirt-debug.h>

@class SWFAction;
@class SWFActionObject;
@class SWFDrawable;
@class SWFMovieClip;
@class FlashView;

@interface SWFPlayer : NSObject
{
	ddPlayer* player;

	SWFActionObject* root;
	SWFActionObject* globals;
	
	BOOL hitBreakpoint;
}

+ (id)player;

- (BOOL)loadMovieAtPath:(NSString*)path forFlashView:(FlashView*)view;

- (ddPlayer*)ddPlayer;

- (SWFMovieClip*)movieClip;
- (SWFDrawable*)drawClip;

- (float)frameRate;

- (BOOL)stepFrame:(ddRect**)rectsPtr nRects:(int*)countPtr mode:(ddStepMode)mode;

- (void)keyDown:(unichar)c;
- (void)keyUp:(unichar)c;
- (void)mouseDown:(NSPoint)point;
- (void)mouseUp:(NSPoint)point;
- (ddCursorType)mouseMoved:(NSPoint)point;

- (void)setTraceFunction:(void*)func userData:(void*)data;

- (SWFActionObject*)root;
- (SWFActionObject*)globals;

- (SWFAction*)currentAction;
- (int)actionOffsetAtFrameDepth:(int)depth;
- (SWFAction*)actionAtFrameDepth:(int)depth;

- (int)actionStackDepth;
- (NSString*)actionNameAtFrameDepth:(int)depth;

- (void)replaceImage:(ddImage*)image;

@end
