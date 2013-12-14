/*
 *  MarkerLayer.h
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

#import <math.h>
#import <set>
#import <map>
#import "Identifiable.h"
#import "Drawable.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "TextureAtlas.h"
#import "MarkerManager.h"

/** The Marker Layer Displays a set of markers on the globe.  Markers are simple 
    stamp-like objects that appear where you designate them.  They can have one or 
    more textures associated with them and a period over which to display them.

    Location and visual information for a Marker is controlled by the WGMarker object.
    Other attributes are in the NSDictionary passed in on creation.
     <list type="bullet">
     <item>minVis        [NSNumber float]
     <item>maxVis        [NSNumber float]
     <item>color         [UIColor]
     <item>drawPriority  [NSNumber int]
     <item>drawOffset    [NSNumber int]
     <item>fade          [NSNumber float]
     </list>
 */
@interface WhirlyKitMarkerLayer : NSObject<WhirlyKitLayer> 

/// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene;

/// Called in the layer thread
- (void)shutdown;

/// Add a single marker.  The returned ID can be used to delete or modify it.
- (WhirlyKit::SimpleIdentity) addMarker:(WhirlyKitMarker *)marker desc:(NSDictionary *)desc;

/// Add a whole array of SingleMarker objects.  These will all be identified by the returned ID.
/// To remove them, pass in that ID.  Selection will be based on individual IDs in
//   the SingleMarkers, if set.
- (WhirlyKit::SimpleIdentity) addMarkers:(NSArray *)markers desc:(NSDictionary *)desc;

/// Same as addMarkers, but removes an existing marker at the same time.
- (WhirlyKit::SimpleIdentity) replaceMarker:(WhirlyKit::SimpleIdentity)oldMarkerID withMarkers:(NSArray *)markers desc:(NSDictionary *)desc;

/// Remove one or more markers, designated by their ID
- (void) removeMarkers:(WhirlyKit::SimpleIdentity)markerID;

@end
