/*
 *  MaplyActiveScreenMarker.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/21/13.
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

#import "MaplyActiveScreenMarker.h"
#import "WhirlyGlobe.h"
#import "MaplyActiveObject_private.h"
#import "MarkerManager.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyActiveScreenMarker
{
    MaplyScreenMarker *screenMarker;
    NSDictionary *desc;
    UIImage *image;
    SimpleIdentity texID;
    SimpleIDSet markerIDs;
}

- (void)setScreenMarker:(MaplyScreenMarker *)newMarker desc:(NSDictionary *)inDesc;
{
    if (dispatchQueue)
    {
        dispatch_async(dispatchQueue,
                       ^{
                           @synchronized(self)
                           {
                               screenMarker = newMarker;
                               desc = inDesc;
                               [self update];
                           }
                       }
                       );
    } else {
        @synchronized(self)
        {
            screenMarker = newMarker;
            desc = inDesc;
            [self update];
        }        
    }
}

// Create new geometry
- (void)update
{
    ChangeSet changes;
    
    MarkerManager *markerManager = (MarkerManager *)scene->getManager(kWKMarkerManager);
    
    if (markerManager)
        markerManager->removeMarkers(markerIDs, changes);
    markerIDs.clear();
    
    // And possibly the image
    if (image != screenMarker.image)
    {
        if (texID != EmptyIdentity)
        {
            changes.push_back(new RemTextureReq(texID));
            texID = EmptyIdentity;
        }
        image = nil;
    }
    
    if (screenMarker)
    {
        // New image
        if (screenMarker.image)
        {
            Texture *tex = new Texture("Active ScreenLabel",screenMarker.image);
            texID = tex->getId();
            image = screenMarker.image;
            changes.push_back(new AddTextureReq(tex));
        }

        // Need the WK version of the Marker
        WhirlyKitMarker *marker = [[WhirlyKitMarker alloc] init];
        marker.loc = GeoCoord(screenMarker.loc.x,screenMarker.loc.y);
        if (texID != EmptyIdentity)
            marker.texIDs.push_back(texID);
        marker.width = screenMarker.size.width;
        marker.height = screenMarker.size.height;
        if (screenMarker.selectable)
        {
            marker.isSelectable = true;
            marker.selectID = Identifiable::genId();
        }
        marker.layoutImportance = screenMarker.layoutImportance;
        
        if (markerManager)
        {
            SimpleIdentity markerID = markerManager->addMarkers(@[marker], desc, changes);
            if (markerID != EmptyIdentity)
                markerIDs.insert(markerID);
        }
    }
    
    
    scene->addChangeRequests(changes);
}

- (void)shutdown
{
    @synchronized(self)
    {
        screenMarker = nil;
        [self update];
    }
}


@end

