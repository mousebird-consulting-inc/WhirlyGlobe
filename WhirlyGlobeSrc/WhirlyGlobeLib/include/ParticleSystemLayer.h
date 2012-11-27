/*
 *  ParticleSystemLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/10/11.
 *  Copyright 2011-2012 mousebird consulting. All rights reserved.
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
#import "DrawCost.h"
#import "ParticleGenerator.h"

/** Representation of a single particle system.
    We give it a geographic location and a normal (in 3-space).
    The rest of the info is in the dictionary.
 */
@interface WhirlyKitParticleSystem : NSObject
{
    WhirlyKit::GeoCoord loc;
    Eigen::Vector3f norm;
}

@property (nonatomic,assign) WhirlyKit::GeoCoord loc;
@property (nonatomic,assign) Eigen::Vector3f norm;

@end

namespace WhirlyKit
{
    
/// The scene representation used internally by the layer to track what belongs
///  to a given particle system ID.
class ParticleSysSceneRep : public Identifiable
{
public:
    ParticleSysSceneRep() { }
    
    SimpleIDSet partSysIDs;    // The particle systems we created
};
typedef std::set<ParticleSysSceneRep *,IdentifiableSorter> ParticleSysSceneRepSet;    
    
}

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
{
    /// The layer thread we live in
    WhirlyKitLayerThread * __weak layerThread;
    
    /// Scene we're making changes to
    WhirlyKit::Scene *scene;

    /// ID of the Particle Generator we're using to implement particles
    WhirlyKit::SimpleIdentity generatorId;

    /// Used to track resources related to particle systems for deletion and modification
    WhirlyKit::ParticleSysSceneRepSet sceneReps;
}

/// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene;

/// Called in the layer thread
- (void)shutdown;

/// Add a single particle system to the layer
- (WhirlyKit::SimpleIdentity) addParticleSystem:(WhirlyKitParticleSystem *)partSystem desc:(NSDictionary *)desc;

/// Add a group of particle systems
- (WhirlyKit::SimpleIdentity) addParticleSystems:(NSArray *)partSystems desc:(NSDictionary *)desc;

/// Remove one or more particle systems
- (void) removeParticleSystems:(WhirlyKit::SimpleIdentity)partSysId;

@end
