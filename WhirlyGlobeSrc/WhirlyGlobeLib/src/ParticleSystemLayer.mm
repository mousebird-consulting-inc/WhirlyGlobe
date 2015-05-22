/*
 *  ParticleSystemLayer.mm
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

#import "ParticleSystemLayer.h"
#import "ParticleSystemManager.h"

using namespace WhirlyKit;

@implementation WhirlyKitParticleSystemLayer
{
    // Layer thread we're on
    WhirlyKitLayerThread * __weak layerThread;
    // Scene we're updating
    Scene *scene;
    ParticleSystemManager *partSysManager;
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
    layerThread = inLayerThread;
    scene = inScene;
    partSysManager = (ParticleSystemManager *)scene->getManager(kWKParticleSystemManager);
    
    [self performSelector:@selector(cleanup) withObject:nil afterDelay:kWKParticleSystemCleanupPeriod];
}

- (void)cleanup
{
    if (!scene)
        return;
    
    NSTimeInterval now = CFAbsoluteTimeGetCurrent();
    
    ChangeSet changes;
    partSysManager->housekeeping(now,changes);
    [layerThread addChangeRequests:changes];
    
    [self performSelector:@selector(cleanup) withObject:nil afterDelay:kWKParticleSystemCleanupPeriod];
}

- (void)shutdown
{
    scene = NULL;
}

@end
