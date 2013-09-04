/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#import "FlashView.h"

@implementation FlashView

- (NSCursor*)fingerCursor
{
	static NSCursor* cursor = nil;

	if ( cursor == nil )
		cursor = [[NSCursor alloc] initWithImage:[NSImage imageNamed:@"fingerCursor"]
													 hotSpot:NSMakePoint(0.0,0.0)];

	return cursor;
}


- (id)initWithFrame:(NSRect)frame
{
    if ( [super initWithFrame:frame] == nil )
		return nil;

	bitmap = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:nil pixelsWide:NSWidth(frame) pixelsHigh:NSHeight(frame) bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSDeviceRGBColorSpace bytesPerRow:0 bitsPerPixel:0];

	bitmapImage = [[NSImage alloc] init];
	[bitmapImage addRepresentation:bitmap];
	[bitmapImage setSize:frame.size];
	[bitmapImage setScalesWhenResized:YES];
	[bitmapImage setCacheMode:NSImageCacheNever];
	
	image = dd_newImageFromBuffer([bitmap bitmapData], NSWidth(frame), [bitmap bytesPerRow], NSHeight(frame));
	ddImage_clear(image, (ddColor)0xffffffff);

    return self;
}


- (void)dealloc
{
	dd_destroyImage(image);
	
	[bitmap release];
	[bitmapImage release];
	
	[super dealloc];
}


- (BOOL)acceptsFirstResponder
{
	return YES;
}


- (id)delegate
{
	return delegate;
}


- (void)drawRect:(NSRect)rect
{
	[bitmapImage drawInRect:rect fromRect:rect operation:NSCompositeCopy fraction:1.0];

	if ( NSWidth(highlightRect) > 0 && NSHeight(highlightRect) > 0 )
	{
		[[NSColor yellowColor] set];
		[NSBezierPath strokeRect:highlightRect];
	}
}


- (ddImage*)image
{
	return image;
}


- (void)mouseDown:(NSEvent*)event
{
	if ( [delegate respondsToSelector:@selector(flashView:mouseDown:)] )
		[(NSObject*)delegate flashView:self mouseDown:event];
}


- (void)mouseDragged:(NSEvent*)event
{
	[self mouseMoved:event];
}


- (void)mouseMoved:(NSEvent*)event
{
	if ( [delegate respondsToSelector:@selector(flashView:mouseMoved:)] )
		[(NSObject*)delegate flashView:self mouseMoved:event];
}


- (void)mouseUp:(NSEvent*)event
{
	if ( [delegate respondsToSelector:@selector(flashView:mouseUp:)] )
		[(NSObject*)delegate flashView:self mouseUp:event];
}


- (void)keyDown:(NSEvent*)event
{
	if ( [delegate respondsToSelector:@selector(flashView:keyDown:)] )
		[(NSObject*)delegate flashView:self keyDown:event];
}


- (void)keyUp:(NSEvent*)event
{
	if ( [delegate respondsToSelector:@selector(flashView:keyUp:)] )
		[(NSObject*)delegate flashView:self keyUp:event];
}


- (void)resizeWithOldSuperviewSize:(NSSize)oldBoundsSize
{
	// XXX - never seems to be called
	[bitmapImage setSize:[self bounds].size];
	[super resizeWithOldSuperviewSize:oldBoundsSize];
}


- (void)setDelegate:(id)inDelegate
{
	delegate = inDelegate;
}


- (void)setHighlightRect:(NSRect)rect
{
	rect.origin.y = NSHeight([self frame]) - rect.origin.y - rect.size.height;
	
	highlightRect = rect;
	[self setNeedsDisplay:YES];
}


- (void)viewDidEndLiveResize
{
	NSRect frame = [self frame];
	
	[bitmapImage removeRepresentation:bitmap];
	[bitmap release];
	
	bitmap = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:nil pixelsWide:NSWidth(frame) pixelsHigh:NSHeight(frame) bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSDeviceRGBColorSpace bytesPerRow:0 bitsPerPixel:0];
	
	[bitmapImage addRepresentation:bitmap];
	[bitmapImage setSize:[self frame].size];
	
	[self setNeedsDisplay:YES];

	dd_destroyImage(image);
	image = dd_newImageFromBuffer([bitmap bitmapData], NSWidth(frame), [bitmap bytesPerRow], NSHeight(frame));
	ddImage_clear(image, (ddColor)0xffffffff);
	
	if ( [delegate respondsToSelector:@selector(flashView:didReplaceImage:)] )
		[(NSObject*)delegate flashView:self didReplaceImage:image];
}


- (void)clearHighlightRect
{
	highlightRect = NSZeroRect;
	[self setNeedsDisplay:YES];
}

@end
