/*
 *  GeometryManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/25/14.
 *  Copyright 2012-2014 mousebird consulting
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
#import "Scene.h"
#import "SelectionManager.h"

namespace WhirlyKit
{
    
/** The geometry scene representation keeps track of drawables and other
 resources we've created to represent generic geometry passed in by the
 user.
 */
class GeomSceneRep : public Identifiable
{
public:
    GeomSceneRep() : fade(0.0) { }
    GeomSceneRep(SimpleIdentity theID) : Identifiable(theID) { }
    
    // Drawables created for this geometry
    SimpleIDSet drawIDs;
    
    // IDs kept with the selection manager
    SimpleIDSet selectIDs;
    
    // If set, the amount of time to fade out before deletion
    float fade;
    
    // Remove the contents of this scene rep
    void clearContents(SelectionManager *selectManager,ChangeSet &changes);
    
    // Enable/disable contents
    void enableContents(SelectionManager *selectManager,bool enable,ChangeSet &changes);
};
    
typedef std::set<GeomSceneRep *,IdentifiableSorter> GeomSceneRepSet;

/// Types supported for raw geometry
typedef enum {WhirlyKitGeometryNone,WhirlyKitGeometryLines,WhirlyKitGeometryTriangles} WhirlyKitGeometryRawType;

/// Raw Geometry object.  Fill it in and pass it to the layer.
class GeometryRaw
{
public:
    GeometryRaw();
    GeometryRaw(const GeometryRaw &that);

    /// Simple triangle representation
    class RawTriangle
    {
    public:
        RawTriangle() { }
        /// Construct with three vertex indices
        RawTriangle(int v0,int v1,int v2) { verts[0] = v0; verts[1] = v1; verts[2] = v2; }
        /// Vertices are indices into a vertex array
        int verts[3];
    };
    
    // Compares type and texture ID
    bool operator == (const GeometryRaw &that) const;

    // Runs a consistency check
    bool isValid() const;

    /// Apply the given tranformation matrix to the geometry (and normals)
    void applyTransform(const Eigen::Matrix4d &mat);
    
    // How big the geometry is likely to be in a drawable
    void estimateSize(int &numPts,int &numTris);
    
    // Calculate bounding box
    void calcBounds(Point3d &ll,Point3d &ur);
    
    // Build geometry into a drawable, using the given transform
    void buildDrawables(std::vector<BasicDrawable *> &draws,const Eigen::Matrix4d &mat,const RGBAColor *colorOverride);

public:
    /// What sort of geometry this is
    WhirlyKitGeometryRawType type;
    /// The points (vertices)
    std::vector<WhirlyKit::Point3d> pts;
    /// Normals to go with the points
    std::vector<WhirlyKit::Point3d> norms;
    /// Texture coordinates, one for each point
    std::vector<WhirlyKit::TexCoord> texCoords;
    /// Colors to go with the points
    std::vector<WhirlyKit::RGBAColor> colors;
    /// The triangles, which reference points
    std::vector<RawTriangle> triangles;
    /// A texture ID for the geometry
    int texId;
};

/// Represents a single Geometry Instance
class GeometryInstance : public Identifiable
{
public:
    GeometryInstance() : mat(mat.Identity()), colorOverride(false), selectable(false) { }
    
    // Placement for the instance
    Eigen::Matrix4d mat;
    // Set if we're forcing the colors in an instance
    bool colorOverride;
    RGBAColor color;
    // True if this is selectable
    bool selectable;
};

#define kWKGeometryManager "WKGeometryManager"
    
/** The Geometry manager displays of simple geometric objects,
 such as spheres, lines, and polygons.
 */
class GeometryManager : public SceneManager
{
public:
    GeometryManager();
    ~GeometryManager();
    
    /// Add raw geometry at the given location
    SimpleIdentity addGeometry(std::vector<GeometryRaw> &geom,const std::vector<GeometryInstance> &instances,NSDictionary *desc,ChangeSet &changes);

    /// Enable/disable active billboards
    void enableGeometry(SimpleIDSet &billIDs,bool enable,ChangeSet &changes);
    
    /// Remove a group of billboards named by the given ID
    void removeGeometry(SimpleIDSet &billIDs,ChangeSet &changes);
    
protected:
    pthread_mutex_t geomLock;
    GeomSceneRepSet sceneReps;
};

}
