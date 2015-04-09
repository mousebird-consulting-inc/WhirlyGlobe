/*
 *  SphericalEarthLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/11/11.
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

#import <math.h>
#import "WhirlyVector.h"
#import "TextureGroup.h"
#import "GlobeScene.h"
#import "DataLayer.h"
#import "LayerThread.h"

namespace WhirlyGlobe
{
/// We send this notification out after the layer has completed loading
#define kWhirlyGlobeSphericalEarthLoaded @"WhirlyGlobeSphericalEarthLoaded"
    
// Each chunk of the globe is broken into this many units
static const unsigned int SphereTessX = 10,SphereTessY = 25;
//static const unsigned int SphereTessX = 20,SphereTessY = 50;
}

/** This is the earth modelled as a sphere.  Yes, this
    probably needs to be an ellipse some day, but not yet.
    It's a data layer, so it starts off in the layer thread
    by creating the earth model itself.  Once that's done,
    it doesn't do anything else. 
 */
@interface WhirlyGlobeSphericalEarthLayer : NSObject<WhirlyKitLayer>

/// If set, the time to fade in the globe
@property (nonatomic,assign) float fade;
/// The drawPriority of any drawables we create.  Useful for sorting in non-z mode.
@property (nonatomic,assign) int drawPriority;

/// Create it like this.  It needs a texture group to run.
/// That provides the images and it will generate the geometry.
- (id)initWithTexGroup:(WhirlyKitTextureGroup *)texGroup;

/// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene;

/// Ask the earth layer what the smallest tesselation size for
///  overlaid geometry should be.  This is intended to avoid Z fighting
- (float)smallestTesselation;

/// Change the texture group being used.
/// The new one must be exactly the same size as the old one.
/// Returns true if that was possible.
- (bool)changeTexGroup:(WhirlyKitTextureGroup *)texGroup;
	
@end
