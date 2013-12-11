/*
 *  MaplySphericalQuadEarthWithTexGroup.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/24/13.
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

#import "MaplySphericalQuadEarthWithTexGroup_private.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation MaplySphericalQuadEarthWithTexGroup
{
    NSString *texGroupName;
    WhirlyKitSphericalEarthQuadLayer *earthLayer;
}

- (id)initWithWithTexGroup:(NSString *)inTexGroupName
{
    self = [super init];
    if (!self)
        return nil;
    
    texGroupName = inTexGroupName;
    
    return self;
}

- (bool)startLayer:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene renderer:(WhirlyKitSceneRendererES *)renderer viewC:(MaplyBaseViewController *)viewC
{
    if (![[NSFileManager defaultManager] fileExistsAtPath:texGroupName])
    {
        texGroupName = [[NSBundle mainBundle] pathForResource:texGroupName ofType:@"plist"];
        if (!texGroupName)
            return false;
    }
    
    earthLayer = [[WhirlyKitSphericalEarthQuadLayer alloc] initWithInfo:texGroupName renderer:renderer];
    if (!earthLayer)
        return nil;
    [layerThread addLayer:earthLayer];
    
    return true;
}

- (void)cleanupLayers:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene
{
    [layerThread removeLayer:earthLayer];
}

@end
