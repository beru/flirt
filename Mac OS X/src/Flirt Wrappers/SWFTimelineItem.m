/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#import "SWFMovieClip.h"
#import "SWFTimelineItem.h"

@implementation SWFTimelineItem

+ (id)itemWithDDTimelineItem:(ddTimelineItem*)inItem
{
	return [[[self alloc] initWithDDTimelineItem:inItem] autorelease];
}


- (id)initWithDDTimelineItem:(ddTimelineItem*)inItem
{
	if ( (self = [super init]) == nil )
		return nil;

	item = inItem;
	
	return self;
}


- (NSString*)description
{
	ddCharacter* character = ddTimelineItem_getCharacter(item);
	ddCharacterType type = ddCharacter_getType(character);
	const char* name = ddTimelineItem_getName(item);

	NSString* typestr;
	
	switch ( type )
	{
		case SHAPE_CHAR: typestr = @"Shape"; break;
		case MORPH_CHAR: typestr = @"Morph"; break;
		case BITMAP_CHAR: typestr = @"Bitmap"; break;
		case BUTTON_CHAR: typestr = @"Button"; break;
		case TEXT_CHAR: typestr = @"Text"; break;
		case FONT_CHAR: typestr = @"Font"; break;
		case MOVIECLIP_CHAR: typestr = @"MovieClip"; break;
		case TEXTFIELD_CHAR: typestr = @"TextField"; break;
		case SOUND_CHAR: typestr = @"Sound"; break;
		default: typestr = [NSString stringWithFormat:@"Unknown (%i)", type]; break;
	}

	if ( name != NULL )
		return [NSString stringWithFormat:@"%@ \"%s\"", typestr, name];
	else
		return typestr;
}


- (SWFMovieClip*)movieClip
{
	ddCharacter* character = ddTimelineItem_getCharacter(item);
	ddCharacterType type = ddCharacter_getType(character);

	if ( type == MOVIECLIP_CHAR )
		return [SWFMovieClip clipWithDDMovieClip:(ddMovieClip*)character];
	else
		return nil;
}

@end
