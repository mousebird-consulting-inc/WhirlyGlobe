/*  LayoutManager.cpp
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

#import "LayoutManager.h"
#import "WhirlyGeometry.h"
#import "GlobeMath.h"
#import "SharedAttributes.h"
#import "LinearTextBuilder.h"
#import "WhirlyKitLog.h"

using namespace Eigen;

namespace WhirlyKit
{

// Default constructor for layout object
LayoutObject::LayoutObject()
    : ScreenSpaceObject(), layoutRepeat(0), layoutOffset(0.0), layoutSpacing(20.0), layoutWidth(10.0), layoutDebug(false),
    importance(MAXFLOAT), clusterGroup(-1), acceptablePlacement(WhirlyKitLayoutPlacementLeft | WhirlyKitLayoutPlacementRight | WhirlyKitLayoutPlacementAbove | WhirlyKitLayoutPlacementBelow)
{
}
    
LayoutObject::LayoutObject(SimpleIdentity theId) : ScreenSpaceObject(theId),
    layoutRepeat(0), layoutOffset(0.0), layoutSpacing(20.0), layoutWidth(10.0), layoutDebug(false),
     importance(MAXFLOAT), clusterGroup(-1), acceptablePlacement(WhirlyKitLayoutPlacementLeft | WhirlyKitLayoutPlacementRight | WhirlyKitLayoutPlacementAbove | WhirlyKitLayoutPlacementBelow)
{
}
    
void LayoutObject::setLayoutSize(const Point2d &layoutSize,const Point2d &offset)
{
    if (layoutSize.x() == 0.0 && layoutSize.y() == 0.0)
        return;
    
    layoutPts.push_back(Point2d(0,0)+offset);
    layoutPts.push_back(Point2d(layoutSize.x(),0.0)+offset);
    layoutPts.push_back(layoutSize+offset);
    layoutPts.push_back(Point2d(0.0,layoutSize.y())+offset);
}

void LayoutObject::setSelectSize(const Point2d &selectSize,const Point2d &offset)
{
    if (selectSize.x() == 0.0 && selectSize.y() == 0.0)
        return;
    
    selectPts.push_back(Point2d(0,0)+offset);
    selectPts.push_back(Point2d(selectSize.x(),0.0)+offset);
    selectPts.push_back(selectSize+offset);
    selectPts.push_back(Point2d(0.0,selectSize.y())+offset);
}
    
LayoutObjectEntry::LayoutObjectEntry(SimpleIdentity theId)
: Identifiable(theId)
{
    currentEnable = newEnable = false;
    currentCluster = newCluster = -1;
    offset = Point2d(MAXFLOAT,MAXFLOAT);
    changed = true;
}
    
LayoutManager::LayoutManager() :
    maxDisplayObjects(0),
    hasUpdates(false),
    showDebugBoundaries(false),
    clusterGen(nullptr),
    vecProgID(EmptyIdentity)
{
}
    
LayoutManager::~LayoutManager()
{
    std::lock_guard<std::mutex> guardLock(lock);

    for (auto layoutObject : layoutObjects)
    {
        delete layoutObject;
    }
    layoutObjects.clear();
}
    
void LayoutManager::setMaxDisplayObjects(int numObjects)
{
    std::lock_guard<std::mutex> guardLock(lock);

    maxDisplayObjects = numObjects;
}

void LayoutManager::setOverrideUUIDs(const std::set<std::string> &uuids)
{
    std::lock_guard<std::mutex> guardLock(lock);

    overrideUUIDs = uuids;
}
    
void LayoutManager::addLayoutObjects(const std::vector<LayoutObject> &newObjects)
{
    std::lock_guard<std::mutex> guardLock(lock);

    for (const auto & newObject : newObjects)
    {
        const LayoutObject &layoutObj = newObject;
        auto *entry = new LayoutObjectEntry(layoutObj.getId());
        entry->obj = newObject;
        layoutObjects.insert(entry);
    }
    hasUpdates = true;
}

void LayoutManager::addLayoutObjects(const std::vector<LayoutObject *> &newObjects)
{
    std::lock_guard<std::mutex> guardLock(lock);

    for (auto newObject : newObjects)
    {
        const LayoutObject *layoutObj = newObject;
        auto *entry = new LayoutObjectEntry(layoutObj->getId());
        entry->obj = *newObject;
        layoutObjects.insert(entry);
    }
    hasUpdates = true;
}

/// Enable/disable layout objects
void LayoutManager::enableLayoutObjects(const SimpleIDSet &theObjects,bool enable)
{
    std::lock_guard<std::mutex> guardLock(lock);

    for (const auto &theObject : theObjects)
    {
        LayoutObjectEntry key(theObject);
        const auto eit = layoutObjects.find(&key);
        if (eit != layoutObjects.end())
        {
            LayoutObjectEntry *entry = *eit;
            if (!enable)
            {
                entry->newCluster = -1;
                entry->currentCluster = -1;
            }
            entry->obj.enable = enable;
        }
    }
    hasUpdates = true;    
}
    
void LayoutManager::removeLayoutObjects(const SimpleIDSet &oldObjects)
{
    std::lock_guard<std::mutex> guardLock(lock);

    for (const auto &oldObject : oldObjects)
    {
        LayoutObjectEntry entry(oldObject);
        const auto eit = layoutObjects.find(&entry);
        if (eit != layoutObjects.end())
        {
            delete *eit;
            layoutObjects.erase(eit);
        }
    }
    hasUpdates = true;
}
    
bool LayoutManager::hasChanges()
{
    std::lock_guard<std::mutex> guardLock(lock);
    if (clusterGen->hasChanges()) {
        hasUpdates = true;
    }
    return hasUpdates;
}

// Return the screen space objects in a form the selection manager can understand
void LayoutManager::getScreenSpaceObjects(const SelectionManager::PlacementInfo &pInfo,
                                          std::vector<ScreenSpaceObjectLocation> &screenSpaceObjs)
{
    std::lock_guard<std::mutex> guardLock(lock);

    // First the regular screen space objects
    for (const auto &entry : layoutObjects)
    {
        if (entry->currentEnable && entry->obj.enable)
        {
            ScreenSpaceObjectLocation ssObj;
            ssObj.shapeIDs.push_back(entry->obj.getId());
            ssObj.dispLoc = entry->obj.worldLoc;
            ssObj.rotation = entry->obj.rotation;
            ssObj.keepUpright = entry->obj.keepUpright;
            ssObj.offset = entry->offset;
            ssObj.pts = entry->obj.selectPts;
            ssObj.mbr.addPoints(entry->obj.selectPts);

            screenSpaceObjs.push_back(ssObj);
        }
    }
    
    // Then the clusters
    for (const auto &cluster : clusters)
    {
        ScreenSpaceObjectLocation ssObj;
        ssObj.shapeIDs = cluster.objectIDs;
        ssObj.dispLoc = cluster.layoutObj.worldLoc;
        ssObj.offset = cluster.layoutObj.offset;
        ssObj.pts = cluster.layoutObj.selectPts;
        ssObj.mbr.addPoints(cluster.layoutObj.selectPts);
        ssObj.isCluster = true;

        screenSpaceObjs.push_back(ssObj);
    }
}

void LayoutManager::addClusterGenerator(PlatformThreadInfo *, ClusterGenerator *inClusterGen)
{
    std::lock_guard<std::mutex> guardLock(lock);
    clusterGen = inClusterGen;
    hasUpdates = true;
}

// Collection of objects we'll cluster together
class ClusteredObjects
{
public:
    ClusteredObjects() = default;
    explicit ClusteredObjects(int clusterID) : clusterID(clusterID) { }
    
    int clusterID{};
    
    LayoutSortingSet layoutObjects;
};
    
struct ClusteredObjectsSorter
{
    // Comparison operator
    bool operator () (const ClusteredObjects *lhs,const ClusteredObjects *rhs) const
    {
        return lhs->clusterID < rhs->clusterID;
    }
};
    
typedef std::set<ClusteredObjects *,ClusteredObjectsSorter> ClusteredObjectsSet;

// Size of the overlap sampler
static const int OverlapSampleX = 10;
static const int OverlapSampleY = 60;

// Now much around the screen we'll take into account
static const float ScreenBuffer = 0.1;
    
bool LayoutManager::calcScreenPt(Point2f &objPt,const LayoutObject *layoutObj,
                                 const ViewStateRef &viewState,
                                 const Mbr &screenMbr,const Point2f &frameBufferSize)
{
    // Figure out where this will land
    bool isInside = false;
    for (unsigned int offi=0;offi<viewState->viewMatrices.size();offi++)
    {
        Eigen::Matrix4d modelTrans = viewState->fullMatrices[offi];
        Point2f thisObjPt = viewState->pointOnScreenFromDisplay(layoutObj->worldLoc,&modelTrans,frameBufferSize);
        if (screenMbr.inside(Point2f(thisObjPt.x(),thisObjPt.y())))
        {
            isInside = true;
            objPt = thisObjPt;
        }
    }
    
    return isInside;
}

Matrix2d LayoutManager::calcScreenRot(float &screenRot,const ViewStateRef &viewState,
                                      const WhirlyGlobe::GlobeViewState *globeViewState,
                                      const ScreenSpaceObject *ssObj,const Point2f &objPt,
                                      const Matrix4d &modelTrans,const Matrix4d &normalMat,
                                      const Point2f &frameBufferSize)
{
    // Switch from counter-clockwise to clockwise
    double rot = 2*M_PI-ssObj->rotation;
    
    Point3d upVec,northVec,eastVec;
    if (!globeViewState)
    {
        upVec = Point3d(0,0,1);
        northVec = Point3d(0,1,0);
        eastVec = Point3d(1,0,0);
    } else {
        Point3d worldLoc = ssObj->getWorldLoc();
        upVec = worldLoc.normalized();
        // Vector pointing north
        northVec = Point3d(-worldLoc.x(),-worldLoc.y(),1.0-worldLoc.z());
        eastVec = northVec.cross(upVec);
        northVec = upVec.cross(eastVec);
    }
    
    // This vector represents the rotation in world space
    Point3d rotVec = eastVec * sin(rot) + northVec * cos(rot);
    
    // Project down into screen space
    Vector4d projRot = normalMat * Vector4d(rotVec.x(),rotVec.y(),rotVec.z(),0.0);
    
    // Use the resulting x & y
    screenRot = (float)(atan2(projRot.y(),projRot.x())-M_PI/2.0);
    // Keep the labels upright
    if (ssObj->keepUpright && screenRot > M_PI/2 && screenRot < 3*M_PI/2)
    {
        screenRot = (float)(screenRot + M_PI);
    }
    Matrix2d screenRotMat;
    screenRotMat = Eigen::Rotation2Dd(screenRot);

    return screenRotMat;
}

// Used for sorting layout objects
class LayoutObjectContainer
{
public:
    LayoutObjectContainer() : importance(-1.0) { }
    explicit LayoutObjectContainer(LayoutObjectEntry *entry) {
        objs.push_back(entry);
        importance = objs[0]->obj.importance;
    }
    
    // Objects that share the same unique ID
    std::vector<LayoutObjectEntry *> objs;
    
    bool operator < (const LayoutObjectContainer &that) const {
        if (objs.empty())  // Never happen
            return false;
        return importance > that.importance;
    }
    
    float importance;
};
typedef std::vector<LayoutObjectContainer> LayoutContainerVec;
    
typedef std::map<std::string,LayoutObjectContainer> UniqueLayoutObjectMap;

void LayoutManager::addDebugOutput(const Point2dVector &pts,
                                   WhirlyGlobe::GlobeViewState *globeViewState,
                                   Maply::MapViewState *mapViewState,
                                   const Point2f &frameBufferSize,
                                   ChangeSet &changes,
                                   int priority,
                                   RGBAColor color)
{
    auto coordAdapt = globeViewState ? globeViewState->coordAdapter : mapViewState->coordAdapter;
    auto coordSys = coordAdapt->getCoordSystem();
    VectorLinearRef lin = VectorLinear::createLinear();

    for (unsigned oi=0;oi<pts.size()+1;oi++) {
        const Point2d &pt = pts[oi%pts.size()];
        if (globeViewState) {
            Point3d modelPt;
            if (globeViewState->pointOnSphereFromScreen(Point2f(pt.x(),pt.y()), globeViewState->fullMatrices[0], frameBufferSize, modelPt, false)) {
                GeoCoord geoPt = coordSys->localToGeographic(coordAdapt->displayToLocal(modelPt));
                lin->pts.emplace_back(geoPt.x(),geoPt.y());
            }
        } else {
            Point3d modelPt;
            if (mapViewState->pointOnPlaneFromScreen(Point2f(pt.x(),pt.y()), mapViewState->fullMatrices[0], frameBufferSize, modelPt, false)) {
                GeoCoord geoPt = coordSys->localToGeographic(coordAdapt->displayToLocal(modelPt));
                lin->pts.emplace_back(geoPt.x(),geoPt.y());
            }
        }
    }
    
    // Turn them back into vectors to debug
    VectorInfo vecInfo;
    vecInfo.color = color;
    vecInfo.drawPriority = priority;
    vecInfo.programID = vecProgID;

    ShapeSet dispShapes { lin };
    SimpleIdentity vecId = vecManage->addVectors(&dispShapes, vecInfo, changes);
    if (vecId != EmptyIdentity)
        debugVecIDs.insert(vecId);
}

static Point2d offsetForOrientation(int orient, const Point2d &span)
{
    // Set up the offset for this orientation
    switch (orient)
    {
        default:
        case 0: return {           0,            0 }; // Don't move at all
        case 1: return { -span.x()/2,  -span.y()/2 }; // Center
        case 2: return {         0.0,  -span.y()/2 }; // Right
        case 3: return {   -span.x(),  -span.y()/2 }; // Left
        case 4: return { -span.x()/2,          0.0 }; // Above
        case 5: return { -span.x()/2,    -span.y() }; // Below
    }
}

// Do the actual layout logic.  We'll modify the offset and on value in place.
bool LayoutManager::runLayoutRules(PlatformThreadInfo *threadInfo,
                                   const ViewStateRef &viewState,
                                   std::vector<ClusterEntry> &clusterEntries,
                                   std::vector<ClusterGenerator::ClusterClassParams> &outClusterParams,
                                   ChangeSet &changes)
{
    if (layoutObjects.empty())
        return false;
    
    bool hadChanges = false;
        
    ClusteredObjectsSet clusterObjs;
    LayoutContainerVec layoutObjs;
    // Special snowflake layout objects (with unique names)
    UniqueLayoutObjectMap uniqueLayoutObjs;
    
    // The globe has some special requirements
    auto globeViewState = dynamic_cast<WhirlyGlobe::GlobeViewState *>(viewState.get());
    auto mapViewState = dynamic_cast<Maply::MapViewState *>(viewState.get());

    // View related matrix stuff
    Matrix4d modelTrans = viewState->fullMatrices[0];
    Matrix4d fullMatrix = viewState->fullMatrices[0];
    Matrix4d fullNormalMatrix = viewState->fullNormalMatrices[0];
    Matrix4d normalMat = viewState->fullMatrices[0].inverse().transpose();
    
    // Turn everything off and sort by importance
    for (const auto &layoutObject : layoutObjects)
    {
        LayoutObjectEntry *layoutObj = layoutObject;
        if (layoutObj->obj.enable)
        {
            LayoutObjectEntry *obj = layoutObject;
            bool use = false;
            if (globeViewState)
            {
                if (obj->obj.state.minVis == DrawVisibleInvalid || obj->obj.state.maxVis == DrawVisibleInvalid ||
                    (obj->obj.state.minVis < globeViewState->heightAboveGlobe && globeViewState->heightAboveGlobe < obj->obj.state.maxVis))
                    use = true;
            } else {
                if (obj->obj.state.minVis == DrawVisibleInvalid || obj->obj.state.maxVis == DrawVisibleInvalid ||
                    (obj->obj.state.minVis < mapViewState->heightAboveSurface && mapViewState->heightAboveSurface < obj->obj.state.maxVis))
                    use = true;
            }
            if (use) {
                // Make sure this one isn't behind the globe
                if (globeViewState)
                {
                    // Layout shape following doesn't work with this check
                    if (obj->obj.layoutShape.empty()) {
                        // Make sure this one is facing toward the viewer
                        use = CheckPointAndNormFacing(layoutObj->obj.worldLoc,layoutObj->obj.worldLoc.normalized(),fullMatrix,fullNormalMatrix) > 0.0;
                    }
                }

                if (use)
                {
                    obj->newCluster = -1;
                    if (obj->obj.clusterGroup > -1)
                    {
                        // Put the entry in the right cluster
                        ClusteredObjects findClusterObj(obj->obj.clusterGroup);
                        ClusteredObjects *thisClusterObj = nullptr;
                        auto cit = clusterObjs.find(&findClusterObj);
                        if (cit == clusterObjs.end())
                        {
                            // Create a new cluster object
                            thisClusterObj = new ClusteredObjects(obj->obj.clusterGroup);
                            clusterObjs.insert(thisClusterObj);

                            hadChanges = true;
                        } else
                            thisClusterObj = *cit;
                        
                        thisClusterObj->layoutObjects.insert(layoutObj);
                        
                        obj->newEnable = false;
                        obj->newCluster = -1;
                    } else {
                        // Not a cluster
                        if (layoutObj->obj.uniqueID.empty())
                            layoutObjs.push_back(LayoutObjectContainer(layoutObj));
                        else {
                            // Add it to a container for its unique name
                            auto it = uniqueLayoutObjs.find(layoutObj->obj.uniqueID);
                            LayoutObjectContainer dest;
                            if (it != uniqueLayoutObjs.end())
                                dest = it->second;
                            // See if we're overriding this importance
                            dest.importance = layoutObj->obj.importance;
                            if (!layoutObj->obj.uniqueID.empty() && overrideUUIDs.find(layoutObj->obj.uniqueID) != overrideUUIDs.end())
                                dest.importance = MAXFLOAT;
                            dest.objs.push_back(layoutObj);
                            uniqueLayoutObjs[layoutObj->obj.uniqueID] = dest;
                        }
                    }
                } else {
                    obj->newEnable = false;
                    obj->newCluster = -1;
                }
            } else {
                obj->newEnable = false;
                obj->newCluster = -1;
            }
            // Note: Update this for clusters
            if ((use && !obj->currentEnable) || (!use && obj->currentEnable))
                hadChanges = true;
        }
    }
    
    // Extents for the layout helpers
    const Point2f frameBufferSize(renderer->framebufferWidth, renderer->framebufferHeight);
    const Mbr screenMbr(frameBufferSize * -ScreenBuffer,
                        frameBufferSize * (1.0 + ScreenBuffer));

    // Need to scale for retina displays
    const float resScale = renderer->getScale();

    if (clusterGen)
    {
        clusterGen->startLayoutObjects(threadInfo);
        
        // Lay out the clusters in order
        for (const auto &cluster : clusterObjs)
        {
            outClusterParams.resize(outClusterParams.size() + 1);
            ClusterGenerator::ClusterClassParams &params = outClusterParams.back();
            clusterGen->paramsForClusterClass(threadInfo,cluster->clusterID,params);

            ClusterHelper clusterHelper(screenMbr,OverlapSampleX,OverlapSampleY,resScale,params.clusterSize);
            
            // Add all the various objects to the cluster and figure out overlaps
            for (const auto &entry : cluster->layoutObjects)
            {
                // Project the point and figure out the rotation
                bool isActive = true;
                Point2f objPt;
                bool isInside = calcScreenPt(objPt,&entry->obj,viewState,screenMbr,frameBufferSize);
                
                isActive &= isInside;
                
                if (isActive)
                {
                    // Deal with the rotation
                    float screenRot = 0.0;
                    Matrix2d screenRotMat;
                    if (entry->obj.rotation != 0.0)
                        screenRotMat = calcScreenRot(screenRot,viewState,globeViewState,&entry->obj,objPt,modelTrans,normalMat,frameBufferSize);
                    
                    // Rotate the rectangle
                    Point2dVector objPts(4);
                    if (screenRot == 0.0)
                    {
                        for (unsigned int ii=0;ii<4;ii++)
                            objPts[ii] = Point2d(objPt.x(),objPt.y()) + entry->obj.layoutPts[ii] * resScale;
                    } else {
                        Point2d center(objPt.x(),objPt.y());
                        for (unsigned int ii=0;ii<4;ii++)
                        {
                            const Point2d &thisObjPt = entry->obj.layoutPts[ii];
                            const Point2d offPt = screenRotMat * (thisObjPt * resScale);
                            objPts[ii] = Point2d(offPt.x(),-offPt.y()) + center;
                        }
                    }

                    clusterHelper.addObject(entry,objPts);
                }
            }
            
            // Deal with the clusters and their own overlaps
            clusterHelper.resolveClusters();

            // Toss the unaffected layout objects into the mix
            for (auto &obj : clusterHelper.simpleObjects)
                if (obj.parentObject < 0)
                {
                    layoutObjs.emplace_back(obj.objEntry);
                    obj.objEntry->newEnable = true;
                    obj.objEntry->newCluster = -1;
                }
            
            // Create new objects for the clusters
            for (auto clusterObj : clusterHelper.clusterObjects)
            {
                std::vector<LayoutObjectEntry *> objsForCluster;
                clusterHelper.objectsForCluster(clusterObj,objsForCluster);
                
                if (!objsForCluster.empty())
                {
                    int clusterEntryID = (int)clusterEntries.size();
                    clusterEntries.resize(clusterEntryID+1);
                    ClusterEntry &clusterEntry = clusterEntries[clusterEntryID];

                    const Point2f clusterLoc = Point2f(clusterObj.center.x(),clusterObj.center.y());

                    // Project the cluster back into a geolocation so we can place it.
                    Point3d dispPt;
                    bool dispPtValid = false;
                    if (globeViewState)
                    {
                        dispPtValid = globeViewState->pointOnSphereFromScreen(clusterLoc,modelTrans,frameBufferSize,dispPt);
                    } else {
                        dispPtValid = mapViewState->pointOnPlaneFromScreen(clusterLoc,modelTrans,frameBufferSize,dispPt,false);
                    }

                    // Note: What happens if the display point isn't valid?
                    if (dispPtValid)
                    {
                        clusterEntry.layoutObj.worldLoc = dispPt;
                        for (auto thisObj : objsForCluster)
                            clusterEntry.objectIDs.push_back(thisObj->obj.getId());
                        clusterGen->makeLayoutObject(threadInfo,cluster->clusterID, objsForCluster, clusterEntry.layoutObj);
                        if (!params.selectable)
                            clusterEntry.layoutObj.selectPts.clear();
                    }
                    clusterEntry.clusterParamID = (int)(outClusterParams.size() - 1);

                    // Figure out if all the objects in this new cluster come from the same old cluster
                    //  and assign the new cluster ID
                    int whichOldCluster = -1;
                    for (auto obj : objsForCluster)
                    {
                        if (obj->currentCluster > -1 && whichOldCluster != -2)
                        {
                            if (whichOldCluster == -1)
                                whichOldCluster = obj->currentCluster;
                            else {
                                if (whichOldCluster != obj->currentCluster)
                                    whichOldCluster = -2;
                            }
                        }
                        obj->newCluster = clusterEntryID;
                    }
                    
                    // If the children all agree about the old cluster, let's reflect that
                    clusterEntry.childOfCluster = (whichOldCluster == -2) ? -1 : whichOldCluster;
                }
            }
        }
        
        // Tear down the clusters
        for (auto clusterObj : clusterObjs)
        {
            delete clusterObj;
        }
        clusterObjs.clear();
        
        clusterGen->endLayoutObjects(threadInfo);
    }
    
//    NSLog(@"----Starting Layout----");
    
    // Set up the overlap sampler
    OverlapHelper overlapMan(screenMbr,OverlapSampleX,OverlapSampleY);
    
    // Add in the unique objects, cluster entries and then sort them all
    for (auto &it : uniqueLayoutObjs) {
        layoutObjs.push_back(it.second);
    }
    std::sort(layoutObjs.begin(),layoutObjs.end());
    
    // Clusters have priority in the overlap.
    for (const auto &it : clusterEntries) {
        Point2f objPt = {0,0};
        /*const bool isInside = */calcScreenPt(objPt,&it.layoutObj,viewState,screenMbr,frameBufferSize);
        auto objPts = it.layoutObj.layoutPts;   // make a copy
        for (auto &pt : objPts)
            pt = pt * resScale + Point2d(objPt.x(),objPt.y());
        overlapMan.addObject(objPts);
    }

    // Lay out the various objects that are active
    int numSoFar = 0;
    for (auto container : layoutObjs)
    {
        bool isActive;
        Point2d objOffset(0.0,0.0);
        Point2dVector objPts(4);
        
        // Start with a max objects check
        isActive = true;
        if (maxDisplayObjects != 0 && (numSoFar >= maxDisplayObjects))
            isActive = false;
        
        // Sort the objects by importance within their container
        std::sort(container.objs.begin(),container.objs.end(),
                  [](const LayoutObjectEntry *a,LayoutObjectEntry *b) -> bool
                  {
                      return a->obj.importance > b->obj.importance;
                  });

        // Some of these may share unique IDs
        bool pickedOne = false;
//        wkLog("----");
        
        for (auto layoutObj : container.objs) {
            layoutObj->newEnable = false;
            layoutObj->obj.layoutModelPlaces.clear();
            layoutObj->obj.layoutPlaces.clear();

            // Layout along a shape
            if (!layoutObj->obj.layoutShape.empty()) {
                // Sometimes there are just a few instances
                int numInstances = 0;
                
                for (unsigned int oi=0;oi<viewState->viewMatrices.size();oi++) {
                    // Set up the text builder to get a set of individual runs to follow
                    LinearTextBuilder textBuilder(viewState,oi,frameBufferSize,
                                                  layoutObj->obj.layoutWidth*1.5f,
                                                  &layoutObj->obj);
                    textBuilder.setPoints(layoutObj->obj.layoutShape);
                    textBuilder.process();
                    // Sort the runs by length and get rid of the ones too short
//                    textBuilder.sortRuns(2.0*layoutObj->obj.layoutSpacing);

                    // Follow the individual runs
                    std::vector<std::vector<Eigen::Matrix3d> > layoutInstances;
                    std::vector<Point3d> layoutModelInstances;

                    auto runs = textBuilder.getScreenVecs();
//                    unsigned int ri=0;
                    for (const auto& run: runs) {
//                        wkLog("Run %d: %d points",ri++,run.size());
                        
                        // We need the length of the glyphs and their center
                        Mbr layoutMbr(layoutObj->obj.layoutPts);
                        float textLen = layoutMbr.ur().x();
                        float midY = layoutMbr.mid().y();

                        LinearWalker walk(run);

                        // Figure out how many times we could lay this out
                        float textRoom = walk.getTotalLength() - 2.0f*layoutObj->obj.layoutSpacing;
                        float textInstance = textRoom / textLen;
                        
                        for (unsigned int ini=0;ini<textInstance;ini++) {
//                            wkLog(" Text Instance %d",ini);
                                                        
                            // Start with an initial offset
                            if (!walk.nextPoint(layoutObj->obj.layoutSpacing, nullptr, nullptr, true))
                                continue;

                            // Check the normal right in the middle
                            Point2f normAtMid;
                            if (!walk.nextPoint(textLen/2.0, nullptr, &normAtMid, false))
                                continue;

                            std::vector<Eigen::Matrix3d> layoutMats;

                            // Center around the world point on the screen
                            Point2f midRun;
                            if (!walk.nextPoint(resScale * layoutMbr.span().x()/2.0, &midRun, nullptr, false))
                                continue;
//                            wkLogLevel(Info, "midRun = (%f,%f)",midRun.x(),midRun.y());
                            Point2f worldScreenPt = midRun;
                            Point3d worldPt(0.0,0.0,0.0);
                            if (!textBuilder.screenToWorld(midRun, worldPt))
                                continue;
                            
                            std::vector<Point2dVector> overlapPts;

                            // Walk through the individual glyphs
                            bool failed = false;
                            int gStart = 0, gEnd = (int)layoutObj->obj.geometry.size()-1, gIncr = 1;
                            bool flipped = false;
                            // If it's upside down, then run it backwards
                            if (normAtMid.y() < 0.0) {
                                flipped = true;
                                gStart = gEnd;  gEnd = 0;  gIncr = -1;
                            }

                            Point2f lastNorm;
                            bool lastNormValid = false;
                            for (int ig=gStart;gIncr > 0 ? ig<=gEnd : ig>=gEnd;ig+=gIncr) {
                                const auto &geom = layoutObj->obj.geometry[ig];
                                const Mbr glyphMbr(geom.coords);
                                const Point2f span = glyphMbr.span();
                                const Point2f midGlyph = glyphMbr.mid();
                                const Affine2d transOrigin(Translation2d(-midGlyph.x(),flipped ? -midY/2.0 : -1.5*midY));

                                // Walk along the line to get a good center
                                Point2f centerPt;
                                Point2f norm;
                                if (!walk.nextPoint(resScale * span.x()/2.0,&centerPt,&norm,true)) {
                                    failed = true;
                                    break;
                                }

                                // If we're too far from the last normal, bail.  The text will look jumbled.
                                double normAng = 0.0;
                                if (lastNormValid) {
                                    // Nifty trick to get a clockwise angle between the two
                                    const double dot = norm.x()*lastNorm.x() + norm.y()*lastNorm.y();
                                    const double det = norm.x()*lastNorm.y() - norm.y()*lastNorm.x();
                                    normAng = atan2(det, dot);
                                    //wkLogLevel(Debug,"normAng = %f",normAng);
                                    if (normAng != 0.0 && abs(normAng) > 45.0 * M_PI / 180.0) {
                                        failed = true;
                                        break;
                                    }
                                }
                                
                                // And let's nudge it over a bit if we're looming in on the previous glyph
                                bool nudged = false;
                                if (normAng != 0.0) {
                                    if (normAng < M_PI / 180.0) {
                                        const float height = span.y();
                                        const float offset = abs(sin(normAng)) * height;
                                        nudged = true;
                                        if (!walk.nextPoint(resScale * offset,&centerPt,&norm,true)) {
                                            failed = true;
                                            break;
                                        }
                                    }
                                }
                                lastNormValid = true;  lastNorm = norm;

                                // Other half of glyph
                                if (!walk.nextPoint(resScale * span.x()/2.0,nullptr,nullptr,true)) {
                                    failed = true;
                                    break;
                                }

                                // Don't forget the space between glyphs
                                if (ig < layoutObj->obj.geometry.size()-1) {
                                    const Mbr glyphNextMbr(layoutObj->obj.geometry[ig+1].coords);
                                    const float padX = std::abs(glyphNextMbr.ll().x() - glyphMbr.ur().x());
                                    walk.nextPoint(resScale * padX, nullptr, nullptr,true);
                                }
                                
                                // Translate the glyph into that position
                                const Affine2d transPlace(Translation2d((centerPt.x()-worldScreenPt.x())/2.0,
                                                                        (worldScreenPt.y()-centerPt.y())/2.0));
                                const double ang = -(atan2(norm.y(),norm.x()) - M_PI_2 + (flipped ? M_PI : 0.0));
                                const Matrix2d screenRot = Eigen::Rotation2Dd(ang).matrix();
                                Matrix3d screenRotMat = Matrix3d::Identity();
                                for (unsigned ix=0;ix<2;ix++)
                                    for (unsigned iy=0;iy<2;iy++)
                                        screenRotMat(ix, iy) = screenRot(ix, iy);
                                const Matrix3d overlapMat = transPlace.matrix() * screenRotMat * transOrigin.matrix();
                                const Matrix3d scaleMat = Eigen::AlignedScaling3d(resScale,resScale,1.0);
                                const Matrix3d testMat = screenRotMat * scaleMat * transOrigin.matrix();
                                if (flipped)
                                    layoutMats.insert(layoutMats.begin(),overlapMat);
                                else
                                    layoutMats.push_back(overlapMat);

                                // Check for overlap
                                Point2dVector objPts;  objPts.reserve(4);
                                for (unsigned int oii=0; oii < 4; oii++) {
                                    const Point3d pt = testMat * Point3d(geom.coords[oii].x(), geom.coords[oii].y(), 1.0);
                                    objPts.emplace_back(pt.x()+centerPt.x(),pt.y()+centerPt.y());
                                }
                                
//                                if (!failed) {
//                                    wkLog("  Geometry %d",ig);
//                                    for (unsigned int ip=0;ip<objPts.size();ip++) {
//                                        wkLog("    (%f,%f)",objPts[ip].x(),objPts[ip].y());
//                                    }
//                                }

                                if (!overlapMan.checkObject(objPts)) {
                                    failed = true;
                                    break;
                                }
                                
                                overlapPts.push_back(objPts);
                            }
                            
                            if (!failed) {
//                                layoutObj->obj.setRotation(textBuilder.getViewStateRotation());
                                layoutModelInstances.push_back(worldPt);
                                layoutInstances.push_back(layoutMats);
                                numInstances++;
                                
                                // Add the individual glyphs to the overlap manager
                                for (auto &glyph: overlapPts)
                                    overlapMan.addObject(glyph);
                            }
                            
                            if (layoutObj->obj.layoutRepeat > 0 && numInstances >= layoutObj->obj.layoutRepeat)
                                break;
                        }
                        
                        if (layoutObj->obj.layoutRepeat > 0 && numInstances >= layoutObj->obj.layoutRepeat)
                            break;
                    }
                    
                    if (!layoutInstances.empty()) {
                        isActive = true;
                        hadChanges = true;
                        layoutObj->newEnable = true;
                        layoutObj->changed = true;
                        layoutObj->obj.layoutPlaces = layoutInstances;
                        layoutObj->obj.layoutModelPlaces = layoutModelInstances;
                        layoutObj->newCluster = -1;
                        layoutObj->offset = Point2d(0.0,0.0);
                    } else {
                        isActive = false;
                    }
                    
                    if (layoutObj->currentEnable != isActive) {
                        layoutObj->changed = true;
                    }

                    // Debugging visual output
                    ShapeSet dispShapes = textBuilder.getVisualVecs();
                    if (!dispShapes.empty() && layoutObj->obj.layoutDebug) {
                        // Turn them back into vectors to debug
                        VectorInfo vecInfo;
                        vecInfo.color = RGBAColor::red();
                        vecInfo.lineWidth = 1.0;
                        vecInfo.drawPriority = 10000000;
                        vecInfo.programID = vecProgID;
                        
                        SimpleIdentity vecId = vecManage->addVectors(&dispShapes, vecInfo, changes);
                        if (vecId != EmptyIdentity)
                            debugVecIDs.insert(vecId);
                    }
                }
            } else {
                // Layout at a point

                // Figure out the rotation situation
                if (pickedOne)
                    isActive = false;
                
                if (isActive)
                {
                    Point2f objPt;
                    bool isInside = calcScreenPt(objPt,&layoutObj->obj,viewState,screenMbr,frameBufferSize);
                    
                    isActive &= isInside;
                    
                    // Deal with the rotation
                    float screenRot = 0.0;
                    Matrix2d screenRotMat = Matrix2d::Identity();
                    if (layoutObj->obj.rotation != 0.0)
                    {
                        screenRotMat = calcScreenRot(screenRot, viewState, globeViewState, &layoutObj->obj,
                                                     objPt, modelTrans, normalMat, frameBufferSize);
                    }
                    
                    // Now for the overlap checks
                    if (isActive)
                    {
                        // Try the four different orientations
                        if (!layoutObj->obj.layoutPts.empty())
                        {
                            bool validOrient = false;
                            for (unsigned int orient=0;orient<6;orient++)
                            {
                                // May only want to be placed certain ways.  Fair enough.
                                if (!(layoutObj->obj.acceptablePlacement & (1<<orient)))
                                    continue;

                                // Layout points are relative to the object, figure out where they are on the screen
                                const Point2dVector &layoutPts = layoutObj->obj.layoutPts;
                                const Mbr layoutMbr(layoutPts);
                                const Point2f span = layoutMbr.span();
                                const Point2f &layoutOrg = layoutMbr.ll();

                                // Set up the offset for this orientation
                                objOffset = offsetForOrientation(orient, span.cast<double>());

                                objPts[0] = objOffset + layoutOrg.cast<double>();
                                objPts[1] = objPts[0] + Point2d(span.x(), 0.0);
                                objPts[2] = objPts[0] + Point2d(span.x(), span.y());
                                objPts[3] = objPts[0] + Point2d(0.0, span.y());

                                for (auto &p : objPts)
                                {
                                    const Point2d offPt = screenRotMat * (p * resScale);
                                    p = Point2d(offPt.x(),-offPt.y()) + objPt.cast<double>();
                                }

                                //wkLogLevel(Debug, "Center pt = (%f,%f), orient = %d, pts:",objPt.x(),objPt.y(),orient);
                                //for (const auto &p : objPts) wkLogLevel(Debug, "  (%f,%f)\n",p.x(),p.y());
                                
                                // Now try it.  Objects we've pegged as essential always win
                                if (overlapMan.addCheckObject(objPts) || container.importance >= MAXFLOAT)
                                {
                                    if (showDebugBoundaries)
                                    {
                                        // Debugging visual output
                                        // The chosen placement is drawn in black.
                                        addDebugOutput(objPts,globeViewState,mapViewState,frameBufferSize,
                                                       changes, 10000000, RGBAColor::black());
                                    }

                                    validOrient = true;
                                    pickedOne = true;
                                    break;
                                }

                                if (showDebugBoundaries)
                                {
                                    // Placements that don't work are drawn in translucent blue
                                    addDebugOutput(objPts,globeViewState,mapViewState,frameBufferSize,
                                                   changes, 10000000, RGBAColor::blue().withAlpha(0.5));
                                }
                            }
                            
                            isActive = validOrient;
                        }
                    }

                    //wkLogLevel(Debug, " Valid (%s): %s, pos = (%f,%f), offset = (%f,%f)",(isActive ? "yes" : "no"),
                    //           layoutObj->obj.hint.c_str(),objPt.x(),objPt.y(),
                    //           layoutObj->offset.x(),layoutObj->offset.y());
                }
            }

            if (isActive)
                numSoFar++;
            
            // See if we've changed any of the state
            if (layoutObj->currentEnable != isActive || layoutObj->newEnable || layoutObj->offset != objOffset)
            {
                layoutObj->changed = true;
                hadChanges = true;
            }
            layoutObj->newEnable = isActive;
            layoutObj->newCluster = -1;
            layoutObj->offset = objOffset;
        }
    }
    
    //wkLogLevel(Debug, "----Finished layout---- changes=%d", hadChanges);
    
    return hadChanges;
}

