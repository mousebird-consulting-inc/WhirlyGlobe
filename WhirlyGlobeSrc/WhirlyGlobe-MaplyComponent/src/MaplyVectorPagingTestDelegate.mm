/*
 *  MaplyVectorPagingTestDelegate.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/23/14.
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

#import "MaplyVectorPagingTestDelgate.h"

@implementation MaplyVectorPagingTestDelegate

- (id)init
{
    self = [super init];
    _coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    
    return self;
}

- (int)minZoom
{
    return 0;
}

- (int)maxZoom
{
    return 10;
}

- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *)layer
{
    NSLog(@"Loaded tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
    [layer tileDidLoad:tileID];
}

@end
