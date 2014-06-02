/*
 *  WGViewTracker.m
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/26/12.
 *  Copyright 2012 mousebird consulting
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

#import "MaplyViewTracker.h"

@implementation MaplyViewTracker

- (id)init
{
    self = [super init];
    // Note: This should be DrawVisibleInvalid
    _minVis = 1e10;
    _maxVis = 1e10;
    
    return self;
}

@end
