/*
 *  MaplyBillboard.mm
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 10/28/13.
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

#import "MaplyBillboard.h"

@implementation MaplyBillboard

- (id)initWithImage:(id)texture color:(UIColor *)color size:(CGSize)size
{
    self = [super init];
    if (!self)
        return nil;
    
    _screenObj = [[MaplyScreenObject alloc] init];
    [_screenObj addImage:texture color:color size:size];
    
    return self;
}

@end
