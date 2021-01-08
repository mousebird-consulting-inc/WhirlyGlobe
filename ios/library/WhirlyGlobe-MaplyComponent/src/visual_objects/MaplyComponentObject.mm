/*
 *  WGComponentObject_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
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

#import "MaplyComponentObject_private.h"

using namespace WhirlyKit;

@implementation MaplyComponentObject

- (instancetype)init
{
    self = [super init];
    contents = ComponentObject_iOSRef(new ComponentObject_iOS());
    contents->isSelectable = true;
    contents->enable = true;
    
    return self;
}

- (id)initWithRef:(WhirlyKit::ComponentObject_iOSRef)compObj
{
    self = [super init];
    contents = compObj;
    
    return self;
}

- (instancetype)initWithDesc:(NSDictionary *)desc
{
    self = [super init];
    contents = ComponentObject_iOSRef(new ComponentObject_iOS());
    contents->isSelectable = true;
    contents->enable = true;
    id enable = desc[kMaplyEnable];
    if (enable)
        contents->enable = [enable boolValue];
    id uuid = desc[kMaplyUUID];
    if ([uuid isKindOfClass:[NSString class]])
    {
        contents->uuid = [uuid asStdString];
    }
    id rep = desc[kMaplyRepresentation];
    if ([rep isKindOfClass:[NSString class]])
    {
        contents->representation = [rep asStdString];
    }

    return self;
}

- (NSString *__nullable)getUUID {
    return contents->uuid.empty() ? nil : [NSString stringWithUTF8String:contents->uuid.c_str()];
}

@end
