/*
 *  NSDictionary+Stuff.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/15/11.
 *  Copyright 2011-2015 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import <Foundation/Foundation.h>


@interface NSDictionary(Stuff)

/// Return the object correspond to name if it's the right type
/// Returns default if missing or wrong type
- (id)objectForKey:(NSString *)name checkType:(id)theType default:(id)theDefault;

/// Return a float for the given name if it exists
/// Returns default if not or wrong type
- (float)floatForKey:(NSString *)name default:(float)theDefault;

/// Return a double for the given name if it exists
/// Returns default if not or wrong type
- (double)doubleForKey:(NSString *)name default:(double)theDefault;

/// Return an integer for the given name if it exists
/// Returns default if not or wrong type
- (int)intForKey:(NSString *)name default:(int)theDefault;

/// Return a boolean for the given name if it exists
/// Returns default if not or wrong type
- (BOOL)boolForKey:(NSString *)name default:(BOOL)theDefault;

/// Return a string for the given name if it exists
/// Returns default if not or wrong type
- (NSString *)stringForKey:(NSString *)name default:(NSString *)theDefault;

/// Parse an enumerated type and return an int
- (int)enumForKey:(NSString *)name values:(NSArray *)values default:(int)theDefault;

@end

// A function we can call to force the linker to bring in categories
#ifdef __cplusplus
extern "C"
#endif
void NSDictionaryDummyFunc();
