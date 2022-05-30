/*  SelectionManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/26/11.
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

#import <math.h>
#import <set>
#import <map>
#import "Identifiable.h"
#import "WhirlyGeometry.h"
#import "WhirlyKitView.h"
#import "MaplyView.h"
#import "GlobeView.h"
#import "Scene.h"
#import "ScreenSpaceBuilder.h"
#import "VectorObject.h"

namespace WhirlyKit
{
    
class Scene;
class SceneManager;

/// Base class for selectable geometry
struct Selectable
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    Selectable() = default;
    Selectable(SimpleIdentity theID) : selectID(theID) { }

    bool isVisibleAt(double heightAboveSurface) const
    {
        return enable && selectID != EmptyIdentity &&
            (minVis == DrawVisibleInvalid || heightAboveSurface >= minVis) &&
            (maxVis == DrawVisibleInvalid || heightAboveSurface <= maxVis);
    }

    bool enable = true;
    /// Used to identify this selectable
    SimpleIdentity selectID = EmptyIdentity;
    float minVis = DrawVisibleInvalid;
    float maxVis = DrawVisibleInvalid;  // Range over which this is visible
};

/** This is used internally to the selection layer to track a
    selectable rectangle.  It consists of geometry and an
    ID to track it.
  */
struct RectSelectable3D : public Selectable
{
    RectSelectable3D() = default;
    RectSelectable3D(SimpleIdentity theID) : Selectable(theID) { }

    // Comparison operator for sorting
    bool operator < (const RectSelectable3D &that) const;
    
    Point3f pts[4];  // Geometry
    Eigen::Vector3f norm;   // Calculate normal
};

typedef std::set<WhirlyKit::RectSelectable3D> RectSelectable3DSet;

/** This is 3D solid.
  */
struct PolytopeSelectable : public Selectable
{
    PolytopeSelectable() = default;
    PolytopeSelectable(SimpleIdentity theID) : Selectable(theID) { }
    PolytopeSelectable(const PolytopeSelectable&) = default;
    PolytopeSelectable(PolytopeSelectable&& other) noexcept :
            Selectable(other.selectID),
            polys(std::move(other.polys)),
            centerPt(other.centerPt)
    {
    }

    // Comparison operator for sorting
    bool operator < (const PolytopeSelectable &that) const;

    std::vector<Point3fVector> polys;
    Point3d centerPt;        // The polygons are offsets of this center
};

typedef std::set<WhirlyKit::PolytopeSelectable> PolytopeSelectableSet;
    
/** 3D solid that can move over time.
  */
struct MovingPolytopeSelectable : public PolytopeSelectable
{
    MovingPolytopeSelectable() = default;
    MovingPolytopeSelectable(SimpleIdentity theID) : PolytopeSelectable(theID) { }
    MovingPolytopeSelectable(const MovingPolytopeSelectable &) = default;
    MovingPolytopeSelectable(MovingPolytopeSelectable&& other) noexcept :
        PolytopeSelectable(std::move(other)),
        endCenterPt(other.endCenterPt),
        startTime(other.startTime),
        duration(other.duration)
    {
    }

    // Comparison operator for sorting
    bool operator < (const MovingPolytopeSelectable &that) const;
    
    Point3d endCenterPt;
    TimeInterval startTime = 0.0;
    TimeInterval duration = 0.0;
};
    
typedef std::set<WhirlyKit::MovingPolytopeSelectable> MovingPolytopeSelectableSet;
    
/** This is a linear features with arbitrary 3D points.
  */
struct LinearSelectable : public Selectable
{
    LinearSelectable() = default;
    LinearSelectable(SimpleIdentity theID) : Selectable(theID) { }
    LinearSelectable(const LinearSelectable &) = default;
    LinearSelectable(LinearSelectable &&other) noexcept :
        Selectable(other.selectID),
        pts(std::move(other.pts))
    {
    }

    // Comparison operator for sorting
    bool operator < (const LinearSelectable &that) const;
    
    Point3dVector pts;
};

typedef std::set<WhirlyKit::LinearSelectable> LinearSelectableSet;

/** Rectangle Selectable (screen space version).
 */
struct RectSelectable2D : public Selectable
{
    RectSelectable2D() = default;
    RectSelectable2D(SimpleIdentity theID) : Selectable(theID) { }
    RectSelectable2D(const RectSelectable2D &) = default;

    // Comparison operator for sorting
    bool operator < (const RectSelectable2D &that) const;
    
    Point3d center;  // Location of the center of the rectangle
    Point2f pts[4];  // Geometry
};

typedef std::set<WhirlyKit::RectSelectable2D> RectSelectable2DSet;

