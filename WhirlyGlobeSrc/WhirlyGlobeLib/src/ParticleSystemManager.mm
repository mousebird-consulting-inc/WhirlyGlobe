/*
 *  ParticleSystemManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/26/15.
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

#import "ParticleSystemManager.h"

namespace WhirlyKit
{

ParticleSystemSceneRep::ParticleSystemSceneRep()
{
}

ParticleSystemSceneRep::ParticleSystemSceneRep(SimpleIdentity inId)
: Identifiable(inId)
{
}
    
ParticleSystemSceneRep::~ParticleSystemSceneRep()
{
}
    
void ParticleSystemSceneRep::clearContents(ChangeSet &changes)
{
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
        changes.push_back(new RemDrawableReq(*it));
}
    
ParticleSystemManager::ParticleSystemManager()
{
    pthread_mutex_init(&partSysLock, NULL);
}
    
ParticleSystemManager::~ParticleSystemManager()
{
    pthread_mutex_destroy(&partSysLock);
    for (auto it : sceneReps)
        delete it;
    sceneReps.clear();
}
    
SimpleIdentity ParticleSystemManager::addParticleSystem(const ParticleSystem &newSystem,ChangeSet &changes)
{
    ParticleSystemSceneRep *sceneRep = new ParticleSystemSceneRep();

    sceneRep->partSys = newSystem;
    
    SimpleIdentity partSysID = sceneRep->getId();
    
    pthread_mutex_lock(&partSysLock);
    sceneReps.insert(sceneRep);
    pthread_mutex_unlock(&partSysLock);
    
    return partSysID;
}
    
void ParticleSystemManager::enableParticleSystem(SimpleIdentity sysID,bool enable,ChangeSet &changes)
{
    pthread_mutex_lock(&partSysLock);
    
    ParticleSystemSceneRep dummyRep(sysID);
    auto it = sceneReps.find(&dummyRep);
    if (it != sceneReps.end())
        (*it)->enableContents(enable, changes);
    
    pthread_mutex_unlock(&partSysLock);
}
    
void ParticleSystemManager::removeParticleSystem(SimpleIdentity sysID,ChangeSet &changes)
{
    pthread_mutex_lock(&partSysLock);
    
    ParticleSystemSceneRep dummyRep(sysID);
    auto it = sceneReps.find(&dummyRep);
    if (it != sceneReps.end())
    {
        (*it)->removeContents(changes);
        sceneReps.erase(it);
    }
    
    pthread_mutex_unlock(&partSysLock);
}
    
void ParticleSystemManager::housekeeping(NSTimeInterval now,ChangeSet &changes)
{
    // Note: Fill this in
}
    
}