// Time we'll take to disappear objects
static float const NewObjectFadeIn = 0.0;
//static float const OldObjectFadeOut = 0.0;

// Layout all the objects we're tracking
void LayoutManager::updateLayout(PlatformThreadInfo *threadInfo,const ViewStateRef &viewState,ChangeSet &changes)
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    
    if (!vecManage)
    {
        vecManage = scene->getManager<VectorManager>(kWKVectorManager);
        if (auto program = scene->findProgramByName(MaplyDefaultLineShader))
        {
            vecProgID = program->getId();
        }
        else if (auto program = scene->findProgramByName(MaplyNoBackfaceLineShader))
        {
            vecProgID = program->getId();
        }
    }
    
    if (!debugVecIDs.empty())
    {
        vecManage->removeVectors(debugVecIDs, changes);
        debugVecIDs.clear();
    }

    std::lock_guard<std::mutex> guardLock(lock);

    const TimeInterval curTime = scene->getCurrentTime();
    
    std::vector<ClusterEntry> oldClusters = std::move(clusters);
    std::vector<ClusterGenerator::ClusterClassParams> oldClusterParams = std::move(clusterParams);

    // This will recalculate the offsets and enables
    // If there were any changes, we need to regenerate
    bool layoutChanges = runLayoutRules(threadInfo,viewState,clusters,clusterParams,changes);
    
    // Compare old and new clusters
    if (!layoutChanges && clusters.size() != oldClusters.size())
    {
        layoutChanges = true;
    }

    if (clusterGen->hasChanges())
    {
        layoutChanges = true;
    }

    hasUpdates = false;

    if (!layoutChanges)
    {
        return;
    }

