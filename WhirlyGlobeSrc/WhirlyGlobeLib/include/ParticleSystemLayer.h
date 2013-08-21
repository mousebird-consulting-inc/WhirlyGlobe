/*
 *  ParticleSystemLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/10/11.
 *  Copyright 2011-2013 mousebird consulting. All rights reserved.
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
#import "Drawable.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "TextureAtlas.h"
#import "ParticleGenerator.h"
#import "ParticleSystemManager.h"

/** Particle System Layer.
    This layer creates and controls particle systems defined by locations
    and a collection attributes in a dictionary.

    A particle system is tied to a specific location where it will spew out
    a large number of individual particles.  The speed, color, and lifetime
    of those particles is controllable.
 
    Location and normal information for a particle system is controlled by
    the ParticleSystem object.  The rest of the parameters are controlled
    by the description dictonary.
    <list type="bullet">
    <item>minLength     [NSNumber float]
    <item>maxLength     [NSNumber float]
    <item>minNumPerSec  [NSNumber float]
    <item>maxNumPerSec  [NSNumber float]
    <item>minLifetime   [NSNumber float]
    <item>maxLifetime   [NSNumber float]
    <item>minPhi        [NSNumber float]
    <item>maxPhi        [NSNumber float]
    <item>minVis        [NSNumber float]
    <item>maxVis        [NSNumber float]
    <item>color         [UIColor]
    <item>colors        [NSArray<UIColor>]
    </list>
 
    When a particular parameter has max and min values, we'll randomly select
    a value in between those two for every given particle.
    <list type="bullet">
    <item>length is the distance a particular particle can travel.
    <item>numPerSec is the number of particles to generate per second
    <item>lifetime is how long a particle will live in seconds
    <item>phi is the angle between a particle's normal and the particle system's local north
    <item>color is a single color which will be applied to all particles
    <item>colors is an optional array of colors from which we'll randomly pick one per particle
    </list>
  */
@interface WhirlyKitParticleSystemLayer : NSObject<WhirlyKitLayer> 

/// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene;

/// Called in the layer thread
- (void)shutdown;

/// Add a group of particle systems
- (WhirlyKit::SimpleIdentity) addParticleSystems:(NSArray *)partSystems desc:(NSDictionary *)desc;

/// Remove one or more particle systems
- (void) removeParticleSystems:(WhirlyKit::SimpleIdentity)partSysId;

@end
