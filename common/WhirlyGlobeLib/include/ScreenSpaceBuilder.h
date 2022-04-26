/*  ScreenSpaceBuild.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/21/14.
 *  Copyright 2011-2022 mousebird consulting.
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
 */

#import "Identifiable.h"
#import "BasicDrawable.h"
#import "TextureAtlas.h"
#import "ScreenSpaceDrawableBuilder.h"
#import "Scene.h"
#import "BaseInfo.h"

#import <math.h>
#import <map>
#import <set>
#import <unordered_set>

namespace WhirlyKit
{

class ScreenSpaceObject;
using ScreenSpaceObjectRef = std::shared_ptr<ScreenSpaceObject>;
struct ScreenSpaceConvexGeometry;
struct ScreenSpaceObjectLocation;

typedef std::unordered_set<SimpleIdentity> SimpleIDUnorderedSet;

/** Screen space objects are used for both labels and markers.  This builder
    helps construct the drawables needed to represent them.
  */
class ScreenSpaceBuilder
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    ScreenSpaceBuilder(SceneRenderer *,CoordSystemDisplayAdapter *,
                       float scale, float centerDist = 10e2f);
    virtual ~ScreenSpaceBuilder() = default;
    
    // State information we're keeping around.
    // Defaults to something reasonable
    struct DrawableState
    {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

        DrawableState() = default;
        DrawableState(const DrawableState&) = default;
        DrawableState(DrawableState&&) noexcept;
        DrawableState& operator=(const DrawableState &) = default;
        DrawableState& operator=(DrawableState &&) noexcept;

        // Comparison operator for set
        bool operator <(const DrawableState &that) const;

        std::vector<SimpleIdentity> texIDs;
        double period = 0.0;
        SimpleIdentity progID = EmptyIdentity;
        TimeInterval fadeUp = 0.0;
        TimeInterval fadeDown = 0.0;
        TimeInterval startEnable = 0.0;
        TimeInterval endEnable = 0.0;
        int64_t drawOrder = 0;
        SimpleIdentity renderTargetID = EmptyIdentity;
        double minZoomVis = DrawVisibleInvalid;
        double maxZoomVis = DrawVisibleInvalid;
        float minVis = DrawVisibleInvalid;
        float maxVis = DrawVisibleInvalid;
        int zoomSlot = -1;
        int drawPriority = 0;
        bool enable = true;
        bool motion = false;
        bool rotation = false;
        bool keepUpright = false;
        bool hasMask = false;
        FloatExpressionInfoRef opacityExp;
        ColorExpressionInfoRef colorExp;
        FloatExpressionInfoRef scaleExp;
        SingleVertexAttributeInfoSet vertexAttrs;
    };
    
    /// Draw priorities can mix and match with other objects, but we probably don't want that
    //void setDrawPriorityOffset(int drawPriorityOffset);
    
    /// Set the active texture ID
    void setTexID(SimpleIdentity texID);
    /// Set the active texture IDs
    void setTexIDs(const std::vector<SimpleIdentity> &texIDs,double period);
    /// Set the active program ID
    void setProgramID(SimpleIdentity progID);
    /// Set the fade in/out
    void setFade(TimeInterval fadeUp,TimeInterval fadeDown);
    /// Set the draw order
    void setDrawOrder(int64_t drawOrder);
    /// Set the draw priority
    void setDrawPriority(int drawPriority);
    /// Set the render target
    void setRenderTarget(SimpleIdentity renderTargetID);
    /// Set the visibility range
    void setVisibility(float minVis,float maxVis);
    /// Set enable based on zoom
    void setZoomInfo(int zoomSlot,double minZoomVis,double maxZoomVis);
    /// Set the opacity function if we have one
    void setOpacityExp(FloatExpressionInfoRef opacityExp);
    /// Set the color function if there is one
    void setColorExp(ColorExpressionInfoRef colorExp);
    /// Set the size function if there is one
    void setScaleExp(FloatExpressionInfoRef sizeExp);
    /// Set the start enable
    void setEnable(bool enable);
    /// Set the enable time range
    void setEnableRange(TimeInterval inStartEnable,TimeInterval inEndEnable);

    /// Add a single rectangle with no rotation
    void addRectangle(const Point3d &worldLoc,const Point2d *coords,
                      const TexCoord *texCoords,const RGBAColor &color,
                      SimpleIDUnorderedSet *drawIDs = nullptr);
    /// Add a single rectangle with rotation, possibly keeping upright
    void addRectangle(const Point3d &worldLoc,double rotation,bool keepUpright,
                      const Point2d *coord,const TexCoord *texCoords,const RGBAColor &color,
                      SimpleIDUnorderedSet *drawIDs = nullptr);

