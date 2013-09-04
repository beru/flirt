/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#import "SWFActionObject.h"
#import "SWFActionValue.h"

@implementation SWFActionObject

+ (id)objectWithDDActionObject:(ddActionObject*)inObject
{
	return [[[self alloc] initWithDDActionObject:inObject] autorelease];
}

- (id)initWithDDActionObject:(ddActionObject*)inObject
{
	if ( (self = [super init]) == nil )
		return nil;
	
	object = inObject;

	return self;
}


- (void)dealloc
{
	[values release];
	
	[super dealloc];
}


- (NSArray*)values
{
	if ( values != nil )
		return values;

	char** nameList;
	ddActionValue* valueList;
	int nValues;
	int i;

	ddActionObject_getProperties(object, &nameList, &valueList, &nValues);

	values = [[NSMutableArray alloc] initWithCapacity:nValues];

	for ( i = 0; i < nValues; ++i )
	{
		NSString* name = [NSString stringWithCString:nameList[i]];
		SWFActionValue* value = [SWFActionValue valueWithDDActionValue:valueList[i]];
		
		[values addObject:[NSDictionary dictionaryWithObjectsAndKeys:name, @"Name", value, @"Value", nil]];
	}

	free(nameList);
	free(valueList);

	return values;
}

@end
