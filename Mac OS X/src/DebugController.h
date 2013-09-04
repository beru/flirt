/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#import <AppKit/AppKit.h>

@class BreakpointTableView;
@class FlashView;
@class SWFAction;
@class SWFActionObject;
@class SWFPlayer;

@interface DebugController : NSWindowController
{
	IBOutlet NSWindow* flashWindow;
	IBOutlet FlashView* flashView;

	IBOutlet NSOutlineView* stageList;
	IBOutlet NSOutlineView* fileList;
	IBOutlet NSOutlineView* variablesList;
	IBOutlet NSTableView* framesList;

	IBOutlet NSView* swapView;
	IBOutlet NSView* debuggerView;
	IBOutlet NSView* consoleView;
	IBOutlet NSView* actionView;
	IBOutlet BreakpointTableView* actionList;
	
	SWFAction* selectedAction; // action to display in console view

	SWFActionObject* selectedObject; // object to display in variables list

	NSToolbar* toolbar;
	
	SWFPlayer* player;
	NSTimer* timer;
}

@end
