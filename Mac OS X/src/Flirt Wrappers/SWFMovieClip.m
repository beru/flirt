/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#import "SWFAction.h"
#import "SWFMovieclip.h"
#import "SWFTimelineItem.h"

#import "flirt-debug.h"

@interface SWFMovieClip (Private)
- (void)buildTimelineArray;
@end

@implementation SWFMovieClip

+ (id)clipWithDDMovieClip:(ddMovieClip*)inClip
{
	return [[[self alloc] initWithDDMovieClip:inClip] autorelease];
}


- (id)initWithDDMovieClip:(ddMovieClip*)inClip
{
	if ( (self = [super init]) == nil )
		return nil;

	clip = inClip;

	return self;
}


- (ddMovieClip*)ddMovieClip
{
	return clip;
}


- (int)frameCount
{
	return ddMovieClip_getFramesTotal(clip);
}


- (NSDictionary*)frameNumber:(int)num
{
	ddTimelineFrame* frame = ddMovieClip_getFrame(clip, num);

	NSMutableDictionary* dict = [[NSMutableDictionary dictionary] retain];

	ddAction* actions = ddTimelineFrame_getActions(frame);
	ddTimelineItem* item = ddTimelineFrame_getItems(frame);
	ddSoundStreamBlock* sound = ddTimelineFrame_getSoundStreamBlock(frame);

	if ( actions != NULL )
		[dict setObject:[SWFAction actionWithDDAction:actions] forKey:@"Actions"];

	if ( item != NULL )
	{
		NSMutableArray* list = [NSMutableArray array];

		while ( item != NULL )
		{
			[list addObject:[SWFTimelineItem itemWithDDTimelineItem:item]];

			item = ddTimelineItem_getNext(item);
		}

		[dict setObject:list forKey:@"Items"];
	}

	if ( sound != NULL )
		[dict setObject:@"Sound" forKey:@"SoundStreamBlock"]; // XXX

	[dict setObject:[NSString stringWithFormat:@"Frame %i", num] forKey:@"Frame"];

	return dict;
}

@end