/** Rectangle selectable that moves over time.
  */
struct MovingRectSelectable2D : public RectSelectable2D
{
    MovingRectSelectable2D() = default;
    MovingRectSelectable2D(SimpleIdentity theID) : RectSelectable2D(theID) { }
    MovingRectSelectable2D(const MovingRectSelectable2D &) = default;
    
    // Calculate the center based on the time
    Point3d centerForTime(TimeInterval now) const;
    
    Point3d endCenter;                  // Location at the end of the time period
    TimeInterval startTime = 0.0;
    TimeInterval endTime = 0.0;         // Start and end time
};

typedef std::set<WhirlyKit::MovingRectSelectable2D> MovingRectSelectable2DSet;

/// Billboard selectable (3D object that turns towards the viewer)
struct BillboardSelectable : public Selectable
{
    BillboardSelectable() = default;
    BillboardSelectable(SimpleIdentity theID) : Selectable(theID) { }
    // Comparison operator for sorting
    bool operator < (const BillboardSelectable &that) const;
    
    Point3d center;  // Location of the middle of the base in display space
    Point3d normal;  // The billboard points up in this direction
    Point2d size;    // Size of the billboard in display space
};
  
typedef std::set<WhirlyKit::BillboardSelectable> BillboardSelectableSet;
    
#define kWKSelectionManager "WKSelectionManager"
    
/** The selection manager tracks a variable number of objects that
     might be selectable.  These consist of a shape and an ID.
    Layers (or the caller) can register objects with the
     selection layer.  These objects will be considered for selection
     when the caller uses pickObject.
 
    All objects are currently being projected to the 2D screen and
     evaluated for distance there.
 
    The selection manager is entirely thread safe except for destruction.
 */
class SelectionManager : public SceneManager
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    /// Pass in the content scaling (not 1.0 if we're on retina)
    SelectionManager(Scene *scene);
    virtual ~SelectionManager() = default;
    
    /// When we're selecting multiple objects we'll return a list of these
    struct SelectedObject
    {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

        SelectedObject(double distIn3D = 0.0, double screenDist = 0.0);
        SelectedObject(SimpleIdentity selectID, double distIn3D, double screenDist);
        SelectedObject(std::vector<SimpleIdentity> selectIDs, double distIn3D, double screenDist);
        SelectedObject(const SelectedObject&) = default;
        SelectedObject &operator=(const SelectedObject&) = default;
        SelectedObject &operator=(SelectedObject&&) noexcept;

        std::vector<SimpleIdentity> selectIDs;    // What we selected.  If it was a cluster, could be more than one
        VectorObjectRef vecObj;     // On the Android side, we use this as a container for selected vectors
        GeoCoord center = { 0, 0 }; // geo location
        double distIn3D;            // 3D distance from eye
        double screenDist;          // 2D distance in screen space
        bool isCluster = false;     // Set if this is a cluster
        int clusterGroup = -1;
        SimpleIdentity clusterId = EmptyIdentity;
    };

    /// Add a rectangle (in 3-space) for selection
    void addSelectableRect(SimpleIdentity selectId,const Point3f *pts,bool enable);
    
    /// Add a rectangle (in 3-space) for selection, but only between the given visibilities
    void addSelectableRect(SimpleIdentity selectId,const Point3f *pts,
                           float minVis,float maxVis,bool enable);
    
    /// Add a screen space rectangle (2D) for selection, between the given visibilities
    void addSelectableScreenRect(SimpleIdentity selectId,const Point3d &center,
                                 const Point2f *pts,float minVis,float maxVis,bool enable);

    /// Add a screen space rectangle (2D) for selection, between the given visibilities, and it's moving
    void addSelectableMovingScreenRect(SimpleIdentity selectId,const Point3d &startCenter,
                                       const Point3d &endCenter,TimeInterval startTime,
                                       TimeInterval endTime,const Point2f *pts,
                                       float minVis,float maxVis,bool enable);
    
    /// Add a rectangular solid for selection.  Pass in 8 points (bottom four + top four)
    void addSelectableRectSolid(SimpleIdentity selectId,const Point3f *pts,
                                float minVis,float maxVis,bool enable);

    /// This version takes Point3d
    void addSelectableRectSolid(SimpleIdentity selectId,const Point3d *pts,
                                float minVis,float maxVis,bool enable);

    /// Add a rectangular solid for selection.  Pass in 8 points (bottom four + top four)
    void addSelectableRectSolid(SimpleIdentity selectId,const BBox &bbox,float minVis,float maxVis,bool enable);
    
    /// Add a polytope, represented by a set of surfaces
    void addPolytope(SimpleIdentity selectId,const std::vector<Point3dVector> &surfaces,
                     float minVis,float maxVis,bool enable);

    /// Add a polytope
    void addPolytopeFromBox(SimpleIdentity selectId,const Point3d &ll,const Point3d &ur,
                            const Eigen::Matrix4d &mat,float minVis,float maxVis,bool enable);
    
    /// Add a polytope that moves over time
    void addMovingPolytope(SimpleIdentity selectId,const std::vector<Point3dVector> &surfaces,
                           const Point3d &startCenter,const Point3d &endCenter,
                           TimeInterval startTime,TimeInterval duration,
                           const Eigen::Matrix4d &mat,float minVis,float maxVis,bool enable);
    
    /// Add a moving polytop from a box
    void addMovingPolytopeFromBox(SimpleIdentity selectID,const Point3d &ll,const Point3d &ur,
                                  const Point3d &startCenter,const Point3d &endCenter,
                                  TimeInterval startTime,TimeInterval duration,
                                  const Eigen::Matrix4d &mat,float minVis,float maxVis,bool enable);

    /// Add a linear in 3-space for selection.
    void addSelectableLinear(SimpleIdentity selectId,const Point3dVector &pts,
                             float minVis,float maxVis,bool enable);
    
    /// Add a billboard for selection.  Pass in the middle of the base and size
    void addSelectableBillboard(SimpleIdentity selectId,const Point3d &center,
                                const Point3d &norm,const Point2d &size,
                                float minVis,float maxVis,bool enable);
    
    /// Remove the given selectable from consideration
    void removeSelectable(SimpleIdentity selectId);
    
    /// Remove a set of selectables from consideration
    void removeSelectables(const SimpleIDSet &selectIDs);
    
    /// Enable/disable selectable
    void enableSelectable(SimpleIdentity selectID,bool enable);
    
    /// Enable/disable a set of selectables
    void enableSelectables(const SimpleIDSet &selectIDs,bool enable);
    
    /// Pass in the view point where the user touched.  This returns the closest hit within the given distance
    SimpleIdentity pickObject(const Point2f &touchPt,float maxDist,const ViewStateRef &viewState);
    
    /// Find all the objects within a given distance and return them, sorted by distance
    void pickObjects(const Point2f &touchPt,float maxDist,
                     const ViewStateRef &viewState,std::vector<SelectedObject> &selObjs);
    
    // Everything we need to project a world coordinate to one or more screen locations
    class PlacementInfo
    {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

        PlacementInfo(ViewStateRef viewState,SceneRenderer *renderer);
        
        ViewStateRef viewState;
        WhirlyGlobe::GlobeViewState *globeViewState;
        Maply::MapViewState *mapViewState;
        double heightAboveSurface;
        
        Point2f frameSize;
        Point2f frameSizeScale;

        /// frame with margin, not scaled
        Mbr frameMbr;
    };

