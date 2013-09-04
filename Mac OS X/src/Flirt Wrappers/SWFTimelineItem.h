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

@class SWFMovieClip;

@interface SWFTimelineItem : NSObject
{
	ddTimelineItem* item;
}

+ (id)itemWithDDTimelineItem:(ddTimelineItem*)item;
- (id)initWithDDTimelineItem:(ddTimelineItem*)item;

- (SWFMovieClip*)movieClip;

@end
