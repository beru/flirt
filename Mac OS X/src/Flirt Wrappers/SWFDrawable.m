/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#import "SWFDrawable.h"
#import "SWFActionObject.h"

@implementation SWFDrawable

+ (id)drawableWithDDDrawable:(ddDrawable*)inDrawable
{
	return [[[self alloc] initWithDDDrawable:inDrawable] autorelease];
}


- (id)initWithDDDrawable:(ddDrawable*)inDrawable
{
	if ( (self = [super init]) == nil )
		return nil;

	drawable = inDrawable;

	return self;
}


- (NSRect)bounds
{
	ddRect rect = ddDrawable_getBounds(drawable);

	double left = DOUBLE_F(rect.left);
	double top = DOUBLE_F(rect.top);
	double right = DOUBLE_F(rect.right);
	double bottom = DOUBLE_F(rect.bottom);

	return NSMakeRect(left, top, right - left, bottom - top);
}


- (NSString*)description
{
	switch ( [self type] )
	{
		case SHAPE_CHAR: return @"Shape";
		case MORPH_CHAR: return @"Morph";
		case BITMAP_CHAR: return @"Bitmap";
		case BUTTON_CHAR: return @"Button";
		case TEXT_CHAR: return @"Text";
		case FONT_CHAR: return @"Font";
		case MOVIECLIP_CHAR: return @"MovieClip";
		case TEXTFIELD_CHAR: return @"TextField";
		case SOUND_CHAR: return @"Sound";
		default: return @"Unknown!";
	}
}


- (ddCharacterType)type
{
	return ddDrawable_getType(drawable);
}


- (SWFActionObject*)object
{
	if ( ddDrawable_getType(drawable) == MOVIECLIP_CHAR )
	{
		ddDrawClip* clip = (ddDrawClip*)drawable;
		
		return [SWFActionObject objectWithDDActionObject:ddDrawClip_getActionObject(clip)];
	}
	
	// XXX - in SWF6, other types have AS representations
	
	return nil;
}


- (NSArray*)subdrawables
{
	if ( subdrawables != nil )
		return subdrawables;

	ddDrawable** drawableList;
	int count;
	int i;

	if ( ddDrawable_getType(drawable) == MOVIECLIP_CHAR )
		ddDrawClip_getDisplayList((ddDrawClip*)drawable, &drawableList, &count);

	else if ( ddDrawable_getType(drawable) == BUTTON_CHAR )
		ddDrawButton_getDisplayList((ddDrawClip*)drawable, &drawableList, &count);

	else
		return nil;
	
	subdrawables = [[NSMutableArray alloc] init];

	for ( i = 0; i < count; ++i )
	{
		if ( drawableList[i] == NULL )
			continue;

		[subdrawables addObject:[SWFDrawable drawableWithDDDrawable:drawableList[i]]];
	}
	
	free(drawableList);

	return subdrawables;
}


- (NSArray*)buttonActions
{
	if ( ddDrawable_getType(drawable) != BUTTON_CHAR )
		return nil;
	
	if ( buttonActions != nil )
		return buttonActions;
	
	buttonActions = [[NSMutableArray alloc] init];
	
	ddDrawButton* button = (ddDrawButton*)drawable;

	//int count;
	
	//ddDrawButton_getActions(button, &count, &actions);
	
	return buttonActions;
}

@end
