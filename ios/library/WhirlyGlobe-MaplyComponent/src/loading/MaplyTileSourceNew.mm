/*
 *  MaplyTileSourceNew.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/13/18.
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

#import "loading/MaplyTileSourceNew.h"

NSString *MaplyTileIDString(MaplyTileID tileID)
{
    return [NSString stringWithFormat:@"%d_%d_%d",tileID.x,tileID.y,tileID.level];
}

@implementation MaplyTileFetchRequest

-(instancetype)init
{
    self = [super init];
    _tileSource = nil;
    _importance = 0.0;
    
    _success = nil;
    _failure = nil;
    
    return self;
}

@end

@implementation MaplyTileInfoNone

- (int)minZoom
{
    return 0;
}

- (int)maxZoom
{
    return 0;
}

- (id)fetchInfoForTile:(MaplyTileID)tileID flipY:(bool)flipY
{
    return nil;
}

@end

