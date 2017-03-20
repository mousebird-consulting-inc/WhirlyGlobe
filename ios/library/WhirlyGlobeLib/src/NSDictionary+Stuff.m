/*
 *  NSDictionary+Stuff.m
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

#import "NSDictionary+Stuff.h"

@implementation NSDictionary(Stuff)

- (id)objectForKey:(NSString *)name checkType:(id)theType default:(id)theDefault
{
    id what = [self objectForKey:name];
    if (!what || ![what isKindOfClass:theType])
        return theDefault;
    
    return what;
}

- (float)floatForKey:(NSString *)name default:(float)theDefault
{
    id what = [self objectForKey:name];
    if (!what || ![what isKindOfClass:[NSNumber class]])
        return theDefault;
    
    NSNumber *num = what;
    return [num floatValue];
}

- (double)doubleForKey:(NSString *)name default:(double)theDefault
{
    id what = [self objectForKey:name];
    if (!what || ![what isKindOfClass:[NSNumber class]])
        return theDefault;
    
    NSNumber *num = what;
    return [num doubleValue];
}

- (int)intForKey:(NSString *)name default:(int)theDefault
{
    id what = [self objectForKey:name];
    if (!what || ![what isKindOfClass:[NSNumber class]])
        return theDefault;
    
    NSNumber *num = what;
    return [num intValue];
}

- (BOOL)boolForKey:(NSString *)name default:(BOOL)theDefault
{
    id what = [self objectForKey:name];
    if (!what || ![what isKindOfClass:[NSNumber class]])
        return theDefault;
    
    NSNumber *num = what;
    return [num boolValue];
}

- (NSString *)stringForKey:(NSString *)name default:(NSString *)theDefault
{
    id what = [self objectForKey:name];
    if (!what || ![what isKindOfClass:[NSString class]])
        return theDefault;
    
    NSString *str = what;
    return str;
}

/// Parse an enumerated type and return an int
- (int)enumForKey:(NSString *)name values:(NSArray *)values default:(int)theDefault
{
    id what = [self objectForKey:name];
    if (!what || ![what isKindOfClass:[NSString class]])
        return theDefault;
    
    NSString *str = what;
    
    int which = 0;
    for (NSString *valStr in values)
    {
        if ([valStr isKindOfClass:[NSString class]])
            if ([str isEqualToString:valStr])
                return which;
        which++;
    }
    
    return theDefault;
}

@end

void NSDictionaryDummyFunc()
{    
}
