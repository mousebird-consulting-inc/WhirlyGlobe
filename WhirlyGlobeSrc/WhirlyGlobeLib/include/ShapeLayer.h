/*
 *  ShapeLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/28/12.
 *  Copyright 2011-2012 mousebird consulting
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

#import "Identifiable.h"
#import "WhirlyVector.h"
#import "DataLayer.h"
#import "layerThread.h"
#import "SelectionLayer.h"

namespace WhirlyKit
{
/// Default priority for shapes.
static const int ShapeDrawPriority=1;

/// Maximum number of triangles we'll stick in a drawable
static const int MaxShapeDrawableTris=1<<15/3;
    
/// Used internally to track shape related resources
class ShapeSceneRep : public Identifiable
{
public:
    ShapeSceneRep();
    ShapeSceneRep(SimpleIdentity inId);
    ~ShapeSceneRep();
    
    // Clear the contents out of the scene
    void clearContents(WhirlyKitSelectionLayer *selectLayer,std::vector<ChangeRequest *> &changeRequests);

    SimpleIDSet drawIDs;  // Drawables created for this
    SimpleIdentity selectID;  // ID in the selection layer
    float fade;  // Time to fade away for removal
};

typedef std::set<ShapeSceneRep *,IdentifiableSorter> ShapeSceneRepSet;

}

/** The base class for simple shapes we'll draw on top of a globe or
    map.
 */
@interface WhirlyKitShape : NSObject
{
    /// If set, this shape should be made selectable
    ///  and it will be if the selection layer has been set
    bool isSelectable;
    /// If the shape is selectable, this is the unique identifier
    ///  for it.  You should set this ahead of time
    WhirlyKit::SimpleIdentity selectID;
    /// If set, we'll use the local color
    bool useColor;
    /// Local color, which will override the default
    WhirlyKit::RGBAColor color;
}

@property (nonatomic,assign) bool isSelectable;
@property (nonatomic,assign) WhirlyKit::SimpleIdentity selectID;
@property (nonatomic,assign) bool useColor;
@property (nonatomic,assign) WhirlyKit::RGBAColor &color;

@end

/// This will display a circle around the location as defined by the base class
@interface WhirlyKitCircle : WhirlyKitShape
{
    /// The location for the origin of the shape
    WhirlyKit::GeoCoord loc;
    /// Radius is in display units
    float radius;
    /// An offset from the globe in display units (radius of the globe = 1.0)
    float height;
}

@property (nonatomic,assign) WhirlyKit::GeoCoord &loc;
@property (nonatomic,assign) float radius;
@property (nonatomic,assign) float height;

@end

/// This puts a sphere around the location
@interface WhirlyKitSphere : WhirlyKitShape
{
    /// The location for the origin of the shape
    WhirlyKit::GeoCoord loc;
    /// An offset in terms of display units (sphere is radius=1.0)
    float height;
    /// Radius is in display units
    float radius;
}

@property (nonatomic,assign) WhirlyKit::GeoCoord &loc;
@property (nonatomic,assign) float height;
@property (nonatomic,assign) float radius;

@end

/// This puts a cylinder with its base at the locaton
@interface WhirlyKitCylinder : WhirlyKitShape
{
    /// The location for the origin of the shape
    WhirlyKit::GeoCoord loc;
    /// Height offset from the ground (in display units)
    float baseHeight;
    /// Radius in display units
    float radius;
    /// Height in display units
    float height;
}

@property (nonatomic,assign) WhirlyKit::GeoCoord &loc;
@property (nonatomic,assign) float baseHeight;
@property (nonatomic,assign) float radius;
@property (nonatomic,assign) float height;

@end

/** A linear feature (with width) that we'll draw on
    top of a globe or map.  This is different from the
    vector layer features in that it has exactly locations.
 */
@interface WhirlyKitShapeLinear : WhirlyKitShape
{
    /// Bounding box in local coordinates.
    /// Note: Doesn't take height into account
    WhirlyKit::Mbr mbr;
    /// These locations are in display coordinates
    std::vector<WhirlyKit::Point3f> pts;
    /// Line width in pixels
    float lineWidth;
}

@property (nonatomic,assign) WhirlyKit::Mbr mbr;
@property (nonatomic,assign) std::vector<WhirlyKit::Point3f> &pts;
@property (nonatomic,assign) float lineWidth;

@end

/**  The Shape Layer displays a set of shapes on the globe or map in specified
     locations.  The type of the object determines the sort of shape displayed.
 
     Location and visual information for a Shape is controlled by the associated object.
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
@interface WhirlyKitShapeLayer : NSObject<WhirlyKitLayer>
{
    /// Layer thread this belongs to
    WhirlyKitLayerThread * __weak layerThread;    
    /// Scene the marker layer is modifying
    WhirlyKit::Scene *scene;
    /// If set, we'll pass markers on for selection
    WhirlyKitSelectionLayer * __weak selectLayer;
    /// Used to track the scene objects that correspond to shapes
    WhirlyKit::ShapeSceneRepSet shapeReps;
}

/// Set this for selection layer support.  If this is set
///  and shapes are designated selectable, then the outline
///  of each shape will be passed to the selection layer
///  and will show up in search results.
@property (nonatomic,weak) WhirlyKitSelectionLayer *selectLayer;

/// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene;

/// Called in the layer thread
- (void)shutdown;

/// Add a single shape.  The returned ID can be used to remove or modify the shape.
- (WhirlyKit::SimpleIdentity) addShape:(WhirlyKitShape *)shapes desc:(NSDictionary *)desc;

/// Add an array of shapes.  The returned ID can be used to remove or modify the group of shapes.
- (WhirlyKit::SimpleIdentity) addShapes:(NSArray *)shapes desc:(NSDictionary *)desc;

/// Remove a group of shapes named by the given ID
- (void) removeShapes:(WhirlyKit::SimpleIdentity)shapeID;

@end
