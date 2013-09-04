/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#import "SWFAction.h"


@implementation SWFAction

+ (id)actionWithDDAction:(ddAction*)inAction
{
	return [[[self alloc] initWithDDAction:inAction] autorelease];
}


- (id)initWithDDAction:(ddAction*)inAction
{
	if ( (self = [super init]) == nil )
		return nil;
	
	action = inAction;

	return self;
}


- (int)length
{
	return ddAction_getLength(action);
}


- (NSString*)opcodeAtOffset:(int)offset
{
	return [NSString stringWithCString:ddAction_opcodeAtOffset(action, offset)];
}


- (SWFAction*)functionAtOffset:(int)offset
{
	ddAction* function = ddAction_functionAtOffset(action, offset);
	
	if ( function == NULL )
		return nil;
	
	return [SWFAction actionWithDDAction:function];
}


- (void)setBreakpoint:(int)line
{
	ddAction_setBreakpoint(action, line);
}


- (void)clearBreakpoint:(int)line
{
	ddAction_clearBreakpoint(action, line);
}

@end
