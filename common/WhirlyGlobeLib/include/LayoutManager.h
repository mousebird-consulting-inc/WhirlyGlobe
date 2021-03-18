/*  LayoutManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/15/13.
 *  Copyright 2011-2021 mousebird consulting.
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
#import "BasicDrawable.h"
#import "Scene.h"
#import "SceneRenderer.h"
#import "ScreenSpaceBuilder.h"
#import "SelectionManager.h"
#import "OverlapHelper.h"
#import "VectorManager.h"

namespace WhirlyKit
{

/// Don't modify it at all
#define WhirlyKitLayoutPlacementNone (1<<0)
/// Okay to center
#define WhirlyKitLayoutPlacementCenter (1U<<1U)
/// Okay to place to the right of a point
#define WhirlyKitLayoutPlacementRight  (1U<<2U)
/// Okay to place it to the left of a point
#define WhirlyKitLayoutPlacementLeft   (1U<<3U)
/// Okay to place on top of a point
#define WhirlyKitLayoutPlacementAbove  (1U<<4U)
/// Okay to place below a point
#define WhirlyKitLayoutPlacementBelow  (1U<<5U)

/** This represents an object in the screen space generator to be laid out
 by the layout engine.  We'll manipulate its offset and enable/disable it
 but won't otherwise change it.
 */
class LayoutObject : public ScreenSpaceObject
{
public:
    LayoutObject();
    LayoutObject(SimpleIdentity theId);
    
    // Set the layout size from width/height
    void setLayoutSize(const Point2d &layoutSize,const Point2d &offset);
    
    // Set the selection size from width/height
    void setSelectSize(const Point2d &layoutSize,const Point2d &offset);

    // Size to use for laying out
    Point2dVector layoutPts;
    
    // Size to use for selection
    Point2dVector selectPts;

    std::string uniqueID;
        
    /// This is used to sort objects for layout.  Bigger is more important.
    float importance;
    /// If set, this is clustering group to sort into
    int clusterGroup;

    int layoutRepeat;      // How many instances
    float layoutOffset;   // Offset left/right
    float layoutSpacing;  // Start/end spacing along line
    float layoutWidth;   // Used in generalization
    bool layoutDebug;    // Turn this on for layout debugging
    Point3dVector layoutShape;
    /// If we're placing glyphs individually we'll do it with matrices
    std::vector<std::vector<Eigen::Matrix3d> > layoutPlaces;
    std::vector<Point3d> layoutModelPlaces;

    /// Options for where to place this object:  WhirlyKitLayoutPlacementLeft, WhirlyKitLayoutPlacementRight,
    ///  WhirlyKitLayoutPlacementAbove, WhirlyKitLayoutPlacementBelow
    unsigned acceptablePlacement;
    /// Debugging hint
    std::string hint;
};

// Private fields we use for object layout
class LayoutObjectEntry : public Identifiable
{
public:
    LayoutObjectEntry(SimpleIdentity theId);
    
    // The layout objects as passed in by the original caller
    LayoutObject obj;
    
    // Set if it's currently on
    bool currentEnable;
    // Set if it's going to be on
    bool newEnable;

    // Set if the object is part of an existing cluster
    int currentCluster;
    // Set if the object is going into a new cluster
    int newCluster;

    // The offset, as calculated
    WhirlyKit::Point2d offset;
    // Set if we changed something during evaluation
    bool changed;
};

typedef std::set<LayoutObjectEntry *,IdentifiableSorter> LayoutEntrySet;

/**  The cluster generator is a callback used to make the images (or whatever)
     for a group of objects.
  */
class ClusterGenerator
{
public:
    virtual ~ClusterGenerator() = default;
    
    // Called right before we start generating layout objects
    virtual void startLayoutObjects(PlatformThreadInfo *) = 0;
    
    // Generate a layout object (with screen space object and such) for the cluster
    virtual void makeLayoutObject(
            PlatformThreadInfo *,
            int clusterID,
            const std::vector<LayoutObjectEntry *> &layoutObjects,
            LayoutObject &newObj) = 0;

    // Called right after all the layout objects are generated
    virtual void endLayoutObjects(PlatformThreadInfo *) = 0;
    
    // Parameters for a particular cluster class needed to make animations and such
    struct ClusterClassParams
    {
        SimpleIdentity motionShaderID;
        bool selectable;
        double markerAnimationTime;
        Point2d clusterSize;
    };
    
