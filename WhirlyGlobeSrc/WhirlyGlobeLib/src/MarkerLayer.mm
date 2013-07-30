/*
 *  MarkerLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/21/11.
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

#import "MarkerLayer.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"
#import "GlobeMath.h"
#import "MarkerGenerator.h"
#import "ScreenSpaceGenerator.h"
#import "LayoutManager.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation WhirlyKitMarkerLayer
{
    /// Layer thread this belongs to
    WhirlyKitLayerThread * __weak layerThread;
    /// Scene the marker layer is modifying
    WhirlyKit::Scene *scene;

    SimpleIDSet markerIDs;
}

- (void)clear
{
    markerIDs.clear();
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
    MarkerManager *markerManager = (MarkerManager *)scene->getManager(kWKMarkerManager);
    ChangeSet changes;
    
    if (markerManager)
        markerManager->removeMarkers(markerIDs,changes);
    
    [layerThread addChangeRequests:changes];
    
    [self clear];
}

// Add a single marker 
- (SimpleIdentity) addMarker:(WhirlyKitMarker *)marker desc:(NSDictionary *)desc
{
    return [self addMarkers:@[marker] desc:desc];
}

// Add a group of markers
- (SimpleIdentity) addMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
        NSLog(@"WhirlyGlobe Marker layer has not been initialized or is not being called from the layer thread, yet you're calling addMarker.  Dropping data on floor.");
        return EmptyIdentity;
    }
    
    ChangeSet changes;
    SimpleIdentity markerID = EmptyIdentity;
    MarkerManager *markerManager = (MarkerManager *)scene->getManager(kWKMarkerManager);
    if (markerManager)
        markerID = markerManager->addMarkers(markers, desc, changes);
    if (markerID != EmptyIdentity)
        markerIDs.insert(markerID);
    [layerThread addChangeRequests:changes];
    
    return markerID;
}

- (WhirlyKit::SimpleIdentity) replaceMarker:(WhirlyKit::SimpleIdentity)oldMarkerID withMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
        NSLog(@"WhirlyGlobe Marker layer has not been initialized or is not being called from the layer thread, yet you're calling replaceMarker.  Dropping data on floor.");
        return EmptyIdentity;
    }
    
    ChangeSet changes;
    SimpleIdentity markerID = EmptyIdentity;
    MarkerManager *markerManager = (MarkerManager *)scene->getManager(kWKMarkerManager);
    if (markerManager)
    {
        SimpleIDSet toRemove;
        toRemove.insert(oldMarkerID);
        markerManager->removeMarkers(toRemove, changes);
        markerID = markerManager->addMarkers(markers, desc, changes);
    }
    [layerThread addChangeRequests:changes];
    if (markerID != EmptyIdentity)
        markerIDs.insert(markerID);
    
    return markerID;
}

// Remove a group of markers
- (void) removeMarkers:(SimpleIdentity)markerID
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
        NSLog(@"WhirlyGlobe Marker layer has not been initialized or is not being called from the layer thread, yet you're calling removeMarkers.  Dropping data on floor.");
        return;
    }
    
    ChangeSet changes;

    SimpleIDSet::iterator it = markerIDs.find(markerID);
    if (it != markerIDs.end())
    {
        markerIDs.erase(it);
        MarkerManager *markerManager = (MarkerManager *)scene->getManager(kWKMarkerManager);
        if (markerManager)
        {
            SimpleIDSet toRemove;
            toRemove.insert(markerID);
            markerManager->removeMarkers(toRemove, changes);
        }
    }
    
    [layerThread addChangeRequests:changes];
}


@end
