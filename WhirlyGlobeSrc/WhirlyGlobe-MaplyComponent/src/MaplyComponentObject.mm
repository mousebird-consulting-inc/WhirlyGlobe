/*
 *  WGComponentObject_private.h
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

#import "MaplyComponentObject_private.h"

@implementation MaplyComponentObject

- (id)init
{
    self = [super init];
    _isSelectable = true;
    _enable = true;
    
    return self;
}

- (id)initWithDesc:(NSDictionary *)desc
{
    self = [super init];
    _isSelectable = true;
    _enable = true;
    id enable = desc[kMaplyEnable];
    if (enable)
        _enable = [enable boolValue];
    _vectorOffset = WhirlyKit::Point2d(0,0);
    
    return self;
}

@end
