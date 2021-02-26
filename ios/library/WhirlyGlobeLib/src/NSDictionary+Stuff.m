/*
 *  NSDictionary+Stuff.m
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/15/11.
 *  Copyright 2011-2019 mousebird consulting
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

- (id _Nullable)objectForKey:(NSString *)name checkType:(id _Nonnull)theType default:(id _Nullable)theDefault
{
    id what = [self objectForKey:name];
    if (!what || ![what isKindOfClass:theType])
        return theDefault;
    
    return what;
}

- (float)floatForKey:(NSString *_Nonnull)name default:(float)theDefault
{
    id what = [self objectForKey:name];
    if (!what || ![what isKindOfClass:[NSNumber class]])
        return theDefault;
    
    NSNumber *num = what;
    return [num floatValue];
}

- (double)doubleForKey:(NSString *_Nonnull)name default:(double)theDefault
{
    id what = [self objectForKey:name];
    if (!what || ![what isKindOfClass:[NSNumber class]])
        return theDefault;
    
    NSNumber *num = what;
    return [num doubleValue];
}

- (int)intForKey:(NSString *_Nonnull)name default:(int)theDefault
{
    id what = [self objectForKey:name];
    if (!what || ![what isKindOfClass:[NSNumber class]])
        return theDefault;
    
    NSNumber *num = what;
    return [num intValue];
}

- (BOOL)boolForKey:(NSString *_Nonnull)name default:(BOOL)theDefault
{
    id what = [self objectForKey:name];
    if (!what || ![what isKindOfClass:[NSNumber class]])
        return theDefault;
    
    NSNumber *num = what;
    return [num boolValue];
}

- (NSString *_Nullable)stringForKey:(NSString *_Nonnull)name default:(NSString *_Nullable)theDefault
{
    id what = [self objectForKey:name];
    if (!what)
        return theDefault;
    if ([what isKindOfClass:[NSString class]])
        return (NSString *)what;
    if ([what isKindOfClass:[NSNumber class]])
        return [(NSNumber *)what stringValue];
    
    return theDefault;
}

/// Parse an enumerated type and return an int
- (int)enumForKey:(NSString *_Nonnull)name values:(NSArray *_Nonnull)values default:(int)theDefault
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

+ (NSDictionary *_Nonnull) dictionaryByMerging:(NSDictionary *_Nullable) dict1 with:(NSDictionary *_Nullable)dict2
{
    NSMutableDictionary *result = dict1 ? [NSMutableDictionary dictionaryWithDictionary:dict1] : [NSMutableDictionary new];
    if (dict2)
    {
        [result addEntriesFromDictionary:dict2];
    }
    return result;
}

- (NSDictionary *_Nonnull) dictionaryByMergingWith:(NSDictionary *_Nullable) dict
{
    return [[self class] dictionaryByMerging:self with:dict];
}

@end

void NSDictionaryDummyFunc()
{    
}
