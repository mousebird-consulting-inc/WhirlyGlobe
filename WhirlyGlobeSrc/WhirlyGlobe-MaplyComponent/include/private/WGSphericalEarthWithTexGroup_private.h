/*
 *  MaplySphericalEarthWithTexGroup.h
 *  WhirlyGlobe-MaplyComponent
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

#import "WGViewControllerLayer_private.h"

@interface WGSphericalEarthWithTexGroup : WGViewControllerLayer

@property (nonatomic,readonly) WhirlyGlobeSphericalEarthLayer *earthLayer;

/// Set up a spherical earth layer with a texture group.
/// Returns nil on failure.
- (id)initWithWithLayerThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene texGroup:(NSString *)texGroupName;

/// Clean up any and all resources 
- (void)cleanupLayers:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene;

@end
