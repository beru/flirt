/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#import "BreakpointTableView.h"
#import "DebugController.h"
#import "FlashView.h"
#import "LocaleMacros.h"
#import "SWFAction.h"
#import "SWFActionObject.h"
#import "SWFActionValue.h"
#import "SWFDrawable.h"
#import "SWFMovieClip.h"
#import "SWFTimelineItem.h"
#import "SWFPlayer.h"

#import <flirt.h>

@interface DebugController (Private)
- (void)showAction:(SWFAction*)action;
- (void)setSelectedObject:(SWFActionObject*)object;
@end

@implementation DebugController

void logmsg(char* str, ...)
{
	va_list args;
	va_start(args, str);
	NSLog([[[NSString alloc] initWithFormat:[NSString stringWithCString:str] arguments:args] autorelease]);
	va_end(args);
}

+ (id)debugController
{
	return [[[self alloc] init] autorelease];
}


- (id)init
{
	if ( (self = [super initWithWindowNibName:@"Debugger"]) == nil )
		return nil;

	player = nil;

	[[self window] makeKeyAndOrderFront:self];
	[flashWindow setAcceptsMouseMovedEvents:YES];

	toolbar = [[[NSToolbar alloc] initWithIdentifier:@"MessageComposerToolbar"] autorelease];

    [toolbar setAllowsUserCustomization:YES];
    [toolbar setAutosavesConfiguration:YES];
    [toolbar setDisplayMode:NSToolbarDisplayModeIconAndLabel];

    [toolbar setDelegate:self];

    [[self window] setToolbar:toolbar];

	[debuggerView setFrame:[swapView bounds]];
	[swapView addSubview:debuggerView];

	[actionView setFrame:[consoleView bounds]];
	[consoleView addSubview:actionView];

	/*
	NSRect rect = [actionList frame];
	rect.origin.x += 15;
	rect.size.width -= 15;
	[actionList setFrame:rect];
	 */
	
	[actionList setDoubleAction:@selector(actionDoubleClicked:)];
	[actionList setTarget:self];
	
	return self;
}


- (void)dealloc
{
	[player release];
	[super dealloc];
}


- (void)actionDoubleClicked:(id)sender
{
	int row = [actionList selectedRow];
	SWFAction* function = [selectedAction functionAtOffset:row];
	
	if ( function != nil )
		[self showAction:function];
}


- (void)continuePlaying:(id)sender
{
	// XXX - if root object is stopped, start it up
	
	float rate = [player frameRate];

	timer = [NSTimer scheduledTimerWithTimeInterval:(1.0/rate) target:self selector:@selector(advanceFrame) userInfo:nil repeats:YES];
}


- (void)advanceFrame:(ddStepMode)mode
{
	ddRect* rects;
	int count;
	int i;

	float height = NSHeight([flashView frame]);

	//static int frame = 0;
	//NSLog(@"Frame: %i", frame++);

	if ( ![player stepFrame:&rects nRects:&count mode:mode] )
	{
		// hit a breakpoint
		
		[framesList reloadData];
		[variablesList reloadData];
		
		[timer invalidate];
		timer = nil;
		
		return;
	}

	for ( i = 0; i < count; ++i )
	{
		ddRect rect = rects[i];

		NSRect r = NSMakeRect(INT_F(rect.left), height - INT_F(rect.bottom),
						INT_F(rect.right - rect.left),
						INT_F(rect.bottom - rect.top));

		[flashView setNeedsDisplayInRect:r];
	}
}


- (void)advanceFrame
{
	[self advanceFrame:STEP_FRAME];
}


- (void)nextFrame:(id)sender
{
	[self advanceFrame:STEP_FRAME];
	
	[fileList reloadData];
	[stageList reloadData];
	[variablesList reloadData];
}


- (void)loadPath:(NSString*)path
{
	player = [[SWFPlayer alloc] init];

	if ( ![player loadMovieAtPath:path forFlashView:flashView] )
	{
		[player release];

		NSRunAlertPanel(@"Error", @"Couldn't open file at %@", @"OK", nil, nil, path);
		return;
	}

	[flashWindow makeKeyAndOrderFront:self];
	
	[flashView setNeedsDisplay:YES];

	[player setTraceFunction:logmsg userData:nil];

	dd_setWarnFunction(logmsg);
}	