//    if (layoutChanges)
//        NSLog(@"LayoutChanges");

    // Get rid of the last set of drawables
    for (const auto &it : drawIDs)
        changes.push_back(new RemDrawableReq(it));
//        NSLog(@"  Remove previous drawIDs = %lu",drawIDs.size());
    drawIDs.clear();

    // Generate the drawables
    ScreenSpaceBuilder ssBuild(renderer,coordAdapter,renderer->scale);
    for (const auto &layoutObj : layoutObjects)
    {
        layoutObj->obj.offset = Point2d(layoutObj->offset.x(),layoutObj->offset.y());
        if (!layoutObj->currentEnable)
        {
            layoutObj->obj.state.fadeDown = curTime;
            layoutObj->obj.state.fadeUp = curTime+NewObjectFadeIn;
        }

        // Note: The animation below doesn't handle offsets

        // Just moved into a cluster
        if (layoutObj->currentEnable && !layoutObj->newEnable && layoutObj->newCluster > -1)
        {
            ClusterEntry *cluster = &clusters[layoutObj->newCluster];
            ClusterGenerator::ClusterClassParams &params = oldClusterParams[cluster->clusterParamID];

            // Animate from the old position to the new cluster position
            ScreenSpaceObject animObj = layoutObj->obj;
            animObj.setMovingLoc(cluster->layoutObj.worldLoc, curTime, curTime+params.markerAnimationTime);
            animObj.setEnableTime(curTime, curTime+params.markerAnimationTime);
            animObj.setFade(curTime, curTime+params.markerAnimationTime);
            animObj.state.progID = params.motionShaderID;
            for (auto &geom : animObj.geometry)
                geom.progID = params.motionShaderID;
            ssBuild.addScreenObject(animObj,animObj.worldLoc,&animObj.geometry);
        }
        else if (!layoutObj->currentEnable && layoutObj->newEnable && layoutObj->currentCluster > -1 && layoutObj->newCluster == -1)
        {
            // Just moved out of a cluster
            ClusterEntry *oldCluster = nullptr;
            if (layoutObj->currentCluster < oldClusters.size())
                oldCluster = &oldClusters[layoutObj->currentCluster];
            else {
                wkLogLevel(Warn,"Cluster ID mismatch");
                continue;
            }
            ClusterGenerator::ClusterClassParams &params = oldClusterParams[oldCluster->clusterParamID];

            // Animate from the old cluster position to the new real position
            ScreenSpaceObject animObj = layoutObj->obj;
            animObj.setMovingLoc(animObj.worldLoc, curTime, curTime+params.markerAnimationTime);
            animObj.worldLoc = oldCluster->layoutObj.worldLoc;
            animObj.setEnableTime(curTime, curTime+params.markerAnimationTime);
            animObj.setFade(curTime+params.markerAnimationTime,curTime);
            animObj.state.progID = params.motionShaderID;
            //animObj.setDrawOrder(?)
            for (auto &geom : animObj.geometry)
                geom.progID = params.motionShaderID;
            ssBuild.addScreenObject(animObj,animObj.worldLoc,&animObj.geometry);

            // And hold off on adding it
            ScreenSpaceObject shortObj = layoutObj->obj;
            //shortObj.setDrawOrder(?)
            shortObj.setEnableTime(curTime+params.markerAnimationTime, 0.0);
            ssBuild.addScreenObject(shortObj,shortObj.worldLoc,&shortObj.geometry);
        } else {
            // It's boring, just add it
            if (layoutObj->newEnable) {
                // It's a single point placement
                if (layoutObj->obj.layoutShape.empty())
                    ssBuild.addScreenObject(layoutObj->obj,layoutObj->obj.worldLoc,&layoutObj->obj.geometry);
                else {
                    // One or more placements along a path
                    for (unsigned int ii=0;ii<layoutObj->obj.layoutPlaces.size();ii++) {
                        ssBuild.addScreenObject(layoutObj->obj, layoutObj->obj.layoutModelPlaces[ii], &layoutObj->obj.geometry, &layoutObj->obj.layoutPlaces[ii]);
                    }
                }
            }
        }

        layoutObj->currentEnable = layoutObj->newEnable;
        layoutObj->currentCluster = layoutObj->newCluster;

        layoutObj->changed = false;
    }