    /// Add a whole bunch of predefined Scene Objects
    /// These will be sorted by orderBy
    void addScreenObjects(std::vector<ScreenSpaceObject> &screenObjects,
                          const std::vector<Eigen::Matrix3d> *places = nullptr,
                          SimpleIDUnorderedSet *drawIDs = nullptr);
    void addScreenObjects(std::vector<ScreenSpaceObject *> &screenObjects,
                          const std::vector<Eigen::Matrix3d> *places = nullptr,
                          SimpleIDUnorderedSet *drawIDs = nullptr);
    void addScreenObjects(std::vector<ScreenSpaceObjectRef> &screenObjects,
                          const std::vector<Eigen::Matrix3d> *places = nullptr,
                          SimpleIDUnorderedSet *drawIDs = nullptr);

    /// Add a single screen space object
    void addScreenObject(const ScreenSpaceObject &screenObject,
                         const Point3d &worldLoc,
                         const std::vector<ScreenSpaceConvexGeometry> *geoms,
                         const std::vector<Eigen::Matrix3d> *places = nullptr,
                         SimpleIDUnorderedSet *drawIDs = nullptr);

    /// Return the drawables constructed.  Caller responsible for deletion.
    void buildDrawables(std::vector<BasicDrawableRef> &draws);
    
    /// Build drawables and add them to the change list
    std::vector<BasicDrawableRef> flushChanges(ChangeSet &changes,SimpleIDSet *drawIDs = nullptr);
    std::vector<BasicDrawableRef> flushChanges(ChangeSet &changes,SimpleIDSet &drawIDs);
    
    /// Calculate the rotation vector for a rotation
    static Point3d CalcRotationVec(CoordSystemDisplayAdapter *coordAdapter,const Point3d &worldLoc,float rot);
    
protected:
    // Wrapper used to track
    struct DrawableWrap
    {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
        
        DrawableWrap(SceneRenderer *sceneRender,const DrawableState &state);
        ~DrawableWrap() = default;
        
        void addVertex(CoordSystemDisplayAdapter *,float scale, const Point3d &worldLoc,
                       const Point3f *dir,float rot, const Point2d &inVert,
                       const TexCoord *texCoord, const RGBAColor *color,
                       const SingleVertexAttributeSet *vertAttrs);
        void addTri(int v0,int v1,int v2);
        
        Point3d center = { 0, 0, 0 };
        DrawableState state;
        
        const ScreenSpaceDrawableBuilderRef &getDrawableBuilder() const { return locDraw; }
        ScreenSpaceDrawableBuilderRef locDraw;
    };

    typedef std::shared_ptr<DrawableWrap> DrawableWrapRef;
    typedef std::map<DrawableState,DrawableWrapRef> DrawableWrapMap;
    
    DrawableWrapRef findOrAddDrawWrap(const DrawableState &state,int numVerts,int numTri,const Point3d &center);
    
    float centerDist;
    float scale;
    int drawPriorityOffset = 0;
    SceneRenderer *sceneRender;
    CoordSystemDisplayAdapter *coordAdapter;
    DrawableState curState;
    DrawableWrapMap drawables;
    std::vector<DrawableWrapRef> fullDrawables;
};

/// Represents a simple set of convex geometry
struct ScreenSpaceConvexGeometry
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    ScreenSpaceConvexGeometry() = default;
    ScreenSpaceConvexGeometry(const ScreenSpaceConvexGeometry &) = default;
    ScreenSpaceConvexGeometry& operator=(const ScreenSpaceConvexGeometry &) = default;
    ScreenSpaceConvexGeometry(ScreenSpaceConvexGeometry&&) noexcept;
    ScreenSpaceConvexGeometry& operator=(ScreenSpaceConvexGeometry &&) noexcept;

    /// Texture ID used for just this object
    std::vector<SimpleIdentity> texIDs;
    /// Program ID used to render this geometry
    SimpleIdentity progID = EmptyIdentity;
    /// Color for the geometry
    RGBAColor color = RGBAColor::white();
    /// Draw order
    int64_t drawOrder = BaseInfo::DrawOrderTiles;
    /// Draw priority
    int drawPriority = -1;
    /// Render target
    SimpleIdentity renderTargetID = EmptyIdentity;
    /// Vertex attributes applied to this piece of geometry
    SingleVertexAttributeSet vertexAttrs;
    
    Point2dVector coords;
    std::vector<TexCoord> texCoords;
};
    
/** Keeps track of the basic information about a screen space object.
 */
