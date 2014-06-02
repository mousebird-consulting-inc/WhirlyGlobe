/*
 *  WGSphericalEarthWithTexGroup.mm
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
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

#import "WGSphericalEarthWithTexGroup_private.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation WGSphericalEarthWithTexGroup
{
    WhirlyKitTextureGroup *texGroup;
}

- (id)initWithWithLayerThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene texGroup:(NSString *)texGroupName
{
    self = [super init];
    super.layerThread = layerThread;
    if (self)
    {
        NSString *infoPath = nil;
        if ([[NSFileManager defaultManager] fileExistsAtPath:texGroupName])
            infoPath = texGroupName;
        else
            infoPath = [[NSBundle mainBundle] pathForResource:texGroupName ofType:@"plist"];
        if (!infoPath)
            return nil;
        texGroup = [[WhirlyKitTextureGroup alloc] initWithInfo:infoPath];
        if (!texGroup)
        {
            self = nil;
            return nil;
        }
        _earthLayer = [[WhirlyGlobeSphericalEarthLayer alloc] initWithTexGroup:texGroup];
        [layerThread addLayer:_earthLayer];
    }
    
    return self;
}

- (void)cleanupLayers:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene
{
    [layerThread removeLayer:_earthLayer];
    _earthLayer = nil;
    texGroup = nil;
}

@end