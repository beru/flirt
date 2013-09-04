/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#import <AppKit/AppKit.h>

@interface BreakpointTableView : NSTableView
{
	id delegate;

	NSMutableSet* breakpoints;
}

- (void)setDelegate:(id)inDelegate;
- (id)delegate;

- (void)setBreakpoints:(NSSet*)breakpoints;

@end

@interface NSObject (BreakpointTableViewDelegate)
- (void)breakpointList:(BreakpointTableView*)list breakpointSet:(int)line;
- (void)breakpointList:(BreakpointTableView*)list breakpointCleared:(int)line;
@end