- (void)openDocument:(id)sender
{
	NSOpenPanel* panel = [NSOpenPanel openPanel];
	int res = [panel runModalForTypes:[NSArray arrayWithObject:@"swf"]];
	NSString* file;

	if ( res != NSOKButton )
		return;

	file = [[panel filenames] objectAtIndex:0];

	if ( !dd_init() )
	{
		NSRunAlertPanel(@"Error", @"Error initializing ddlib", @"OK", nil, nil);
		[NSApp terminate];
		return;
	}

	[self loadPath:file];
	
	[fileList reloadData];
	[stageList reloadData];
	[variablesList reloadData];
}

/*
 - (void)flashFrame
 {
	 ddRect* rects;
	 int count;
	 int i;

	 static int num = 0;

	 int height = NSHeight([self frame]);

	 if ( num++ % 2 == 0 )
		 ddPlayer_stepFlashUpdates(player);
	 else
		 ddPlayer_drawUpdates(player);

	 ddPlayer_getUpdateRects(player, &rects, &count);

	 for ( i = 0; i < count; ++i )
	 {
		 ddRect rect = rects[i];
		 NSRect r = NSMakeRect(INT_F(rect.left), height - INT_F(rect.bottom),
						 INT_F(rect.right - rect.left),
						 INT_F(rect.bottom - rect.top));

		 [self setNeedsDisplayInRect:r];
	 }
 }
 */

- (void)outlineViewSelectionDidChange:(NSNotification*)notification
{
	NSOutlineView* view = [notification object];

	if ( view == fileList )
	{
		int row = [fileList selectedRow];

		if ( row == -1 )
			return;
		
		id item = [fileList itemAtRow:row];

		if ( [item isKindOfClass:[SWFAction class]] )
			[self showAction:(SWFAction*)item];
	}
	else if ( view == stageList )
	{
		int row = [stageList selectedRow];

		if ( row == -1 )
		{
			[flashView clearHighlightRect];
		}
		else
		{
			SWFDrawable* drawable = [stageList itemAtRow:row];
			[flashView setHighlightRect:[drawable bounds]];
			
			// show movie clip properties in variablesList
			if ( [drawable type] == MOVIECLIP_CHAR )
				[self setSelectedObject:[drawable object]];
		}
	}
	else if ( view == variablesList )
	{
		int row = [variablesList selectedRow];
		
		if ( row == -1 )
			return;

		// if selected object is a script function, display code

		id item = [variablesList itemAtRow:row];

		if ( [item isKindOfClass:[NSDictionary class]] )
		{
			SWFAction* action = [(SWFActionValue*)[item objectForKey:@"Value"] functionValue];
			
			if ( action != nil )
				[self showAction:action];
		}
		else if ( [item isKindOfClass:[SWFAction class]] )
			[self showAction:(SWFAction*)item];
	}
}


- (void)tableViewSelectionDidChange:(NSNotification*)notification
{
	NSOutlineView* view = [notification object];
	
	if ( view == framesList )
	{
		int row = [framesList selectedRow];

		if ( row == -1 )
			[self showAction:nil];
		else
		{
			int depth = [player actionStackDepth] - row - 1;
			[self showAction:[player actionAtFrameDepth:depth]];
			
			int row = [player actionOffsetAtFrameDepth:depth];
			[actionList selectRow:row byExtendingSelection:NO];
			
			[actionList scrollRowToVisible:row];
		}
	}
}


- (void)setSelectedObject:(SWFActionObject*)object
{
	[selectedObject autorelease];
	selectedObject = [object retain];
	
	[variablesList reloadData];
}


- (void)pause:(id)sender
{
	[timer invalidate];
	timer = nil;

	[fileList reloadData];
	[stageList reloadData];
}


- (void)showAction:(SWFAction*)action
{
	if ( ![actionView isDescendantOf:swapView] )
	{
		[actionView setFrame:[consoleView bounds]];
		[swapView replaceSubview:[[consoleView subviews] objectAtIndex:0] with:actionView];
	}

	[selectedAction autorelease];
	selectedAction = [action retain];
	
	[actionList reloadData];
}


- (void)stepInto:(id)sender
{
	[self advanceFrame:STEP_INTO];
	
	[fileList reloadData];
	[stageList reloadData];
	[variablesList reloadData];	
}


- (void)stepOver:(id)sender
{
	[self advanceFrame:STEP_OVER];
	
	[fileList reloadData];
	[stageList reloadData];
	[variablesList reloadData];	
}


- (void)finish:(id)sender
{
	[self advanceFrame:STEP_FINISH];
	
	[fileList reloadData];
	[stageList reloadData];
	[variablesList reloadData];	
}


- (BOOL)validateToolbarItem:(NSToolbarItem*)item
{
	SEL action = [item action];
	
	if ( action == @selector(stepInto:) ||
		 action == @selector(stepInto:) )
	{
		return ([player actionStackDepth] > 0);
	}
	
	return YES;
}

