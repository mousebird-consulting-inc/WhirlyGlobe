/*
 *  ParticleSystemLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/10/11.
 *  Copyright 2011-2012 mousebird consulting
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

#import "ParticleSystemLayer.h"
#import "NSDictionary+Stuff.h"
#import "GlobeMath.h"
#import "UIColor+Stuff.h"

using namespace Eigen;
using namespace WhirlyKit;

#pragma mark - Particle System

@implementation WhirlyKitParticleSystem

@synthesize loc;
@synthesize norm;

@end


#pragma mark - Particle System Info

@interface ParticleSystemInfo : NSObject 
{
    SimpleIdentity destId;
    NSArray *systems;
    NSDictionary *desc;
    NSArray *colors;
}

@property (nonatomic,assign) SimpleIdentity destId;
@property (nonatomic) NSArray *systems;
@property (nonatomic) NSDictionary *desc;

- (id)initWithSystems:(NSArray *)inSystems desc:(NSDictionary *)inDesc;

@end

@implementation ParticleSystemInfo

@synthesize destId;
@synthesize systems;
@synthesize desc;

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


#pragma mark - Particle System Layer

@implementation WhirlyKitParticleSystemLayer

- (void)clear
{
    for (ParticleSysSceneRepSet::iterator it = sceneReps.begin();
         it != sceneReps.end(); ++it)
        delete *it;
    sceneReps.clear();

    scene = NULL;
}

- (void)dealloc
{
    [self clear];
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
    layerThread = inLayerThread;
    scene = inScene;
    
    // Add the Particle Generator to the scene
    // This will create particles for us every frame
    ParticleGenerator *gen = new ParticleGenerator(500000);
    generatorId = gen->getId();
    scene->addChangeRequest(new AddGeneratorReq(gen));
}

// Remove outstanding particle systems
- (void)shutdown
{
    std::vector<ChangeRequest *> changeRequests;
    
    for (ParticleSysSceneRepSet::iterator it = sceneReps.begin();
         it != sceneReps.end(); ++it)
    {
        ParticleSysSceneRep *partRep = *it;
        for (SimpleIDSet::iterator sit = partRep->partSysIDs.begin();
             sit != partRep->partSysIDs.end(); ++sit)
            changeRequests.push_back(new ParticleGeneratorRemSystemRequest(generatorId,*sit));        
    }
    
    if (generatorId != EmptyIdentity)
    {
        changeRequests.push_back(new RemGeneratorReq(generatorId));
        generatorId = EmptyIdentity;
    }
    
    scene->addChangeRequests(changeRequests);
    
    [self clear];
}

// Parse the basic particle system parameters out of an NSDictionary
- (ParticleGenerator::ParticleSystem)parseParams:(NSDictionary *)desc defaultSystem:(ParticleGenerator::ParticleSystem *)defaultParams
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

// Do the actual work off setting up and adding one or more particle systems
- (void)runAddSystems:(ParticleSystemInfo *)systemInfo
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    ParticleSysSceneRep *sceneRep = new ParticleSysSceneRep();
    sceneRep->setId(systemInfo.destId);
    
    // Parse out the general parameters
    ParticleGenerator::ParticleSystem defaultSystem = ParticleGenerator::ParticleSystem::makeDefault();
    ParticleGenerator::ParticleSystem baseParams = [self parseParams:systemInfo.desc defaultSystem:&defaultSystem];

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
        
        // Hand it off to the renderer
        scene->addChangeRequest(new ParticleGeneratorAddSystemRequest(generatorId,newPartSys));
    }
    
    sceneReps.insert(sceneRep);
}

// The actual work of removing a set of particle systems
- (void)runRemoveSystems:(NSNumber *)num
{
    // Look for the matching particle system(s)
    ParticleSysSceneRep dumbRep;
    dumbRep.setId([num intValue]);
    ParticleSysSceneRepSet::iterator it = sceneReps.find(&dumbRep);
    if (it != sceneReps.end())
    {
        ParticleSysSceneRep *sceneRep = *it;
        for (SimpleIDSet::iterator sit = sceneRep->partSysIDs.begin();
             sit != sceneRep->partSysIDs.end(); ++sit)
            scene->addChangeRequest(new ParticleGeneratorRemSystemRequest(generatorId,*sit));
        
        sceneReps.erase(it);
        delete sceneRep;
    }
}

// Add a single particle system
- (SimpleIdentity) addParticleSystem:(WhirlyKitParticleSystem *)partSystem desc:(NSDictionary *)desc
{
    ParticleSystemInfo *systemInfo = [[ParticleSystemInfo alloc] initWithSystems:[NSArray arrayWithObject:partSystem] desc:desc];
    systemInfo.destId = Identifiable::genId();
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runAddSystems:systemInfo];
    else
        [self performSelector:@selector(runAddSystems:) onThread:layerThread withObject:systemInfo waitUntilDone:NO];
    
    return systemInfo.destId;
}

/// Add a group of particle systems
- (WhirlyKit::SimpleIdentity) addParticleSystems:(NSArray *)partSystems desc:(NSDictionary *)desc
{
    if (!layerThread || !scene)
    {
        NSLog(@"WhirlyGlobe Particle Systems layer has not been initialized, yet you're calling addParticleSystem.  Dropping data on floor.");
        return EmptyIdentity;
    }

    ParticleSystemInfo *systemInfo = [[ParticleSystemInfo alloc] initWithSystems:partSystems desc:desc];
    systemInfo.destId = Identifiable::genId();
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runAddSystems:systemInfo];
    else
        [self performSelector:@selector(runAddSystems:) onThread:layerThread withObject:systemInfo waitUntilDone:NO];
    
    return systemInfo.destId;    
}

/// Remove one or more particle systems
- (void) removeParticleSystems:(WhirlyKit::SimpleIdentity)partSysId
{
    NSNumber *num = [NSNumber numberWithInt:partSysId];
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runRemoveSystems:num];
    else
        [self performSelector:@selector(runRemoveSystems:) onThread:layerThread withObject:num waitUntilDone:NO];
}

@end
