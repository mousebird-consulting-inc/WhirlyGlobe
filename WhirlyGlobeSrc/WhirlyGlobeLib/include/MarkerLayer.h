/*
 *  MarkerLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/21/11.
 *  Copyright 2011-2012 mousebird consulting. All rights reserved.
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
#import "DrawCost.h"
#import "SelectionLayer.h"
#import "LayoutLayer.h"

namespace WhirlyKit
{
/// Default priority for markers.  At the end, basically
static const int MarkerDrawPriority=10000;
    
/// Maximum number of triangles we'll stick in a drawable
static const int MaxMarkerDrawableTris=1<<15/3;
}

namespace WhirlyKit
{

/// Marker representation.
/// Used internally to track marker resources
class MarkerSceneRep : public Identifiable
{
public:
    MarkerSceneRep();
    ~MarkerSceneRep() { };
    
    SimpleIDSet drawIDs;  // Drawables created for this
    SimpleIDSet selectIDs;  // Selection rect
    SimpleIDSet markerIDs;  // IDs for markers sent to the generator
    SimpleIDSet screenShapeIDs;  // IDs for screen space objects
    float fade;   // Time to fade away for deletion
};
typedef std::set<MarkerSceneRep *,IdentifiableSorter> MarkerSceneRepSet;
    
}

/** WhirlyGlobe Marker
    A single marker object to be placed on the globe.  It will show
    up with the given width and height and be selectable if so desired.
 */
@interface WhirlyKitMarker : NSObject
{
    /// If set, this marker should be made selectable
    ///  and it will be if the selection layer has been set
    bool isSelectable;
    /// If the marker is selectable, this is the unique identifier
    ///  for it.  You should set this ahead of time
    WhirlyKit::SimpleIdentity selectID;
    /// The location for the center of the marker.
    WhirlyKit::GeoCoord loc;
    /// The list of textures to use.  If there's just one
    ///  we show that.  If there's more than one, we switch
    ///  between them over the period.
    std::vector<WhirlyKit::SimpleIdentity> texIDs;
    /// The width in 3-space (remember the globe has radius = 1.0)
    float width;
    /// The height in 3-space (remember the globe has radius = 1.0)
    float height;
    /// Set if we want a static rotation.  Only matters in screen space
    bool lockRotation;
    /// This is rotation clockwise from north in radians
    float rotation;
    /// The period over which we'll switch textures
    NSTimeInterval period;
    /// For markers with more than one texture, this is the offset
    ///  we'll use when calculating position within the period.
    NSTimeInterval timeOffset;
    /// Value to use for the layout engine.  Set to MAXFLOAT by
    ///  default, which will always display.
    float layoutImportance;
}

@property (nonatomic,assign) bool isSelectable;
@property (nonatomic,assign) WhirlyKit::SimpleIdentity selectID;
@property (nonatomic,assign) WhirlyKit::GeoCoord loc;
@property (nonatomic,assign) std::vector<WhirlyKit::SimpleIdentity> &texIDs;
@property (nonatomic,assign) bool lockRotation;
@property (nonatomic,assign) float width,height,rotation;
@property (nonatomic,assign) NSTimeInterval period;
@property (nonatomic,assign) NSTimeInterval timeOffset;
@property (nonatomic,assign) float layoutImportance;

/// Add a texture ID to be displayed
- (void)addTexID:(WhirlyKit::SimpleIdentity)texID;

@end

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
{
    /// Layer thread this belongs to
    WhirlyKitLayerThread * __weak layerThread;
    /// ID for the marker generator
    WhirlyKit::SimpleIdentity generatorId;    
    /// Scene the marker layer is modifying
    WhirlyKit::Scene *scene;
    /// If set, we'll pass markers on for selection
    WhirlyKitSelectionLayer * __weak selectLayer;
    /// If set, this is the layout layer we'll pass some labels off to (those being laid out)
    WhirlyKitLayoutLayer * __weak layoutLayer;
    /// Used to track what scene components correspond to which markers
    WhirlyKit::MarkerSceneRepSet markerReps;
    /// Screen space generator on the render side
    WhirlyKit::SimpleIdentity screenGenId;
}

/// Set this for selection layer support.  If this is set
///  and markers are designated selectable, then the outline
///  of each marker will be passed to the selection layer
///  and will show up in search results.
@property (nonatomic,weak) WhirlyKitSelectionLayer *selectLayer;

/// Set this to use the layout engine for markers so marked
@property (nonatomic,weak) WhirlyKitLayoutLayer *layoutLayer;

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
