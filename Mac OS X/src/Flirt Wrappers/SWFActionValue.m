/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#import "SWFAction.h"
#import "SWFActionObject.h"
#import "SWFActionValue.h"

@implementation SWFActionValue

+ (id)valueWithDDActionValue:(ddActionValue)inValue
{
	return [[[self alloc] initWithDDActionValue:inValue] autorelease];
}

- (id)initWithDDActionValue:(ddActionValue)inValue
{
	if ( (self = [super init]) == nil )
		return nil;

	value = inValue;

	return self;
}


- (void)dealloc
{
	[object release];
	[super dealloc];
}


- (NSString*)description
{
	char* str = ddActionValue_getStringValue(nil, value);
	NSString* ret = [NSString stringWithCString:str];

	free(str);
	
	return ret;
}


- (ddActionValueType)type
{
	return value.type;
}


- (SWFActionObject*)object
{
	if ( object != nil )
		return object;
	
	if ( value.type == VALUE_OBJECT )
		object = [[SWFActionObject objectWithDDActionObject:value.data.objectValue] retain];

	return object;
}


- (SWFAction*)functionValue
{
	ddActionFunction* function = ddActionValue_getFunctionValue(value);

	if ( function != NULL )
	{
		ddAction* action = ddActionFunction_getBytecodeAction(function);

		if ( action != NULL )
			return [SWFAction actionWithDDAction:action];
	}

	return nil;
}

@end