//        NSLog(@"Got %lu clusters",clusters.size());

    // Add in the clusters
    for (const auto &cluster : clusters)
    {
        // Animate from the old cluster if there is one
        if (cluster.childOfCluster > -1)
        {
            ClusterEntry *oldCluster = nullptr;
            if (cluster.childOfCluster < oldClusters.size())
                oldCluster = &oldClusters[cluster.childOfCluster];
            else {
                wkLogLevel(Warn,"Cluster ID mismatch");
                continue;
            }
            const auto &params = oldClusterParams[oldCluster->clusterParamID];

            // Animate from the old cluster to the new one
            ScreenSpaceObject animObj = cluster.layoutObj;
            animObj.setMovingLoc(animObj.worldLoc, curTime, curTime+params.markerAnimationTime);
            animObj.worldLoc = oldCluster->layoutObj.worldLoc;
            animObj.setEnableTime(curTime, curTime+params.markerAnimationTime);
            animObj.state.progID = params.motionShaderID;
            //animObj.setDrawOrder(?)
            for (auto &geom : animObj.geometry)
                geom.progID = params.motionShaderID;
            ssBuild.addScreenObject(animObj, animObj.worldLoc, &animObj.geometry);

            // Hold off on adding the new one
            ScreenSpaceObject shortObj = cluster.layoutObj;
            //shortObj.setDrawOrder(?)
            shortObj.setEnableTime(curTime+params.markerAnimationTime, 0.0);
            ssBuild.addScreenObject(shortObj, shortObj.worldLoc, &shortObj.geometry);

        }
        else
        {
            ssBuild.addScreenObject(cluster.layoutObj, cluster.layoutObj.worldLoc, &cluster.layoutObj.geometry);
        }
    }

    ssBuild.flushChanges(changes, drawIDs);

    //NSLog(@"  Adding new drawIDs = %lu",drawIDs.size());
}

}