    // Return the shader used when moving objects into and out of clusters
    virtual void paramsForClusterClass(PlatformThreadInfo *,int clusterID,ClusterClassParams &clusterParams) = 0;
};
    
#define kWKLayoutManager "WKLayoutManager"
 
/** A bookkeeping entry for a single cluster to track its location.
  */
class ClusterEntry
{
public:
    // The layout object for the cluster itself
    LayoutObject layoutObj;
    // Object IDs for all the objects clustered together
    std::vector<SimpleIdentity> objectIDs;
    // If set, the cluster is a child of this older one
    int childOfCluster;
    // Pointer into cluster parameters
    int clusterParamID;
};
    
// Sort more important things to the front
typedef struct
{
    bool operator () (const LayoutObjectEntry *a,const LayoutObjectEntry *b) const
    {
        if (a->obj.importance == b->obj.importance)
            return a > b;
        return a->obj.importance > b->obj.importance;
    }
} LayoutEntrySorter;
typedef std::set<LayoutObjectEntry *,LayoutEntrySorter> LayoutSortingSet;

/** The layout manager handles 2D text and marker layout.  We feed it objects
    we want to be drawn and it will figure out which ones should be visible
    and which shouldn't.
 
    This manager is entirely thread safe except for destruction.
  */
class LayoutManager : public SceneManager
{
public:
    LayoutManager();
    virtual ~LayoutManager();
    
    /// If set, the maximum number of objects to display
    void setMaxDisplayObjects(int numObjects);
    
    /// Mark the UUIDs that we'll force to always display
    void setOverrideUUIDs(const std::set<std::string> &uuids);
    
    /// Add objects for layout (thread safe)
    void addLayoutObjects(const std::vector<LayoutObject> &newObjects);

    /// Add objects for layout (thread safe)
    void addLayoutObjects(const std::vector<LayoutObject *> &newObjects);

    /// Remove objects for layout (thread safe)
    void removeLayoutObjects(const SimpleIDSet &oldObjects);
    
    /// Enable/disable layout objects
    void enableLayoutObjects(const SimpleIDSet &layoutObjects,bool enable);
    
    /// Run the layout logic for everything we're aware of (thread safe)
    void updateLayout(PlatformThreadInfo *threadInfo,const ViewStateRef &viewState,ChangeSet &changes);
    
    /// True if we've got changes since the last update
    bool hasChanges();
    
    /// Return the active objects in a form the selection manager can handle
    void getScreenSpaceObjects(const SelectionManager::PlacementInfo &pInfo,std::vector<ScreenSpaceObjectLocation> &screenSpaceObjs);
    
    /// Add a generator for cluster images
    void addClusterGenerator(PlatformThreadInfo *,ClusterGenerator *clusterGen);
    
protected:
    static bool calcScreenPt(Point2f &objPt,LayoutObject *layoutObj,const ViewStateRef &viewState,const Mbr &screenMbr,const Point2f &frameBufferSize);
    static Eigen::Matrix2d calcScreenRot(float &screenRot,const ViewStateRef &viewState,WhirlyGlobe::GlobeViewState *globeViewState,ScreenSpaceObject *ssObj,const Point2f &objPt,const Eigen::Matrix4d &modelTrans,const Eigen::Matrix4d &normalMat,const Point2f &frameBufferSize);

    bool runLayoutRules(PlatformThreadInfo *threadInfo,
                        const ViewStateRef &viewState,
                        std::vector<ClusterEntry> &clusterEntries,
                        std::vector<ClusterGenerator::ClusterClassParams> &outClusterParams,
                        ChangeSet &changes);
    
    VectorManagerRef vecManage;
    
    /// If non-zero the maximum number of objects we'll display at once
    int maxDisplayObjects;
    /// If there were updates since the last layout
    bool hasUpdates;
    /// Objects we're controlling the placement for
    LayoutEntrySet layoutObjects;
    /// Drawables created on the last round
    SimpleIDSet drawIDs;
    /// Clusters on the current round
    std::vector<ClusterEntry> clusters;
    /// Display parameter for the clusters
    std::vector<ClusterGenerator::ClusterClassParams> clusterParams;
    /// Cluster generators
    ClusterGenerator *clusterGen;
    /// Features we'll force to always display
    std::set<std::string> overrideUUIDs;
    
    SimpleIDSet debugVecIDs;  // Used to display debug lines for text layout
    SimpleIdentity vecProgID;
};
typedef std::shared_ptr<LayoutManager> LayoutManagerRef;

}
