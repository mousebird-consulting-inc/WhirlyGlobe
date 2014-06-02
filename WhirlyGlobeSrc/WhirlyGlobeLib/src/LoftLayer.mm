/*
 *  LoftLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/16/11.
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

#import "LoftLayer.h"
#import "UIColor+Stuff.h"
#import "NSDictionary+Stuff.h"

using namespace WhirlyKit;
using namespace WhirlyGlobe;


@implementation WhirlyKitLoftLayer
{
    WhirlyKitLayerThread * __weak layerThread;
    WhirlyKit::Scene *scene;
    SimpleIDSet loftIDs;
}

- (id)init
{
    if ((self = [super init]))
    {
        _gridSize = 10.0 / 180.0 * M_PI;  // Default to 10 degrees
    }
    
    return self;
}

- (void)clear
{
    loftIDs.clear();
    scene = NULL;
}

- (void)dealloc
{
    [self clear];
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(Scene *)inScene
{
	scene = inScene;
    layerThread = inLayerThread;
}

- (void)shutdown
{
    ChangeSet changeRequests;
    
    LoftManager *loftManager = (LoftManager *)scene->getManager(kWKLoftedPolyManager);
    
    ChangeSet changes;
    if (loftManager)
        loftManager->removeLoftedPolys(loftIDs, changes);
    [layerThread addChangeRequests:changes];
    
    [self clear];
}


// Add a lofted poly
- (WhirlyKit::SimpleIdentity) addLoftedPolys:(WhirlyKit::ShapeSet *)shapes desc:(NSDictionary *)desc cacheName:(NSString *)cacheName cacheHandler:(NSObject<WhirlyKitLoftedPolyCache> *)cacheHandler
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
        NSLog(@"Loft layer not initialized or being called in wrong thread.  Dropping data.");
        return EmptyIdentity;
    }

    LoftManager *loftManager = (LoftManager *)scene->getManager(kWKLoftedPolyManager);

    SimpleIdentity loftID = EmptyIdentity;
    ChangeSet changes;
    if (loftManager)
    {
        loftID = loftManager->addLoftedPolys(shapes, desc, cacheName, cacheHandler, _gridSize, changes);
        if (loftID != EmptyIdentity)
            loftIDs.insert(loftID);
    }
    [layerThread addChangeRequests:changes];
    
    return loftID;
}

- (WhirlyKit::SimpleIdentity) addLoftedPoly:(WhirlyKit::VectorShapeRef)shape desc:(NSDictionary *)desc cacheName:(NSString *)cacheName cacheHandler:(NSObject<WhirlyKitLoftedPolyCache> *)cacheHandler
{
    ShapeSet shapes;
    shapes.insert(shape);
    
    return [self addLoftedPolys:&shapes desc:desc cacheName:(NSString *)cacheName cacheHandler:cacheHandler];
}

// Remove the lofted poly
- (void)removeLoftedPoly:(SimpleIdentity)polyID
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
        NSLog(@"Loft layer not initialized or being called in wrong thread.  Dropping data.");
        return;
    }

    SimpleIDSet::iterator it = loftIDs.find(polyID);
    if (it != loftIDs.end())
    {
        ChangeSet changes;
        LoftManager *loftManager = (LoftManager *)scene->getManager(kWKLoftedPolyManager);
        if (loftManager)
        {
            SimpleIDSet theIDs;
            theIDs.insert(polyID);
            loftManager->removeLoftedPolys(theIDs, changes);
        }
        loftIDs.erase(it);
        [layerThread addChangeRequests:changes];
    }
}

@end
