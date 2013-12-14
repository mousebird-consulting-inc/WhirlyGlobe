/*
 *  ShapeLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/28/11.
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

#import "ShapeLayer.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"
#import "GlobeMath.h"
#import "VectorData.h"
#import "ShapeManager.h"

using namespace WhirlyKit;

@implementation WhirlyKitShapeLayer
{
    /// Layer thread this belongs to
    WhirlyKitLayerThread * __weak layerThread;
    /// Scene the marker layer is modifying
    WhirlyKit::Scene *scene;

    SimpleIDSet shapeIDs;
}

- (void)clear
{
    shapeIDs.clear();
}

- (void)dealloc
{
    [self clear];
}

// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(Scene *)inScene
{
    layerThread = inLayerThread;
    scene = inScene;
}

- (void)shutdown
{
    ChangeSet changes;
    ShapeManager *shapeManager = (ShapeManager *)scene->getManager(kWKShapeManager);
    if (shapeManager)
        shapeManager->removeShapes(shapeIDs, changes);
    shapeIDs.clear();
    
    [layerThread addChangeRequests:(changes)];
    
    [self clear];
}

// Add a single shape
- (SimpleIdentity) addShape:(WhirlyKitShape *)shape desc:(NSDictionary *)desc
{
    return [self addShapes:[NSArray arrayWithObject:shape] desc:desc];
}

// Add a whole bunch of shapes
- (SimpleIdentity) addShapes:(NSArray *)shapes desc:(NSDictionary *)desc
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
       NSLog(@"ShapeLayer: Tried to call shape layer before it was initialized or in wrong thread.  Dropping shapes on floor.");
       return EmptyIdentity;
    }

    ChangeSet changes;
    ShapeManager *shapeManager = (ShapeManager *)scene->getManager(kWKShapeManager);
    SimpleIdentity shapeID = EmptyIdentity;
    if (shapeManager)
    {
        shapeID = shapeManager->addShapes(shapes, desc, changes);
    }
    if (shapeID != EmptyIdentity)
        shapeIDs.insert(shapeID);

    [layerThread addChangeRequests:changes];

    return shapeID;
}

// Remove a group of shapes
- (void) removeShapes:(WhirlyKit::SimpleIdentity)shapeID
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
        NSLog(@"ShapeLayer: Tried to call shape layer before it was initialized or in wrong thread.  Dropping remove on floor.");
        return;
    }

    ChangeSet changes;
    ShapeManager *shapeManager = (ShapeManager *)scene->getManager(kWKShapeManager);
    if (shapeManager)
    {
        SimpleIDSet::iterator it = shapeIDs.find(shapeID);
        if (it != shapeIDs.end())
        {
            shapeIDs.erase(it);
            SimpleIDSet toRemove;
            toRemove.insert(shapeID);
            shapeManager->removeShapes(toRemove, changes);
        }
    }

    [layerThread addChangeRequests:changes];
}

@end

