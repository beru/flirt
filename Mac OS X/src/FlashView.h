/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#import <AppKit/AppKit.h>

#import <flirt.h>

@interface FlashView : NSView
{
	id delegate;
	
	NSBitmapImageRep* bitmap;
	NSImage* bitmapImage;
	
	ddImage* image;

	NSRect highlightRect;
}

- (void)setDelegate:(id)delegate;
- (id)delegate;

- (ddImage*)image;

- (NSCursor*)fingerCursor;

- (void)setHighlightRect:(NSRect)rect;
- (void)clearHighlightRect;

@end

@interface NSObject (FlashViewDelegate)
- (void)flashView:(FlashView*)view didReplaceImage:(ddImage*)image;
- (void)flashView:(FlashView*)view mouseDown:(NSEvent*)event;
- (void)flashView:(FlashView*)view mouseUp:(NSEvent*)event;
- (void)flashView:(FlashView*)view mouseMoved:(NSEvent*)point;
- (void)flashView:(FlashView*)view keyUp:(NSEvent*)event;
- (void)flashView:(FlashView*)view keyDown:(NSEvent*)event;
@end
