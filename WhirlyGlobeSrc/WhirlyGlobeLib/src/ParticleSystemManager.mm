/*
 *  ParticleSystemManager.mm
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

#import "ParticleSystemManager.h"
#import "NSDictionary+Stuff.h"
#import "GlobeMath.h"
#import "UIColor+Stuff.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation WhirlyKitParticleSystem

@end

@interface ParticleSystemInfo : NSObject
{
    NSArray *colors;
}

@property (nonatomic,assign) SimpleIdentity destId;
@property (nonatomic) NSArray *systems;
@property (nonatomic) NSDictionary *desc;

- (id)initWithSystems:(NSArray *)inSystems desc:(NSDictionary *)inDesc;

@end

@implementation ParticleSystemInfo

- (id)initWithSystems:(NSArray *)inSystems desc:(NSDictionary *)inDesc
{
    self = [super init];
    if (self)
    {
        self.systems = inSystems;
        self.desc = inDesc;
    }
    
    return self;
}

@end


namespace WhirlyKit
{
    
ParticleSystemManager::ParticleSystemManager()
{
    pthread_mutex_init(&partLock, NULL);
}
    
ParticleSystemManager::~ParticleSystemManager()
{
    pthread_mutex_destroy(&partLock);
    for (ParticleSysSceneRepSet::iterator it = partReps.begin();
         it != partReps.end(); ++it)
        delete *it;
    partReps.clear();
}
 
    // Parse the basic particle system parameters out of an NSDictionary
ParticleGenerator::ParticleSystem ParticleSystemManager::parseParams(NSDictionary *desc,ParticleGenerator::ParticleSystem *defaultParams)
{
    ParticleGenerator::ParticleSystem params;
    if (defaultParams)
        params = *defaultParams;
        
        params.minLength = [desc floatForKey:@"minLength" default:params.minLength];
        params.maxLength = [desc floatForKey:@"maxLength" default:params.maxLength];
        params.numPerSecMin = [desc intForKey:@"minNumPerSec" default:params.numPerSecMin];
        params.numPerSecMax = [desc intForKey:@"maxNumPerSec" default:params.numPerSecMax];
        params.minLifetime = [desc floatForKey:@"minLifetime" default:params.minLifetime];
        params.maxLifetime = [desc floatForKey:@"maxLifetime" default:params.maxLifetime];
        params.minPhi = [desc floatForKey:@"minPhi" default:params.minPhi];
        params.maxPhi = [desc floatForKey:@"maxPhi" default:params.maxPhi];
        params.minVis = [desc floatForKey:@"minVis" default:DrawVisibleInvalid];
        params.maxVis = [desc floatForKey:@"maxVis" default:DrawVisibleInvalid];
        UIColor *color = [desc objectForKey:@"color"];
        NSArray *colors = [desc objectForKey:@"colors"];
        if (!colors && color)
            colors = [NSArray arrayWithObject:color];
            
            for (UIColor *thisColor in colors)
            {
                params.colors.push_back([thisColor asRGBAColor]);
            }
    
    return params;
}
    
/// Add a group of particle systems
SimpleIdentity ParticleSystemManager::addParticleSystems(NSArray *partSystems,NSDictionary *desc,SimpleIdentity genId,ChangeSet &changes)
{
    ParticleSystemInfo *systemInfo = [[ParticleSystemInfo alloc] initWithSystems:partSystems desc:desc];
    systemInfo.destId = Identifiable::genId();

    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    ParticleSysSceneRep *sceneRep = new ParticleSysSceneRep();
    SimpleIdentity partID = systemInfo.destId;
    sceneRep->setId(systemInfo.destId);
    
    // Parse out the general parameters
    ParticleGenerator::ParticleSystem defaultSystem = ParticleGenerator::ParticleSystem::makeDefault();
    ParticleGenerator::ParticleSystem baseParams = parseParams(systemInfo.desc,&defaultSystem);
    
    // Now run through the particle systems and kick them off
    for (WhirlyKitParticleSystem *partSys in systemInfo.systems)
    {
        // Set up the specifics of this one
        ParticleGenerator::ParticleSystem *newPartSys = new ParticleGenerator::ParticleSystem(baseParams);
        newPartSys->setId(Identifiable::genId());
        sceneRep->partSysIDs.insert(newPartSys->getId());
        newPartSys->loc = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal([partSys loc]));
        // Note: Won't work at the poles
        newPartSys->dirUp = [partSys norm];
        newPartSys->dirE = Vector3f(0,0,1).cross(newPartSys->dirUp);
        newPartSys->dirN = newPartSys->dirUp.cross(newPartSys->dirE);
        
        changes.push_back(new ParticleGeneratorAddSystemRequest(genId,newPartSys));
    }
    
    pthread_mutex_lock(&partLock);
    partReps.insert(sceneRep);
    pthread_mutex_unlock(&partLock);
    
    return partID;
}

/// Remove one or more particle systems
void ParticleSystemManager::removeParticleSystems(SimpleIDSet partIDs,SimpleIdentity genId,ChangeSet &changes)
{
    pthread_mutex_lock(&partLock);

    for (SimpleIDSet::iterator it = partIDs.begin(); it != partIDs.end(); ++it)
    {
        ParticleSysSceneRep dummyRep(*it);
        ParticleSysSceneRepSet::iterator pit = partReps.find(&dummyRep);
        if (pit != partReps.end())
        {
            ParticleSysSceneRep *sceneRep = *pit;
            for (SimpleIDSet::iterator idIt = sceneRep->partSysIDs.begin(); idIt != sceneRep->partSysIDs.end(); ++idIt)
                changes.push_back(new ParticleGeneratorRemSystemRequest(genId,*idIt));
            
            partReps.erase(pit);
        }
    }

    pthread_mutex_unlock(&partLock);
}
    
}
