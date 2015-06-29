/*
 *  ParticleSystemLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/21/15.
 *  Copyright 2011-2015 mousebird consulting. All rights reserved.
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
#import <set>
#import <map>
#import "Identifiable.h"
#import "BasicDrawable.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "LayoutManager.h"

// How often we clean up old particle systems
#define kWKParticleSystemCleanupPeriod 1.0

/** The Particle System Layer nudges the particle system manager occasionally to clean up
 old particle batches.
 */
@interface WhirlyKitParticleSystemLayer : NSObject<WhirlyKitLayer>

/// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene;

/// Called in the layer thread
- (void)shutdown;

@end
