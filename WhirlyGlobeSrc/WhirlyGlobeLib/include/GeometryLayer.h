/*
 *  GeometryLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/19/12.
 *  Copyright 2012 mousebird consulting
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
#import <boost/shared_ptr.hpp>
#import <boost/pointer_cast.hpp>
#import "WhirlyVector.h"
#import "TextureGroup.h"
#import "GlobeScene.h"
#import "DataLayer.h"
#import "LayerThread.h"

namespace WhirlyGlobe
{
    
/** The geometry scene representation keeps track of drawables and other
    resources we've created to represent generic geometry passed in by the
    user.
 */
class GeomSceneRep : public WhirlyKit::Identifiable
{
public:
    // Drawables created for this geometry
    WhirlyKit::SimpleIDSet drawIDs;
    // If set, the amount of time to fade out before deletion
    float fade;
    
    // Generate the proper objects to remove anything from the scene
    void removeFromScene(std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    // Generate a fade out request for the scene
    void fadeOutScene(std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
};

typedef boost::shared_ptr<GeomSceneRep> GeomSceneRepRef;
typedef std::set<WhirlyGlobe::GeomSceneRepRef,WhirlyKit::IdentifiableRefSorter> GeomSceneRepSet;
    
/// Simple triangle representation
class RawTriangle 
{
public:
    RawTriangle() { }
    RawTriangle(int v0,int v1,int v2) { verts[0] = v0; verts[1] = v1; verts[2] = v2; }
    int verts[3];
};
}

/// Base class for geometry types used in WhirlyGlobeGeometryLayer
@interface WhirlyGlobeGeometry : NSObject
@end

/// Types supported for raw geometry
typedef enum {WhirlyGlobeGeometryNone,WhirlyGlobeGeometryLines,WhirlyGlobeGeometryTriangles} WhilyGlobeGeometryRawType;

/// Raw Geometry object.  Fill it in and pass it to the layer.
@interface WhirlyGlobeGeometryRaw : WhirlyGlobeGeometry
{
    WhilyGlobeGeometryRawType type;
    std::vector<WhirlyKit::Point3f> pts;
    std::vector<WhirlyKit::Point3f> norms;
    std::vector<WhirlyKit::TexCoord> texCoords;
    std::vector<WhirlyKit::RGBAColor> colors;
    std::vector<WhirlyGlobe::RawTriangle> triangles;
    WhirlyKit::SimpleIdentity texId;
}

@property (nonatomic,assign) WhilyGlobeGeometryRawType type;
@property (nonatomic) std::vector<WhirlyKit::Point3f> &pts;
@property (nonatomic) std::vector<WhirlyKit::Point3f> &norms;
@property (nonatomic) std::vector<WhirlyKit::TexCoord> &texCoords;
@property (nonatomic) std::vector<WhirlyKit::RGBAColor> &colors;
@property (nonatomic) std::vector<WhirlyGlobe::RawTriangle> &triangles;
@property (nonatomic,assign) WhirlyKit::SimpleIdentity texId;

// Make a copy of the raw geometry and return it
+ (WhirlyGlobeGeometryRaw *)geometryWithGeometry:(WhirlyGlobeGeometryRaw *)inGeom;

// Runs a consistency check
- (bool)isValid;

/// Apply the given tranformation matrix to the geometry (and normals)
- (void)applyTransform:(Eigen::Matrix4f &)mat;

/// Apply a transform that orients the geometry as if it were a model at the given
///  position with its nose pointed along forward.  You can also rotate (clockwise)
///  around up.
- (void)applyPosition:(WhirlyKit::Point3f)pos up:(WhirlyKit::Point3f)up forward:(WhirlyKit::Point3f)forward heading:(float)ang;

/// Same thing as applyPosition, but it returns the matrix instead of applying it
+ (Eigen::Matrix4f)makePosition:(WhirlyKit::Point3f)pos up:(WhirlyKit::Point3f)up forward:(WhirlyKit::Point3f)forward heading:(float)ang;

/// Construct the drawables for this raw geometry object.
/// Adds them to the drawable array passed in.
/// Be careful when you use this.
- (void)makeDrawables:(std::vector<WhirlyKit::Drawable *> &)drawables;

@end

/** A set of raw geometry and textures, suitable for reading
    from and writing to permanent storage.
 */
@interface WhirlyGlobeGeometrySet : NSObject
{
    std::vector<WhirlyKit::Texture *> textures;
    NSMutableArray *geom;
}

@property (nonatomic,readonly) std::vector<WhirlyKit::Texture *> &textures;
@property (nonatomic,readonly) NSMutableArray *geom;

/// Load cached geometry and textures from the given file
- (id)initWithFile:(NSString *)fullPath;

/// Write to storage using the given base name in the given location
- (bool)writeToFile:(NSString *)fullPath;

@end

/** The Geometry Layer manages the display of simple geometric objects,
    such as spheres, lines, and polygons.
 
    When adding geometry, you can pass in an optional dictionary
    describing how the objects will look.  Any of the following are valid:
     <list type="bullet">
     </list>
  */
@interface WhirlyGlobeGeometryLayer : NSObject<WhirlyKitLayer>
{
    WhirlyKit::Scene *scene;
    WhirlyKitLayerThread * __weak layerThread;    
    
    WhirlyGlobe::GeomSceneRepSet geomReps;
}

/// Add raw geometry at the given location
- (WhirlyKit::SimpleIdentity)addGeometry:(WhirlyGlobeGeometry *)geom desc:(NSDictionary *)desc;

/// Add a group of geometry together
- (WhirlyKit::SimpleIdentity)addGeometryArray:(NSArray *)geom desc:(NSDictionary *)desc;

/// Replace one group of geometry with another
- (WhirlyKit::SimpleIdentity)replaceGeometry:(WhirlyKit::SimpleIdentity)geomID withGeometry:(WhirlyGlobeGeometry *)geom desc:(NSDictionary *)desc;

/// Replace one group of geometry with a whole array
- (WhirlyKit::SimpleIdentity)replaceGeometry:(WhirlyKit::SimpleIdentity)geomID withGeometryArray:(NSArray *)geom desc:(NSDictionary *)desc;

/// Remove an entire set of geometry at once by its ID
- (void)removeGeometry:(WhirlyKit::SimpleIdentity)geomID;

@end