protected:
    static Eigen::Matrix2d calcScreenRot(float &screenRot,
                                         const ViewStateRef &viewState,
                                         const WhirlyGlobe::GlobeViewState *globeViewState,
                                         const ScreenSpaceObjectLocation *ssObj,
                                         const Point2f &objPt,
                                         const Eigen::Matrix4d &modelTrans,
                                         const Eigen::Matrix4d &normalMat,
                                         const Point2f &frameBufferSize);

    // Projects a world coordinate to one or more points on the screen (wrapping)
    static void projectWorldPointToScreen(const Point3d &worldLoc,const PlacementInfo &pInfo,Point2dVector &screenPts,float scale);

    // Convert rect selectables into more generic screen space objects
    void getScreenSpaceObjects(const PlacementInfo &pInfo,std::vector<ScreenSpaceObjectLocation> &screenObjs,TimeInterval now);

    // Internal object picking method
    void pickObjects(const Point2f &touchPt,float maxDist,const ViewStateRef &viewState,
                     bool multi,std::vector<SelectedObject> &selObjs);

    Scene *scene;
    /// The selectable objects themselves
    WhirlyKit::RectSelectable3DSet rect3Dselectables;
    WhirlyKit::RectSelectable2DSet rect2Dselectables;
    WhirlyKit::MovingRectSelectable2DSet movingRect2Dselectables;
    WhirlyKit::PolytopeSelectableSet polytopeSelectables;
    WhirlyKit::MovingPolytopeSelectableSet movingPolytopeSelectables;
    WhirlyKit::LinearSelectableSet linearSelectables;
    WhirlyKit::BillboardSelectableSet billboardSelectables;
};
typedef std::shared_ptr<SelectionManager> SelectionManagerRef;
 
}