@end

@implementation DebugController (ToolbarDelegate)

- (NSToolbarItem*)toolbar:(NSToolbar*)toolbar
		  itemForItemIdentifier:(NSString*)itemIdent willBeInsertedIntoToolbar:(BOOL)willBeInserted
{
	// Create items for toolbar

	NSToolbarItem* toolbarItem = [[[NSToolbarItem alloc] initWithItemIdentifier:itemIdent] autorelease];

	if ( [itemIdent isEqual:@"Open"] )
	{
		[toolbarItem setLabel:LOCAL(@"Open")];
		[toolbarItem setPaletteLabel:LOCAL(@"Open File")];

		[toolbarItem setImage:[NSImage imageNamed:@"toolbar-open"]];
		[toolbarItem setToolTip:LOCAL(@"Open a movie in the debugger")];

		[toolbarItem setAction:@selector(openDocument:)];
		[toolbarItem setTarget:self];
	}
	else if ( [itemIdent isEqual:@"Restart"] )
	{
		[toolbarItem setLabel:LOCAL(@"Restart")];
		[toolbarItem setPaletteLabel:LOCAL(@"Restart")];

		[toolbarItem setImage:[NSImage imageNamed:@"toolbar-restart"]];
		[toolbarItem setToolTip:LOCAL(@"Restart movie")];

		[toolbarItem setAction:@selector(restart:)];
		[toolbarItem setTarget:self];
	}
	else if ( [itemIdent isEqual:@"Pause"] )
	{
		[toolbarItem setLabel:LOCAL(@"Pause")];
		[toolbarItem setPaletteLabel:LOCAL(@"Pause")];

		[toolbarItem setImage:[NSImage imageNamed:@"toolbar-pause"]];
		[toolbarItem setToolTip:LOCAL(@"Pause execution")];

		[toolbarItem setAction:@selector(pause:)];
		[toolbarItem setTarget:self];
	}
	else if ( [itemIdent isEqual:@"StepFrame"] )
	{
		[toolbarItem setLabel:LOCAL(@"Next Frame")];
		[toolbarItem setPaletteLabel:LOCAL(@"Next Frame")];

		[toolbarItem setImage:[NSImage imageNamed:@"toolbar-nextframe"]];
		[toolbarItem setToolTip:LOCAL(@"Move forward a frame")];

		[toolbarItem setAction:@selector(nextFrame:)];
		[toolbarItem setTarget:self];
	}
	else if ( [itemIdent isEqual:@"Continue"] )
	{
		[toolbarItem setLabel:LOCAL(@"Continue")];
		[toolbarItem setPaletteLabel:LOCAL(@"Continue")];

		[toolbarItem setImage:[NSImage imageNamed:@"toolbar-continue"]];
		[toolbarItem setToolTip:LOCAL(@"Continue execution")];

		[toolbarItem setAction:@selector(continuePlaying:)];
		[toolbarItem setTarget:self];
	}
	else if ( [itemIdent isEqual:@"StepOver"] )
	{
		[toolbarItem setLabel:LOCAL(@"Step Over")];
		[toolbarItem setPaletteLabel:LOCAL(@"Step Over")];

		[toolbarItem setImage:[NSImage imageNamed:@"toolbar-stepover"]];
		[toolbarItem setToolTip:LOCAL(@"Step over method or function call")];

		[toolbarItem setAction:@selector(stepOver:)];
		[toolbarItem setTarget:self];
	}
	else if ( [itemIdent isEqual:@"StepInto"] )
	{
		[toolbarItem setLabel:LOCAL(@"Step Into")];
		[toolbarItem setPaletteLabel:LOCAL(@"Step Into")];

		[toolbarItem setImage:[NSImage imageNamed:@"toolbar-stepinto"]];
		[toolbarItem setToolTip:LOCAL(@"Step into method or function call")];

		[toolbarItem setAction:@selector(stepInto:)];
		[toolbarItem setTarget:self];
	}
	else if ( [itemIdent isEqual:@"StepOut"] )
	{
		[toolbarItem setLabel:LOCAL(@"Step Out")];
		[toolbarItem setPaletteLabel:LOCAL(@"Step Out")];

		[toolbarItem setImage:[NSImage imageNamed:@"toolbar-stepout"]];
		[toolbarItem setToolTip:LOCAL(@"Step out of current function or method")];

		[toolbarItem setAction:@selector(stepOut:)];
		[toolbarItem setTarget:self];
	}
	
	return toolbarItem;
}


