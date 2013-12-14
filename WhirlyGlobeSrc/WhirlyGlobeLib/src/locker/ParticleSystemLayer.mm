/*
 *  ParticleSystemLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/10/11.
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

#import "ParticleSystemLayer.h"
#import "NSDictionary+Stuff.h"
#import "GlobeMath.h"
#import "UIColor+Stuff.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation WhirlyKitParticleSystemLayer
{
    /// The layer thread we live in
    WhirlyKitLayerThread * __weak layerThread;
    
    /// Scene we're making changes to
    WhirlyKit::Scene *scene;
    
    SimpleIdentity generatorId;
    
    SimpleIDSet partIDs;
}

- (void)clear
{
    partIDs.clear();
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
    [layerThread addChangeRequest:(new AddGeneratorReq(gen))];
}

// Remove outstanding particle systems
- (void)shutdown
{
    ChangeSet changes;
    
    ParticleSystemManager *partManager = (ParticleSystemManager *)scene->getManager(kWKParticleSystemManager);
    if (partManager)
        partManager->removeParticleSystems(partIDs,generatorId,changes);    
    
    [layerThread addChangeRequests:(changes)];
    
    [self clear];
}


- (WhirlyKit::SimpleIdentity) addParticleSystems:(NSArray *)partSystems desc:(NSDictionary *)desc
{
    if (!layerThread || !scene || ([NSThread currentThread] != layerThread))
    {
        NSLog(@"Particle systems layer called before being initialized or in wrong thread.  Dropping data on floor.");
        return EmptyIdentity;
    }
    
    SimpleIdentity partID = EmptyIdentity;
    ChangeSet changes;
    ParticleSystemManager *partManager = (ParticleSystemManager *)scene->getManager(kWKParticleSystemManager);
    if (partManager)
    {
        partID = partManager->addParticleSystems(partSystems, desc, generatorId, changes);
        if (partID != EmptyIdentity)
            partIDs.insert(partID);
    }
    [layerThread addChangeRequests:changes];
    
    return partID;
}

/// Remove one or more particle systems
- (void) removeParticleSystems:(SimpleIdentity)partID
{
    ChangeSet changes;
    ParticleSystemManager *partManager = (ParticleSystemManager *)scene->getManager(kWKParticleSystemManager);
    
    SimpleIDSet::iterator it = partIDs.find(partID);
    if (it != partIDs.end())
    {
        if (partManager)
        {
            SimpleIDSet theIDs;
            theIDs.insert(partID);
            partManager->removeParticleSystems(theIDs, generatorId, changes);
        }
        partIDs.erase(it);
    }
    
    [layerThread addChangeRequests:changes];
}

@end
