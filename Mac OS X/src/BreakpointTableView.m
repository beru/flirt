/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#import "BreakpointTableView.h"

@implementation BreakpointTableView

#define GUTTER_WIDTH 20

- (void)awakeFromNib
{
	breakpoints = [[NSMutableSet alloc] init];
}


- (void)dealloc
{
	[breakpoints release];
	[super dealloc];
}


- (void)drawMarkerAtRow:(int)row
{
	float rowHeight = [self rowHeight] + [self intercellSpacing].height;
	
	[[NSColor redColor] set];

	[NSBezierPath fillRect:NSMakeRect(1, row * rowHeight + 1, GUTTER_WIDTH - 2, rowHeight - 2)];
}


- (void)drawRect:(NSRect)rect
{
	NSRect frame = [self frame];
	NSRect oldframe = frame;
	
	frame.size.width -= GUTTER_WIDTH - rect.origin.x;
	frame.origin.x = GUTTER_WIDTH;
	[self setFrame:frame];
	
	[super drawRect:rect];
	
	[self setFrame:oldframe];
	
	[[NSColor whiteColor] set];
	[NSBezierPath fillRect:NSMakeRect(0, 0, GUTTER_WIDTH, rect.size.height)];

	[[NSColor controlShadowColor] set];
	
	[NSBezierPath strokeLineFromPoint:NSMakePoint(GUTTER_WIDTH + 0.5, 0)
							  toPoint:NSMakePoint(GUTTER_WIDTH + 0.5, rect.size.height)];

	int i;
	float rowHeight = [self rowHeight] + [self intercellSpacing].height;
	
	for ( i = rect.origin.y / rowHeight; i <= NSMaxY(rect) / rowHeight; ++i )
	{
		if ( [breakpoints containsObject:[NSNumber numberWithInt:i]] )
			[self drawMarkerAtRow:i];
	}
}


- (BOOL)isFlipped
{
	return YES;
}


- (void)setDelegate:(id)inDelegate
{
	delegate = inDelegate;
}


- (id)delegate
{
	return delegate;
}


- (void)mouseDown:(NSEvent*)event
{
	NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
	
	if ( point.x >= GUTTER_WIDTH )
	{
		NSPoint location = [event locationInWindow];
		location.x += GUTTER_WIDTH;
		
		event = [NSEvent mouseEventWithType:[event type] location:location modifierFlags:[event modifierFlags] timestamp:[event timestamp] windowNumber:[event windowNumber] context:[event context] eventNumber:[event eventNumber] clickCount:[event clickCount] pressure:[event pressure]];

		[super mouseDown:event];
		return;
	}

	float rowHeight = [self rowHeight] + [self intercellSpacing].height;
	NSNumber* row = [NSNumber numberWithInt:(point.y / rowHeight)];

	if ( [breakpoints containsObject:row] )
	{
		[breakpoints removeObject:row];
		
		if ( [delegate respondsToSelector:@selector(breakpointList:breakpointCleared:)] )
			[delegate breakpointList:self breakpointCleared:[row intValue]];
	}
	else
	{
		[breakpoints addObject:row];
		
		if ( [delegate respondsToSelector:@selector(breakpointList:breakpointSet:)] )
			[delegate breakpointList:self breakpointSet:[row intValue]];
	}
	
	[self setNeedsDisplay:YES]; // XXX - could be more specific
}


- (void)setBreakpoints:(NSSet*)inBreakpoints
{
	[breakpoints release];
	breakpoints = [inBreakpoints mutableCopy];
}

@end