- (NSArray*)toolbarDefaultItemIdentifiers:(NSToolbar*)toolbar
{
	// The default set of items

	return [NSArray arrayWithObjects:@"Open", NSToolbarFlexibleSpaceItemIdentifier, @"Restart", @"Pause", @"StepFrame", @"Continue", @"StepOver", @"StepInto", @"StepOut", nil];
}


- (NSArray*)toolbarAllowedItemIdentifiers:(NSToolbar*)toolbar
{
	// The set of all possible items

	return [NSArray arrayWithObjects:@"Open", @"Restart", @"Pause", @"StepFrame", @"Continue", @"StepOver", @"StepInto", @"StepOut", NSToolbarFlexibleSpaceItemIdentifier, nil];
}

@end

@implementation DebugController (FlashViewDelegate)

- (void)flashView:(FlashView*)view didReplaceImage:(ddImage*)image
{
	[player replaceImage:image];
}


- (void)flashView:(FlashView*)view mouseDown:(NSEvent*)event
{
	NSPoint point = [view convertPoint:[event locationInWindow] fromView:nil];
	point.y = NSHeight([view frame]) - point.y;

	[player mouseDown:point];
}


- (void)flashView:(FlashView*)view mouseMoved:(NSEvent*)event
{
	ddCursorType cursor;
	NSPoint point = [view convertPoint:[event locationInWindow] fromView:nil];
	point.y = NSHeight([view frame]) - point.y;
	
	cursor = [player mouseMoved:point];

	if ( cursor == DD_ARROW_CURSOR )
		[[NSCursor arrowCursor] set];
	else if ( cursor == DD_BUTTON_CURSOR )
		[[view fingerCursor] set];
	else if ( cursor == DD_TEXTFIELD_CURSOR )
		[[NSCursor IBeamCursor] set];	
}


- (void)flashView:(FlashView*)view mouseUp:(NSEvent*)event
{
	NSPoint point = [view convertPoint:[event locationInWindow] fromView:nil];
	point.y = NSHeight([view frame]) - point.y;
	
	[player mouseUp:point];
}


- (void)flashView:(FlashView*)view keyDown:(NSEvent*)event
{
	[player keyDown:[[event characters] characterAtIndex:0]];
}


- (void)flashView:(FlashView*)view keyUp:(NSEvent*)event
{
	[player keyUp:[[event characters] characterAtIndex:0]];
}

@end

@implementation DebugController (OutlineViewDataSource)

- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(id)item
{
	if ( outlineView == fileList )
	{
		if ( item == nil )
			return [[player movieClip] frameNumber:index];

		if ( [item isKindOfClass:[NSDictionary class]] )
		{
			id actions = [(NSDictionary*)item objectForKey:@"Actions"];
			
			if ( actions != nil )
			{
				if ( index == 0 )
					return actions;
				else
					--index;
			}

			return [[(NSDictionary*)item objectForKey:@"Items"] objectAtIndex:index];
		}

		if ( [item isKindOfClass:[SWFTimelineItem class]] )
			return [[(SWFTimelineItem*)item movieClip] frameNumber:index];
	}
	else if ( outlineView == stageList )
	{
		if ( item == nil )
			return [[[player drawClip] subdrawables] objectAtIndex:index];

		if ( [item isKindOfClass:[SWFDrawable class]] )
		{
			if ( [(SWFDrawable*)item type] == BUTTON_CHAR )
			{
				NSArray* actions = [(SWFDrawable*)item buttonActions];
				
				if ( index < (signed)[actions count] )
					return [[actions objectAtIndex:index] objectForKey:@"Name"];
				
				index -= [actions count];
			}
			
			return [[(SWFDrawable*)item subdrawables] objectAtIndex:index];
		}
	}
	else if ( outlineView == variablesList )
	{
		if ( item == nil )
		{
			if ( selectedObject != nil )
				return [[selectedObject values] objectAtIndex:index];
			
			if ( index == 0 )
				return @"globals";
			else
				return @"_root";
		}

		if ( [item isEqualTo:@"globals"] )
			return [[[player globals] values] objectAtIndex:index];
		
		else if ( [item isEqualTo:@"_root"] )
			return [[[player root] values] objectAtIndex:index];

		if ( [item isKindOfClass:[NSDictionary class]] )
		{
			SWFActionValue* value = [(NSDictionary*)item objectForKey:@"Value"];

			if ( value != nil && [value type] == VALUE_OBJECT )
				return [[[value object] values] objectAtIndex:index];
			else
				return [value description];
		}
	}
	
	return nil;
}


- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
	if ( [item isKindOfClass:[SWFTimelineItem class]] )
		return ([(SWFTimelineItem*)item movieClip] != nil);
	
	if ( [item isKindOfClass:[SWFDrawable class]] )
	{
		ddCharacterType type = [(SWFDrawable*)item type];
		
		return (type == MOVIECLIP_CHAR || type == BUTTON_CHAR);
	}
	
	if ( [item isKindOfClass:[SWFAction class]] )
		return NO;

	if ( outlineView == variablesList )
	{
		if ( [item isKindOfClass:[NSDictionary class]] )
		{
			ddActionValueType type = [(SWFActionValue*)[(NSDictionary*)item objectForKey:@"Value"] type];
			
			return (type == VALUE_OBJECT || type == VALUE_FUNCTION);
		}
	}
	
	return YES;
}


- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
	if ( outlineView == fileList )
	{
		if ( item == nil )
			return [[player movieClip] frameCount];
		
		if ( [item isKindOfClass:[NSDictionary class]] )
		{
			int count = [[(NSDictionary*)item objectForKey:@"Items"] count];
			
			if ( [(NSDictionary*)item objectForKey:@"Actions"] != nil )
				++count;

			return count;
		}

		if ( [item isKindOfClass:[SWFTimelineItem class]] )
			return [[(SWFTimelineItem*)item movieClip] frameCount];
	}
	else if ( outlineView == stageList )
	{
		if ( item == nil )
			return [[[player drawClip] subdrawables] count];

		if ( [item isKindOfClass:[SWFDrawable class]] )
		{
			int count = [[(SWFDrawable*)item subdrawables] count];
			
			if ( [(SWFDrawable*)item type] == BUTTON_CHAR )
				count += [[(SWFDrawable*)item buttonActions] count];
			
			return count;
		}
	}
	else if ( outlineView == variablesList )
	{
		if ( item == nil )
		{
			if ( selectedObject != nil )
				return [[selectedObject values] count];
			else
				return 2;
		}

		if ( [item isEqualTo:@"globals"] )
			return [[[player globals] values] count];
		else if ( [item isEqualTo:@"_root"] )
			return [[[player root] values] count];

		if ( [item isKindOfClass:[NSDictionary class]] )
		{
			SWFActionValue* value = [(NSDictionary*)item objectForKey:@"Value"];
			SWFActionObject* object = [value object];
			
			return [[object values] count];
		}
	}
	else if ( outlineView == stageList )
	{
		return 0;
	}
	
	return 0;
}


- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
	if ( outlineView == fileList )
	{
		if ( [item isKindOfClass:[NSDictionary class]] )
			return [(NSDictionary*)item objectForKey:@"Frame"];
		
		if ( [item isKindOfClass:[SWFTimelineItem class]] )
			return [item description];
		
		return [item class];
	}
	else if ( outlineView == stageList )
	{
		return [item description];
	}
	else if ( outlineView == variablesList )
	{
		if ( [[tableColumn identifier] isEqualToString:@"Name"] )
		{
			if ( [item isKindOfClass:[NSString class]] )
				return item;
			
			return [item objectForKey:@"Name"];
		}
		else if ( [[tableColumn identifier] isEqualToString:@"Value"] )
		{
			if ( [item isKindOfClass:[NSString class]] )
				return nil;
			else
				return [[item objectForKey:@"Value"] description];
		}
	}
	
	return [item description];
}

@end

@implementation DebugController (TableViewDataSource)

- (int)numberOfRowsInTableView:(NSTableView*)tableView
{
	if ( tableView == actionList )
	{
		if ( selectedAction != nil )
			return [selectedAction length];
		else
			return 0;
	}
	else if ( tableView == framesList )
	{
		return [player actionStackDepth];
	}
	
	return 0;
}

- (id)tableView:(NSTableView*)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
	if ( tableView == actionList )
	{
		if ( selectedAction != nil )
			return [@"    " stringByAppendingString:[selectedAction opcodeAtOffset:row]];
		else
			return nil;
	}
	else if ( tableView == framesList )
	{
		if ( [[tableColumn identifier] isEqualToString:@"Number"] )
			return [NSNumber numberWithInt:([player actionStackDepth] - row)];
		else
			return [player actionNameAtFrameDepth:row];
	}
	
	return nil;
}

@end

@implementation DebugController (BreakpointTableViewDelegate)

- (void)breakpointList:(BreakpointTableView*)list breakpointSet:(int)line
{
	[selectedAction setBreakpoint:line];
}


- (void)breakpointList:(BreakpointTableView*)list breakpointCleared:(int)line
{
	[selectedAction clearBreakpoint:line];
}

@end
