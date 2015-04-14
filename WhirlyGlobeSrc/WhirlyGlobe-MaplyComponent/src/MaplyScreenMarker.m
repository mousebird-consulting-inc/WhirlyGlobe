/*
 *  WGScreenMarker.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
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

#import "MaplyScreenMarker.h"

@implementation MaplyScreenMarker

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    _selectable = true;
    _offset = CGPointMake(0, 0);
    _layoutSize = CGSizeMake(-1.0, -1.0);
    _period = 0.0;
    
    return self;
}

@end

@implementation MaplyMovingScreenMarker
@end
