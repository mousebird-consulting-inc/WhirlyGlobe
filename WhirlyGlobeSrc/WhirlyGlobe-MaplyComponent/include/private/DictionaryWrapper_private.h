/*
 *  DictionaryWrapper_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 12/16/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import <WhirlyGlobe.h>

@interface NSMutableDictionary(Dictionary)

/// Create an NSMutableDictionary from a Maply Dictionary source
+ (NSMutableDictionary *)DictionaryWithMaplyDictionary:(WhirlyKit::Dictionary *)dict;

@end

@interface NSDictionary(Dictionary)

/// Fill in a Maply Dictionary from an NSSDictionary
- (void)copyToMaplyDictionary:(WhirlyKit::Dictionary *)dict;

@end
