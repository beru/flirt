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

@interface SWFAction : NSObject
{
	ddAction* action;
}

+ (id)actionWithDDAction:(ddAction*)action;
- (id)initWithDDAction:(ddAction*)action;

- (int)length;
- (NSString*)opcodeAtOffset:(int)index;
- (SWFAction*)functionAtOffset:(int)index;

- (void)setBreakpoint:(int)line;
- (void)clearBreakpoint:(int)line;

@end
