/*
 *  MarkerManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/16/13.
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
#import "SelectionManager.h"
#import "LayoutManager.h"
#import "Scene.h"

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

    // Clear the contents out of the scene
    void clearContents(SelectionManager *selectManager,LayoutManager *layoutManager,SimpleIdentity generatorId,SimpleIdentity screenGenId,ChangeSet &changes);

    SimpleIDSet drawIDs;  // Drawables created for this
    SimpleIdentity selectID;  // ID used for selection
    SimpleIDSet markerIDs;  // IDs for markers sent to the generator
    SimpleIDSet screenShapeIDs;  // IDs for screen space objects
    float fade;   // Time to fade away for deletion
};
typedef std::set<MarkerSceneRep *,IdentifiableSorter> MarkerSceneRepSet;
    
}

// Used to pass marker information between threads
@interface WhirlyKitMarkerInfo : NSObject
{
    NSArray         *markers;  // Individual marker objects
    UIColor         *color;
    int             drawOffset;
    float           minVis,maxVis;
    bool            screenObject;
    float           width,height;
    int             drawPriority;
    float           fade;
    WhirlyKit::SimpleIdentity  markerId;
}

@property (nonatomic) NSArray *markers;
@property (nonatomic) UIColor *color;
@property (nonatomic,assign) int drawOffset;
@property (nonatomic,assign) float minVis,maxVis;
@property (nonatomic,assign) bool screenObject;
@property (nonatomic,assign) float width,height;
@property (nonatomic,assign) int drawPriority;
@property (nonatomic,assign) float fade;
@property (nonatomic,assign) WhirlyKit::SimpleIdentity markerId;
@property (nonatomic,assign) WhirlyKit::SimpleIdentity replaceID;

- (id)initWithMarkers:(NSArray *)markers desc:(NSDictionary *)desc;

- (void)parseDesc:(NSDictionary *)desc;

@end

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

namespace WhirlyKit
{
    
#define kWKMarkerManager "WKMarkerManager"

/** The Marker Manager is used to create and destroy geometry for 2D and 3D markers.
    It's entirely thread safe except for the 
  */
class MarkerManager : public SceneManager
{
public:
    MarkerManager();
    virtual ~MarkerManager();
    
    /// Add an array of markers, returning the identity that corresponds
    SimpleIdentity addMarkers(NSArray *markers,NSDictionary *desc,ChangeSet &changes);
    
    /// Remove the given set of markers
    void removeMarkers(SimpleIDSet &markerIDs,ChangeSet &changes);
    
    /// Called by the scene once things are set up
    virtual void setScene(Scene *inScene);
    
protected:
    pthread_mutex_t markerLock;
    /// Resources associated with given markers
    MarkerSceneRepSet markerReps;
    /// ID for the marker generator
    WhirlyKit::SimpleIdentity generatorId;
    /// Screen space generator on the render side
    WhirlyKit::SimpleIdentity screenGenId;
};

}
