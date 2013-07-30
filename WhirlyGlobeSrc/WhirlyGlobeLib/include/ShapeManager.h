/*
 *  ShapeManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/16/13.
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

#import "Identifiable.h"
#import "WhirlyVector.h"
#import "DataLayer.h"
#import "layerThread.h"
#import "SelectionManager.h"
#import "Scene.h"

namespace WhirlyKit
{
    
/// Used internally to track shape related resources
class ShapeSceneRep : public Identifiable
{
public:
    ShapeSceneRep();
    ShapeSceneRep(SimpleIdentity inId);
    ~ShapeSceneRep();
    
    // Enable/disable the contents
    void enableContents(WhirlyKit::SelectionManager *selectManager,bool enable,ChangeSet &changeRequests);
    
    // Clear the contents out of the scene
    void clearContents(WhirlyKit::SelectionManager *selectManager,ChangeSet &changeRequests);
    
    SimpleIDSet drawIDs;  // Drawables created for this
    SimpleIDSet selectIDs;  // IDs in the selection layer
    float fade;  // Time to fade away for removal
};

typedef std::set<ShapeSceneRep *,IdentifiableSorter> ShapeSceneRepSet;
    
}

/** The base class for simple shapes we'll draw on top of a globe or
 map.
 */
@interface WhirlyKitShape : NSObject

/// If set, this shape should be made selectable
///  and it will be if the selection layer has been set
@property (nonatomic,assign) bool isSelectable;
/// If the shape is selectable, this is the unique identifier
///  for it.  You should set this ahead of time
@property (nonatomic,assign) WhirlyKit::SimpleIdentity selectID;
/// If set, we'll use the local color
@property (nonatomic,assign) bool useColor;
/// Local color, which will override the default
@property (nonatomic,assign) WhirlyKit::RGBAColor &color;

@end

/// This will display a circle around the location as defined by the base class
@interface WhirlyKitCircle : WhirlyKitShape

/// The location for the origin of the shape
@property (nonatomic,assign) WhirlyKit::GeoCoord &loc;
/// Radius is in display units
@property (nonatomic,assign) float radius;
/// An offset from the globe in display units (radius of the globe = 1.0)
@property (nonatomic,assign) float height;

@end

/// This puts a sphere around the location
@interface WhirlyKitSphere : WhirlyKitShape

/// The location for the origin of the shape
@property (nonatomic,assign) WhirlyKit::GeoCoord &loc;
/// An offset in terms of display units (sphere is radius=1.0)
@property (nonatomic,assign) float height;
/// Radius is in display units
@property (nonatomic,assign) float radius;

@end

/// This puts a cylinder with its base at the locaton
@interface WhirlyKitCylinder : WhirlyKitShape

/// The location for the origin of the shape
@property (nonatomic,assign) WhirlyKit::GeoCoord &loc;
/// Height offset from the ground (in display units)
@property (nonatomic,assign) float baseHeight;
/// Radius in display units
@property (nonatomic,assign) float radius;
/// Height in display units
@property (nonatomic,assign) float height;

@end

/** A linear feature (with width) that we'll draw on
 top of a globe or map.  This is different from the
 vector layer features in that it has exactly locations.
 */
@interface WhirlyKitShapeLinear : WhirlyKitShape

/// Bounding box in local coordinates.
/// Note: Doesn't take height into account
@property (nonatomic,assign) WhirlyKit::Mbr mbr;
/// These locations are in display coordinates
@property (nonatomic,assign) std::vector<WhirlyKit::Point3f> &pts;
/// Line width in pixels
@property (nonatomic,assign) float lineWidth;

@end

namespace WhirlyKit
{
    
#define kWKShapeManager "WKShapeManager"

/** The Shape Manager is used to create and destroy geometry for shapes like circles, cylinders,
    and so forth.  It's entirely thread safe (except for destruction).
  */
class ShapeManager : public SceneManager
{
public:
    ShapeManager();
    virtual ~ShapeManager();
    
    /// Add an array of shapes.  The returned ID can be used to remove or modify the group of shapes.
    SimpleIdentity addShapes(NSArray *shapes,NSDictionary * desc,ChangeSet &changes);
    
    /// Remove a group of shapes named by the given ID
    void removeShapes(SimpleIDSet &shapeIDs,ChangeSet &changes);
    
    /// Enable/disable a group of shapes
    void enableShapes(SimpleIDSet &shapeIDs,bool enable,ChangeSet &changes);
    
protected:
    pthread_mutex_t shapeLock;
    ShapeSceneRepSet shapeReps;
};
    
}
