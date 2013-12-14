/*
 *  ParticleSystemManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/30/13.
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

/** Representation of a single particle system.
 We give it a geographic location and a normal (in 3-space).
 The rest of the info is in the dictionary.
 */
@interface WhirlyKitParticleSystem : NSObject

/// Where the particle system base is
@property (nonatomic,assign) WhirlyKit::GeoCoord loc;
/// Direction we're sending particles out
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
    ParticleSysSceneRep(SimpleIdentity theId) : Identifiable(theId) { }
    
    SimpleIDSet partSysIDs;    // The particle systems we created
};
typedef std::set<ParticleSysSceneRep *,IdentifiableSorter> ParticleSysSceneRepSet;
    
#define kWKParticleSystemManager "WKParticleSystemManager"
    
/** The particle system manager handles geometry related to active particle systems.
    It's thread safe except for deletion.
  */
class ParticleSystemManager : public SceneManager
{
public:
    ParticleSystemManager();
    virtual ~ParticleSystemManager();
    
    /// Add a group of particle systems
    SimpleIdentity addParticleSystems(NSArray *partSystems,NSDictionary *desc,SimpleIdentity genId,ChangeSet &changes);
    
    /// Remove one or more particle systems
    void removeParticleSystems(SimpleIDSet partIDs,SimpleIdentity genId,ChangeSet &changes);
    
protected:
    ParticleGenerator::ParticleSystem parseParams(NSDictionary *desc,ParticleGenerator::ParticleSystem *defaultParams);
    
    pthread_mutex_t partLock;
    ParticleSysSceneRepSet partReps;
};
    
}