class ScreenSpaceObject : public Identifiable
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    friend class LayoutManager;
    friend class SelectionManager;
    friend class ScreenSpaceBuilder;

    ScreenSpaceObject() = default;
    ScreenSpaceObject(SimpleIdentity theId);
    ScreenSpaceObject(const ScreenSpaceObject &) = default;
    ScreenSpaceObject(ScreenSpaceObject &&) noexcept;
    ScreenSpaceObject &operator=(const ScreenSpaceObject&) = default;
    ScreenSpaceObject &operator=(ScreenSpaceObject&&) noexcept;
    virtual ~ScreenSpaceObject() = default;
        
    /// Center of the object in world coordinates
    void setWorldLoc(const Point3d &worldLoc);
    Point3d getWorldLoc() const;
    Point3d getEndWorldLoc() const;
    TimeInterval getStartTime() const;
    TimeInterval getEndTime() const;
    
    /// Set up the end location and timing
    void setMovingLoc(const Point3d &worldLoc,TimeInterval startTime,TimeInterval endTime);
    
    void setEnable(bool enable);
    void setEnableTime(TimeInterval startEnable,TimeInterval endEnable);
    void setVisibility(float minVis,float maxVis);
    void setZoomInfo(int zoomSlot,double minZoomVis,double maxZoomVis);
    void setOpacityExp(FloatExpressionInfoRef opacityExp);
    void setColorExp(ColorExpressionInfoRef colorExp);
    void setScaleExp(FloatExpressionInfoRef scaleExp);
    void setDrawOrder(int64_t drawOrder);
    int64_t getDrawOrder() const;
    void setDrawPriority(int drawPriority);
    int getDrawPriority() const;
    void setRenderTarget(SimpleIdentity renderTargetID);
    void setKeepUpright(bool keepUpright);
    void setRotation(double rotation);
    double getRotation() const { return rotation; }
    bool hasRotation() const { return state.rotation; };
    void setFade(TimeInterval fadeUp,TimeInterval fadeDown);
    void setOffset(const Point2d &offset);
    void setPeriod(TimeInterval period);
    void setOrderBy(long orderBy);

    void addGeometry(const ScreenSpaceConvexGeometry &geom);
    void addGeometry(ScreenSpaceConvexGeometry &&geom);
    void addGeometry(std::vector<ScreenSpaceConvexGeometry> &&geom);
    void addGeometry(const std::vector<ScreenSpaceConvexGeometry> &geom);
    const std::vector<ScreenSpaceConvexGeometry> *getGeometry() const { return &geometry; }
    
    // Get a program ID either from the drawable state or geometry
    SimpleIdentity getTypicalProgramID();
    
protected:
    bool enable = true;
    TimeInterval startEnable = 0.0;
    TimeInterval endEnable = 0.0;
    Point3d worldLoc {0,0,0};
    Point3d endWorldLoc { 0, 0, 0 };
    TimeInterval startTime = 0.0;
    TimeInterval endTime = 0.0;
    Point2d offset { 0, 0 };
    double rotation = 0.0;
    long orderBy = -1;
    bool keepUpright = false;
    ScreenSpaceBuilder::DrawableState state;
    std::vector<ScreenSpaceConvexGeometry> geometry;
};
using ScreenSpaceObjectRef = std::shared_ptr<ScreenSpaceObject>;

/** We use the screen space object location to communicate where
    a screen space object is on the screen.
  */
struct ScreenSpaceObjectLocation
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    ScreenSpaceObjectLocation() = default;
    ScreenSpaceObjectLocation(const ScreenSpaceObjectLocation &) = default;
    ScreenSpaceObjectLocation& operator=(const ScreenSpaceObjectLocation &) = default;
    ScreenSpaceObjectLocation(ScreenSpaceObjectLocation &&) noexcept;
    ScreenSpaceObjectLocation& operator=(ScreenSpaceObjectLocation &&) noexcept;

    bool isCluster() const { return clusterId != EmptyIdentity; }

    // IDs for selected objects (one if regular, more than one for cluster)
    std::vector<SimpleIdentity> shapeIDs;
    // Location of object in display space
    Point3d dispLoc {0,0,0};
    // Offset on the screen (presumably if it's been moved around during layout)
    Point2d offset {0,0};
    // Set if we're supposed to stay vertical
    bool keepUpright = false;
    // Rotation if there is one
    double rotation = 0.0;
    // Size of the object in screen space
    Point2dVector pts;
    // Bounding box, for quick testing
    Mbr mbr;

    // The cluster group that this cluster came from, if it's a cluster.
    int clusterGroup = -1;
    // The ID of the cluster object, for re-grouping objects later
    SimpleIdentity clusterId = EmptyIdentity;
};
    
}
