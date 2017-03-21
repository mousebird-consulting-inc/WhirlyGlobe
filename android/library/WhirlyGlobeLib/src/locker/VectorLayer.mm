/*
 *  VectorLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/26/11.
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

#import "WhirlyGeometry.h"
#import "VectorLayer.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"
#import "Tesselator.h"

using namespace WhirlyKit;

@implementation WhirlyKitVectorLayer
{
    /// Vectors currently being represented
    WhirlyKit::SimpleIDSet vecIDs;
    WhirlyKit::Scene *scene;
    WhirlyKitLayerThread * __weak layerThread;
}

- (void)clear
{
    vecIDs.clear();
    scene = NULL;
}

- (void)dealloc
{
    [self clear];
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
	scene = inScene;
    layerThread = inLayerThread;
}

- (void)shutdown
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    
    if (!scene)
        return;
    
    ChangeSet changes;
    
    VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);
    if (vectorManager)
        vectorManager->removeVectors(vecIDs, changes);
    
    [layerThread addChangeRequests:changes];
    
    [self clear];
}

// Add a vector
// We make up an ID for it before it's actually created
- (SimpleIdentity)addVector:(VectorShapeRef)shape desc:(NSDictionary *)dict
{
    ShapeSet shapes;
    shapes.insert(shape);
    return [self addVectors:&shapes desc:dict];
}

// Add a group of vectors and cache it to the given file, which might be on disk
- (SimpleIdentity)addVectors:(ShapeSet *)shapes desc:(NSDictionary *)desc
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
        NSLog(@"WhirlyGlobe Vector layer has not been initialized or addVectors is being called in the wrong thread.  Dropping data on floor.");
        return EmptyIdentity;
    }
    
    SimpleIdentity vecID = EmptyIdentity;
    ChangeSet changes;
    VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);
    if (vectorManager)
    {
        vecID = vectorManager->addVectors(shapes, desc, changes);
        if (vecID != EmptyIdentity)
            vecIDs.insert(vecID);
    }
    
    [layerThread addChangeRequests:changes];
    return vecID;
}

// Change how the vector is represented
- (void)changeVector:(SimpleIdentity)vecID desc:(NSDictionary *)desc
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
        NSLog(@"WhirlyGlobe Vector layer has not been initialized or changeVector is being called in the wrong thread.  Dropping data on floor.");
        return;
    }
    
    ChangeSet changes;
    VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);
    if (vectorManager && vecIDs.find(vecID) != vecIDs.end())
        vectorManager->changeVectors(vecID, desc, changes);
    
    [layerThread addChangeRequests:changes];    
}

// Replace the given set of vectors wit the new one
- (WhirlyKit::SimpleIdentity)replaceVector:(WhirlyKit::SimpleIdentity)oldVecID withVectors:(WhirlyKit::ShapeSet *)shapes desc:(NSDictionary *)desc
{
    if (!layerThread || !scene)
    {
        NSLog(@"WhirlyGlobe Vector layer has not been initialized or replaceVector is being called in the wrong thread.  Dropping data on floor.");
        return EmptyIdentity;
    }
    
    SimpleIdentity newVecID = EmptyIdentity;
    ChangeSet changes;
    VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);
    SimpleIDSet::iterator it = vecIDs.find(oldVecID);
    if (vectorManager)
    {
        if (it != vecIDs.end())
        {
            SimpleIDSet theIDs;
            theIDs.insert(oldVecID);
            vectorManager->removeVectors(theIDs, changes);
            vecIDs.erase(it);
        }
        SimpleIdentity newVecID = vectorManager->addVectors(shapes, desc, changes);
        if (newVecID != EmptyIdentity)
            vecIDs.insert(newVecID);
    }
    
    [layerThread addChangeRequests:changes];
    
    return newVecID;
}

- (void)enableVector:(WhirlyKit::SimpleIdentity)vecID enable:(bool)enable
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
        NSLog(@"WhirlyGlobe Vector layer has not been initialized or enableVector is being called in the wrong thread.  Dropping data on floor.");
        return;
    }
    
    ChangeSet changes;
    VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);
    if (vectorManager && vecIDs.find(vecID) != vecIDs.end())
    {
        SimpleIDSet theIDs;
        theIDs.insert(vecID);
        vectorManager->enableVectors(theIDs, enable, changes);
    }
    
    [layerThread addChangeRequests:changes];
}

// Remove the vector
- (void)removeVector:(SimpleIdentity)vecID
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
        NSLog(@"WhirlyGlobe Vector layer has not been initialized or enableVector is being called in the wrong thread.  Dropping data on floor.");
        return;
    }

    ChangeSet changes;
    VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);
    SimpleIDSet::iterator it = vecIDs.find(vecID);
    if (vectorManager && it != vecIDs.end())
    {
        SimpleIDSet theIDs;
        theIDs.insert(vecID);
        vectorManager->removeVectors(theIDs, changes);
        vecIDs.erase(it);
    }
    
    [layerThread addChangeRequests:changes];
}

@end
