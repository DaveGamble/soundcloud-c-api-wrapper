/*
 * Copyright 2009 Ullrich Schäfer, Gernot Poetsch for SoundCloud Ltd.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 *
 * For more information and documentation refer to
 * http://soundcloud.com/api
 * 
 */

#import "SCParameterTableDataSource.h"


@implementation SCParameterTableDataSource

#pragma mark Lifecycle

- (id) init
{
	self = [super init];
	if (self != nil) {
		parameterDictionary = [[NSMutableDictionary alloc] init];
	}
	return self;
}


- (void)dealloc;
{
	[parameterDictionary release];
	[super dealloc];
}

#pragma mark Accessors

@synthesize parameterDictionary;

- (void)setParameterDictionary:(NSDictionary *)inParameterDictionary;
{
	if(inParameterDictionary != parameterDictionary){
		[parameterDictionary release];
		parameterDictionary = [inParameterDictionary mutableCopy];
	}
}

#pragma mark Datasource informal protocol

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView;
{
	return [parameterDictionary count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;
{
	NSString *key = [[parameterDictionary allKeys] objectAtIndex:row];
	NSString *colIdentifier = [tableColumn identifier];
	if([colIdentifier isEqualTo:@"Key"])
		return key;
	if([colIdentifier isEqualTo:@"Value"])
		return [parameterDictionary objectForKey:key];
	NSLog(@"Unknown Table Column: %@", colIdentifier);
	return @"check parameter datasource.. :(";
}

#pragma mark adding & removing parameters
- (void)addParameterWithKey:(NSString *)inKey value:(NSString *)inValue;
{
	NSLog(@"added parameter with key %@ and value %@", inKey, inValue);
	[parameterDictionary setObject:inValue forKey:inKey];
}

- (void)removeParametersAtIndexes:(NSIndexSet *)indexes;
{
	NSArray *keysToRemove = [[parameterDictionary allKeys] objectsAtIndexes:indexes];
	for (NSString *key in keysToRemove) {
		[parameterDictionary removeObjectForKey:key];
	}
}
@end
