/*  LayoutManager.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/15/13.
 *  Copyright 2011-2023 mousebird consulting.
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
#import "Expect.h"

using namespace Eigen;

namespace WhirlyKit
{

LayoutObject::LayoutObject(SimpleIdentity theId) :
    ScreenSpaceObject(theId)
{
}

LayoutObject::LayoutObject(LayoutObject &&other) noexcept :
        ScreenSpaceObject(std::forward<ScreenSpaceObject>(other)),
        layoutPts          (std::move(other.layoutPts)),
        selectPts          (std::move(other.selectPts)),
        uniqueID           (std::move(other.uniqueID)),
        importance         (other.importance),
        clusterGroup       (other.clusterGroup),
        layoutRepeat       (other.layoutRepeat),
        layoutOffset       (other.layoutOffset),
        layoutSpacing      (other.layoutSpacing),
        layoutWidth        (other.layoutWidth),
        layoutDebug        (other.layoutDebug),
        layoutShape        (std::move(other.layoutShape)),
        mergeID            (std::move(other.mergeID)),
        layoutPlaces       (std::move(other.layoutPlaces)),
        layoutModelPlaces  (std::move(other.layoutModelPlaces)),
        acceptablePlacement(other.acceptablePlacement),
        hint               (std::move(other.hint))
{
}

LayoutObject &LayoutObject::operator=(LayoutObject &&other) noexcept
{
    if (this != &other)
    {
        this->ScreenSpaceObject::operator=(std::forward<LayoutObject&&>(other));
        layoutPts           = std::move(other.layoutPts);
        selectPts           = std::move(other.selectPts);
        uniqueID            = std::move(other.uniqueID);
        importance          = other.importance;
        clusterGroup        = other.clusterGroup;
        layoutRepeat        = other.layoutRepeat;
        layoutOffset        = other.layoutOffset;
        layoutSpacing       = other.layoutSpacing;
        layoutWidth         = other.layoutWidth;
        layoutDebug         = other.layoutDebug;
        layoutShape         = std::move(other.layoutShape);
        layoutPlaces        = std::move(other.layoutPlaces);
        layoutModelPlaces   = std::move(other.layoutModelPlaces);
        mergeID             = std::move(other.mergeID);
        acceptablePlacement = other.acceptablePlacement;
        hint                = std::move(other.hint);
    }
    return *this;
}

void LayoutObject::setLayoutSize(const Point2d &layoutSize,const Point2d &offset)
{
    if (layoutSize.x() == 0.0 && layoutSize.y() == 0.0)
        return;

    layoutPts.clear();
    layoutPts.reserve(4);
    layoutPts.push_back(Point2d(0,0)+offset);
    layoutPts.push_back(Point2d(layoutSize.x(),0.0)+offset);
    layoutPts.push_back(layoutSize+offset);
    layoutPts.push_back(Point2d(0.0,layoutSize.y())+offset);
}

void LayoutObject::setSelectSize(const Point2d &selectSize,const Point2d &offset)
{
    if (selectSize.x() == 0.0 && selectSize.y() == 0.0)
        return;

    selectPts.clear();
    selectPts.reserve(4);
    selectPts.push_back(Point2d(0,0)+offset);
    selectPts.push_back(Point2d(selectSize.x(),0.0)+offset);
    selectPts.push_back(selectSize+offset);
    selectPts.push_back(Point2d(0.0,selectSize.y())+offset);
}

LayoutObjectEntry::LayoutObjectEntry(SimpleIdentity theId)
    : Identifiable(theId)
{
}

LayoutObjectEntry::LayoutObjectEntry(const LayoutObject &inObj) : //NOLINT
    Identifiable(inObj.getId()),
    obj(inObj)
{
}

LayoutObjectEntry::LayoutObjectEntry(LayoutObject &&inObj) noexcept :
    Identifiable(inObj.getId()),
    obj(std::move(inObj))
{
}

ClusterEntry::ClusterEntry(ClusterEntry &&other) noexcept :
    layoutObj(std::move(other.layoutObj)),
    objectIDs(std::move(other.objectIDs)),
    childOfCluster(other.childOfCluster),
    clusterParamID(other.clusterParamID)
{
}

ClusterEntry &ClusterEntry::operator=(ClusterEntry &&other) noexcept
{
    if (this != &other)
    {
        layoutObj = std::move(other.layoutObj);
        objectIDs = std::move(other.objectIDs);
        childOfCluster = other.childOfCluster;
        clusterParamID = other.clusterParamID;
    }
    return *this;
}

LayoutManager::LayoutManager() :
    SceneManager(),
    minLayoutTime(0.0)
{
}

LayoutManager::~LayoutManager()
{
    try
    {
        std::lock_guard<std::mutex> guardLock(lock);
        layoutObjects.clear();
    }
    WK_STD_DTOR_CATCH()
}

void LayoutManager::setMaxDisplayObjects(int numObjects)
{
    std::lock_guard<std::mutex> guardLock(lock);

    maxDisplayObjects = numObjects;
}

void LayoutManager::setOverrideUUIDs(const std::set<std::string> &uuids)
{
    std::lock_guard<std::mutex> guardLock(lock);

    overrideUUIDs.clear();
    overrideUUIDs.reserve(uuids.size());
    overrideUUIDs.insert(uuids.begin(), uuids.end());
}

void LayoutManager::addLayoutObjects(const std::vector<LayoutObject> &newObjects)
{
    if (!newObjects.empty() && !shutdown)
    {
        // Construct the new objects first
        std::vector<LayoutObjectEntryRef> toAdd;
        toAdd.reserve(newObjects.size());
        for (const auto &newObject : newObjects)
        {
            toAdd.emplace_back(std::make_shared<LayoutObjectEntry>(newObject));
        }

        // then dump them into the set
        addLayoutObjects(std::move(toAdd));
    }
}

void LayoutManager::addLayoutObjects(const std::vector<LayoutObject *> &newObjects)
{
    if (!newObjects.empty() && !shutdown)
    {
        // Construct the new objects first
        std::vector<LayoutObjectEntryRef> toAdd;
        toAdd.reserve(newObjects.size());
        for (const auto &newObject : newObjects)
        {
            toAdd.emplace_back(std::make_shared<LayoutObjectEntry>(*newObject));
        }
        addLayoutObjects(std::move(toAdd));
    }
}

void LayoutManager::addLayoutObjects(std::vector<LayoutObject> &&newObjects)
{
    if (!newObjects.empty() && !shutdown)
    {
        // Construct the new objects first
        std::vector<LayoutObjectEntryRef> toAdd;
        toAdd.reserve(newObjects.size());
        for (auto &newObject : newObjects)
        {
            toAdd.emplace_back(std::make_shared<LayoutObjectEntry>(std::move(newObject)));
        }
        addLayoutObjects(std::move(toAdd));
    }
}

void LayoutManager::addLayoutObjects(std::vector<LayoutObjectRef> &&newObjects)
{
    if (!newObjects.empty() && !shutdown)
    {
        // Construct the new objects first
        std::vector<LayoutObjectEntryRef> toAdd;
        toAdd.reserve(newObjects.size());
        for (auto &newObject : newObjects)
        {
            toAdd.emplace_back(std::make_shared<LayoutObjectEntry>(std::move(*newObject)));
        }
        addLayoutObjects(std::move(toAdd));
    }
}

void LayoutManager::addLayoutObjects(std::vector<LayoutObjectEntryRef> &&toAdd)
{
    std::lock_guard<std::mutex> guardLock(lock);
    layoutObjects.insert(std::make_move_iterator(toAdd.begin()),
                         std::make_move_iterator(toAdd.end()));
    hasUpdates = true;
}

/// Enable/disable layout objects
void LayoutManager::enableLayoutObjects(const SimpleIDSet &theObjectIds,bool enable)
{
    if (theObjectIds.empty())
    {
        return;
    }

    const auto key = std::make_shared<LayoutObjectEntry>(EmptyIdentity);

    std::lock_guard<std::mutex> guardLock(lock);

    for (const auto theObjectId : theObjectIds)
    {
        key->setId(theObjectId);
        const auto eit = layoutObjects.find(key);
        if (eit != layoutObjects.end())
        {
            const LayoutObjectEntryRef &entry = *eit;
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

void LayoutManager::removeLayoutObjects(const SimpleIDSet &oldObjectIds)
{
    if (oldObjectIds.empty())
    {
        return;
    }

    const auto key = std::make_shared<LayoutObjectEntry>(EmptyIdentity);

    std::lock_guard<std::mutex> guardLock(lock);

    for (const auto oldObjectId : oldObjectIds)
    {
        key->setId(oldObjectId);
        if (layoutObjects.erase(key))
        {
            hasUpdates = true;
            hasRemoves = true;
        }
    }
}

bool LayoutManager::hasChanges()
{
    if (auto cg = clusterGenerator)
    if (cg->hasChanges())
    {
        hasUpdates = true;
    }
    return hasUpdates;
}

void LayoutManager::setFadeEnabled(bool enable)
{
    std::lock_guard<std::mutex> guardLock(lock);
    fadeEnabled = enable;
    hasUpdates = true;
}

void LayoutManager::setFadeInTime(TimeInterval time)
{
    std::lock_guard<std::mutex> guardLock(lock);
    newObjectFadeIn = time;
    hasUpdates = true;
}

void LayoutManager::setFadeOutTime(TimeInterval time)
{
    std::lock_guard<std::mutex> guardLock(lock);
    oldObjectFadeOut = time;
    hasUpdates = true;
}

// Return the screen space objects in a form the selection manager can understand
void LayoutManager::getScreenSpaceObjects(const SelectionManager::PlacementInfo &pInfo,
                                          std::vector<ScreenSpaceObjectLocation> &screenSpaceObjs)
{
    // Allocate first, we don't care if the sizes aren't exactly right
    screenSpaceObjs.reserve(layoutObjects.size() + clusters.size());

    std::lock_guard<std::mutex> guardLock(lock);

    // First the regular screen space objects
    for (const auto &entry : layoutObjects)
    {
        if (entry->currentEnable && entry->obj.enable)
        {
            screenSpaceObjs.emplace_back();
            ScreenSpaceObjectLocation &ssObj = screenSpaceObjs.back();
            ssObj.shapeIDs.push_back(entry->obj.getId());
            ssObj.dispLoc = entry->obj.worldLoc;
            ssObj.rotation = entry->obj.rotation;
            ssObj.keepUpright = entry->obj.keepUpright;
            ssObj.offset = entry->offset;
            ssObj.pts = entry->obj.selectPts;
            ssObj.mbr.addPoints(entry->obj.selectPts);
        }
    }

    // Then the clusters
    for (const auto &cluster : clusters)
    {
        screenSpaceObjs.emplace_back();
        ScreenSpaceObjectLocation &ssObj = screenSpaceObjs.back();
        ssObj.shapeIDs = cluster.objectIDs;
        ssObj.dispLoc = cluster.layoutObj.worldLoc;
        ssObj.offset = cluster.layoutObj.offset;
        ssObj.pts = cluster.layoutObj.selectPts;
        ssObj.mbr.addPoints(cluster.layoutObj.selectPts);
        ssObj.clusterGroup = cluster.layoutObj.clusterGroup;
        ssObj.clusterId = cluster.layoutObj.getId();
    }
}

void LayoutManager::addClusterGenerator(PlatformThreadInfo *, ClusterGeneratorRef inClusterGen)
{
    clusterGenerator = std::move(inClusterGen);
    hasUpdates = true;
}

void LayoutManager::setRenderer(SceneRenderer *inRenderer)
{
    if (!inRenderer && renderer && !shutdown)
    {
        // An `updateLayout` may be running right now, and clearing `renderer` could crash.
        // The scene should have canceled already, but try to handle it by setting the cancel
        // flag and waiting just a little while for layout to not be running.
        cancelUpdate();
        constexpr auto wait = std::chrono::milliseconds(50);
        const auto lock = std::unique_lock<std::timed_mutex>(internalLock, wait);
        wkLogLevel(Warn, "Layout teardown without cancellation, %s",
                   lock.owns_lock() ? "successfully canceled" : "proceeding unsafely");
    }
    SceneManager::setRenderer(inRenderer);
}

void LayoutManager::setScene(Scene *inScene)
{
    SceneManager::setScene(inScene);
    if (!inScene)
    {
        cancelUpdate();
    }
}

void LayoutManager::teardown()
{
    cancelUpdate();
    SceneManager::teardown();
}

void LayoutManager::cancelUpdate()
{
    std::lock_guard<std::mutex> guardLock(lock);
    cancelLayout = true;
}

// Add the object, unless it's already present.
//
// If the object replaces an existing object, that replaced object and true are returned.
// If the object is already present and is not added, the existing object and false are returned.
// Otherwise, the inserted object and true are returned.
std::pair<LayoutObjectEntryRef,bool> LayoutManager::ClusteredObjects::addObject(LayoutObjectEntryRef obj)
{
    // If it has a unique ID...
    if (!obj->obj.uniqueID.empty())
    {
        // Try adding it to the unique ID set
        const auto result = uniqueLayoutObjects.insert(obj);
        if (!result.second)
        {
            // An item with this ID already exists, check the importance
            const LayoutObjectEntryRef otherObj = *result.first;    // Note: Copy the ref, the iterator's value will change
            if (obj->obj.importance > otherObj->obj.importance)
            {
                // The new object is more important.  Remove the existing
                // object from both sets and add the new one in its place.
                uniqueLayoutObjects.insert(uniqueLayoutObjects.erase(result.first), obj);

                // The layout objects are arranged by importance, and `set::find` won't match
                // anything, so search the range of not-less and not-greater for the unique ID.
                for (auto range = layoutObjects.equal_range(otherObj);
                     range.first != range.second; ++range.first)
                {
                    if ((*range.first)->obj.uniqueID == otherObj->obj.uniqueID)
                    {
                        layoutObjects.erase(range.first);
                        break;  // We only expect one match
                    }
                }
                layoutObjects.insert(obj);
                return std::make_pair(otherObj, true);
            }
            else
            {
                // This object is less important than one already present.
                return std::make_pair(otherObj, false);
            }
        }
    }
    const auto result = layoutObjects.insert(std::move(obj));
    assert(result.second);  // We expect it to always be inserted
    return std::make_pair(*result.first, true);
}

void LayoutManager::deferUntil(TimeInterval minTime)
{
    auto curMinTime = minLayoutTime.load(std::memory_order_relaxed);
    while (!minLayoutTime.compare_exchange_weak(
            curMinTime,std::max(curMinTime, minTime),
            std::memory_order_release, std::memory_order_relaxed)) {
        // no-op
    }
}

// Size of the overlap sampler grid, optimized for labels that are wider than they are tall
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
    const double rot = 2*M_PI-ssObj->rotation;

    Point3d upVec,northVec,eastVec;
    if (!globeViewState)
    {
        upVec = Point3d(0,0,1);
        northVec = Point3d(0,1,0);
        eastVec = Point3d(1,0,0);
    } else {
        const Point3d worldLoc = ssObj->getWorldLoc();
        upVec = worldLoc.normalized();
        // Vector pointing north
        northVec = Point3d(-worldLoc.x(),-worldLoc.y(),1.0-worldLoc.z());
        eastVec = northVec.cross(upVec);
        northVec = upVec.cross(eastVec);
    }

    // This vector represents the rotation in world space
    const Point3d rotVec = eastVec * sin(rot) + northVec * cos(rot);

    // Project down into screen space
    const Vector4d projRot = normalMat * Vector4d(rotVec.x(),rotVec.y(),rotVec.z(),0.0);

    // Use the resulting x & y
    screenRot = (float)(atan2(projRot.y(),projRot.x())-M_PI_2);
    // Keep the labels upright
    if (ssObj->keepUpright && screenRot > M_PI_2 && screenRot < 3*M_PI_2)
    {
        screenRot = (float)(screenRot + M_PI);
    }

    return Matrix2d(Eigen::Rotation2Dd(screenRot));
}

// Used for sorting layout objects
struct LayoutManager::LayoutObjectContainer
{
    LayoutObjectContainer() : importance(-1.0) { }
    explicit LayoutObjectContainer(LayoutObjectEntryRef entry) {
        objs.push_back(std::move(entry));
        importance = objs[0]->obj.importance;
    }

    // Objects that share the same unique ID
    std::vector<LayoutObjectEntryRef> objs;

    bool operator < (const LayoutObjectContainer &that) const {
        if (objs.empty())  // Never happen
            return false;
        return importance > that.importance;
    }

    float importance;
};

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

static Point2d offsetForOrientation(unsigned orient, const Point2d &span)
{
    // Set up the offset for this orientation
    switch (orient)
    {
        default:
        case 0: return {           0,            0 }; // Don't move at all
        case 1: return { -span.x()/2,  -span.y()/2 }; // Center
        case 2: return {         0.0,  -span.y()/2 }; // Right
        case 3: return { -span.x(),    -span.y()/2 }; // Left
        case 4: return { -span.x()/2,          0.0 }; // Above
        case 5: return { -span.x()/2,  -span.y()   }; // Below
    }
}

// Do the actual layout logic.  We'll modify the offset and on value in place.
bool LayoutManager::runLayoutRules(PlatformThreadInfo *threadInfo,
                                   const ViewStateRef &viewState,
                                   const LayoutEntrySet &localLayoutObjects,
                                   const std::unordered_set<std::string> &localOverrideUUIDs,
                                   std::vector<ClusterEntry> &clusterEntries,
                                   std::vector<ClusterGenerator::ClusterClassParams> &outClusterParams,
                                   ChangeSet &changes)
{
    if (localLayoutObjects.empty())
        return false;

    bool hadChanges = false;

    ClusteredObjectsSet clusterGroups;
    LayoutContainerVec layoutObjs;

    // Special snowflake layout objects (with unique names)
    typedef std::unordered_map<std::string,LayoutObjectContainer> UniqueLayoutObjectMap;
    UniqueLayoutObjectMap uniqueLayoutObjs(localLayoutObjects.size());

    // The globe has some special requirements
    auto globeViewState = dynamic_cast<WhirlyGlobe::GlobeViewState *>(viewState.get());
    auto mapViewState = dynamic_cast<Maply::MapViewState *>(viewState.get());

    // View related matrix stuff
    const Matrix4d modelTrans = viewState->fullMatrices[0];
    const Matrix4d fullMatrix = viewState->fullMatrices[0];
    const Matrix4d fullNormalMatrix = viewState->fullNormalMatrices[0];
    const Matrix4d normalMat = viewState->fullMatrices[0].inverse().transpose();

    // Turn everything off and sort by importance
    for (const auto &layoutObjRef : localLayoutObjects)
    {
        auto * const obj = layoutObjRef.get();
        if (obj->obj.enable)
        {
            if (UNLIKELY(cancelLayout))
            {
                break;
            }

            bool use = obj->obj.state.minVis == DrawVisibleInvalid ||
                       obj->obj.state.maxVis == DrawVisibleInvalid;
            if (!use)
            {
                if (globeViewState)
                {
                    use = obj->obj.state.minVis < globeViewState->heightAboveGlobe &&
                          globeViewState->heightAboveGlobe < obj->obj.state.maxVis;
                }
                else
                {
                    use = obj->obj.state.minVis < mapViewState->heightAboveSurface &&
                          mapViewState->heightAboveSurface < obj->obj.state.maxVis;
                }
            }

            // Make sure this one isn't behind the globe
            if (use && globeViewState)
            {
                // Layout shape following doesn't work with this check
                if (obj->obj.layoutShape.empty())
                {
                    // Make sure this one is facing toward the viewer
                    use = CheckPointAndNormFacing(obj->obj.worldLoc,obj->obj.worldLoc.normalized(),
                                                  fullMatrix,fullNormalMatrix) > 0.0;
                }
            }

            if (use)
            {
                obj->newCluster = -1;
                if (obj->obj.clusterGroup > -1)
                {
                    // Put the entry in the right cluster
                    ClusteredObjects findClusterObj(obj->obj.clusterGroup);
                    const auto cit = clusterGroups.find(&findClusterObj);
                    if (cit == clusterGroups.end())
                    {
                        // Create a new cluster object
                        auto newClusterObj = new ClusteredObjects(obj->obj.clusterGroup);
                        newClusterObj->addObject(layoutObjRef);

                        clusterGroups.insert(newClusterObj);

                        hadChanges = true;
                    }
                    else
                    {
                        const auto result = (*cit)->addObject(layoutObjRef);

                        // Was there already a matching item present?
                        if (result.first && result.first.get() != layoutObjRef.get())
                        {
                            // If the new object replaced an existing one, and that replaced
                            // object has a current cluster, copy it to the new one.
                            if (result.second && layoutObjRef->currentCluster == -1 &&
                                                 result.first->currentCluster != -1)
                            {
                                // The new object replaced an older one
                                layoutObjRef->currentCluster = result.first->currentCluster;
                            }
                            // If the new object was not added, and it has a current cluster,
                            // copy it to the object that's already present.
                            else if (!result.second && layoutObjRef->currentCluster != -1 &&
                                                       result.first->currentCluster == -1)
                            {
                                result.first->currentCluster = layoutObjRef->currentCluster;
                            }
                        }
                    }

                    obj->newEnable = false;
                }
                else    // Not a cluster
                {
                    if (layoutObjs.empty())
                    {
                        layoutObjs.reserve(localLayoutObjects.size());
                    }

                    if (obj->obj.uniqueID.empty())
                    {
                        layoutObjs.emplace_back(layoutObjRef);
                    }
                    else
                    {
                        // Add it to a container for its unique name
                        LayoutObjectContainer &dest = uniqueLayoutObjs[obj->obj.uniqueID];

                        // See if we're overriding this importance
                        dest.importance = obj->obj.importance;
                        if (!obj->obj.uniqueID.empty() && localOverrideUUIDs.find(obj->obj.uniqueID) != localOverrideUUIDs.end())
                            dest.importance = MAXFLOAT;

                        dest.objs.push_back(layoutObjRef);
                    }
                }
            }
            else
            {
                obj->newEnable = false;
                obj->newCluster = -1;
            }

            // Note: Update this for clusters
            if ((use && !obj->currentEnable) || (!use && obj->currentEnable))
            {
                hadChanges = true;
            }
        }
    }

    // Extents for the layout helpers
    const Point2f frameBufferSize = renderer->getFramebufferSize();
    const Mbr screenMbr(frameBufferSize * -ScreenBuffer,
                        frameBufferSize * (1.0 + ScreenBuffer));

    // Need to scale for retina displays
    const float resScale = renderer->getScale();

    runLayoutClustering(threadInfo, layoutObjs, clusterGroups, clusterEntries,
                        outClusterParams, viewState, mapViewState, globeViewState,
                        frameBufferSize, screenMbr, modelTrans, normalMat);

    if (UNLIKELY(cancelLayout))
    {
        return false;
    }

//    NSLog(@"----Starting Layout----");

    // Set up the overlap sampler
    OverlapHelper overlapMan(screenMbr,OverlapSampleX,OverlapSampleY,localLayoutObjects.size());

    // Add in the unique objects, cluster entries and then sort them all
    for (auto &it : uniqueLayoutObjs)
    {
        layoutObjs.push_back(it.second);
    }
    std::sort(layoutObjs.begin(),layoutObjs.end());

    // Clusters have priority in the overlap.
    for (const auto &it : clusterEntries)
    {
        Point2f objPt = {0,0};
        /*const bool isInside = */calcScreenPt(objPt,&it.layoutObj,viewState,screenMbr,frameBufferSize);
        auto objPts = it.layoutObj.layoutPts;   // make a copy
        for (auto &pt : objPts)
        {
            pt = pt * resScale + objPt.cast<double>();
        }
        overlapMan.addObject(objPts);
    }

    std::unordered_multimap<std::string, LayoutObjectEntryRef> mergeMap(localLayoutObjects.size());

    // Lay out the various objects that are active
    int numSoFar = 0;
    for (auto &container : layoutObjs)
    {
        if (UNLIKELY(cancelLayout))
        {
            break;
        }

        Point2d objOffset(0.0,0.0);
        Point2dVector objPts(4);

        // Start with a max objects check
        bool isActive = (maxDisplayObjects == 0 || (numSoFar < maxDisplayObjects));

        // Sort the objects by importance within their container, large to small
        std::sort(container.objs.begin(),container.objs.end(),
                  [](const LayoutObjectEntryRef &a,const LayoutObjectEntryRef &b) -> bool {
                      return a->obj.importance > b->obj.importance;
                  });

        // Some of these may share unique IDs
        bool pickedOne = false;

        for (auto &layoutObj : container.objs)
        {
            if (UNLIKELY(cancelLayout))
            {
                break;
            }

            layoutObj->newEnable = false;
            layoutObj->obj.layoutModelPlaces.clear();
            layoutObj->obj.layoutPlaces.clear();

            // Layout along a shape
            if (!layoutObj->obj.layoutShape.empty())
            {
                layoutAlongShape(layoutObj, viewState, frameBufferSize, overlapMan, changes, isActive, hadChanges);
            }
            else
            {
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
                                if (!(layoutObj->obj.acceptablePlacement & (1U<<orient)))
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
                                if (container.importance >= MAXFLOAT ||
                                    overlapMan.addCheckObject(objPts, layoutObj->obj.mergeID))
                                {
                                    if (showDebugBoundaries || layoutObj->obj.layoutDebug)
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

                                if (showDebugBoundaries || layoutObj->obj.layoutDebug)
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

            //wkLog("%d n=%lld id=%s active=%s picked=%s", numSoFar,
            //      container.objs.size(), layoutObj->obj.uniqueID.c_str(),
            //      isActive?"T":"F", pickedOne?"T":"F");
            
            if (isActive)
                numSoFar++;

            // Keep merged items in sync.
            if (!layoutObj->obj.mergeID.empty())
            {
                // Consider the objects we've already seen with the same merge ID
                const auto range = mergeMap.equal_range(layoutObj->obj.mergeID);
                for (auto ii = range.first; ii != range.second; ++ii)
                {
                    auto &prevObj = *ii->second;
                    if (isActive && !prevObj.newEnable)
                    {
                        // That object was disabled, we need to disable this one to match.
                        isActive = false;
                        layoutObj->newEnable = false;
                        // we can stop looking
                        break;
                    }
                    else if (!isActive && prevObj.newEnable)
                    {
                        // That object was enabled, we need to disable it to match this one.
                        // This might actually undo the change leaving us with no changes, but we
                        // can't easily detect that.
                        prevObj.newEnable = false;
                        layoutObj->changed = true;
                        hadChanges = true;
                    }
                }
                // If this one is still enabled, or is the first disabled
                // item of its ID that we've seen, we need to keep track of it.
                if (layoutObj->newEnable || range.first == range.second)
                {
                    mergeMap.insert(std::make_pair(layoutObj->obj.mergeID, layoutObj));
                }
            }

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

void LayoutManager::runLayoutClustering(PlatformThreadInfo *threadInfo,
                                        LayoutContainerVec &layoutObjs,
                                        ClusteredObjectsSet &clusterGroups,
                                        std::vector<ClusterEntry> &clusterEntries,
                                        std::vector<ClusterGenerator::ClusterClassParams> &outClusterParams,
                                        const ViewStateRef &viewState,
                                        Maply::MapViewState *mapViewState,
                                        WhirlyGlobe::GlobeViewState *globeViewState,
                                        const Point2f &frameBufferSize,
                                        const Mbr &screenMbr,
                                        const Matrix4d &modelTrans,
                                        const Matrix4d &normalMat)
{
    const float resScale = renderer->getScale();

    const auto clusterGen = clusterGenerator;
    if (!clusterGen)
    {
        return;
    }

    clusterGen->startLayoutObjects(threadInfo);

    // Lay out the cluster groups in order
    for (const auto &cluster : clusterGroups)
    {
        outClusterParams.resize(outClusterParams.size() + 1);
        ClusterGenerator::ClusterClassParams &params = outClusterParams.back();
        clusterGen->paramsForClusterClass(threadInfo,cluster->clusterID,params);

        ClusterHelper clusterHelper(screenMbr,OverlapSampleX,OverlapSampleY,resScale,params.clusterSize);

        // Add all the various objects to the cluster and figure out overlaps
        for (const auto &entry : cluster->getLayoutObjects())
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
                {
                    screenRotMat = calcScreenRot(screenRot,viewState,globeViewState,&entry->obj,
                                                 objPt,modelTrans,normalMat,frameBufferSize);
                }

                // Rotate the rectangle
                Point2dVector objPts(4);
                if (screenRot == 0.0)
                {
                    for (unsigned int ii=0;ii<4;ii++)
                        objPts[ii] = Point2d(objPt.x(),objPt.y()) + entry->obj.layoutPts[ii] * resScale;
                }
                else
                {
                    Point2d center = objPt.cast<double>();
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
        clusterHelper.resolveClusters(cancelLayout);

        if (UNLIKELY(cancelLayout))
        {
            break;
        }

        // Toss the unaffected layout objects into the mix
        layoutObjs.reserve(layoutObjs.size() + clusterHelper.simpleObjects.size());
        for (const auto &obj : clusterHelper.simpleObjects)
        {
            if (obj.parentObject < 0)
            {
                layoutObjs.emplace_back(obj.objEntry);
                obj.objEntry->newEnable = true;
                obj.objEntry->newCluster = -1;
            }
        }

        // Create new objects for the clusters
        for (const auto &clusterObj : clusterHelper.clusterObjects)
        {
            std::vector<LayoutObjectEntryRef> objsForCluster;
            clusterHelper.objectsForCluster(clusterObj,objsForCluster);

            if (!objsForCluster.empty())
            {
                const int clusterEntryID = (int)clusterEntries.size();
                clusterEntries.emplace_back();
                ClusterEntry &clusterEntry = clusterEntries.back();

                const Point2f clusterLoc = clusterObj.center.cast<float>();

                // Project the cluster back into a geolocation so we can place it.
                Point3d dispPt;
                bool dispPtValid = false;
                if (globeViewState)
                {
                    dispPtValid = globeViewState->pointOnSphereFromScreen(clusterLoc,modelTrans,frameBufferSize,dispPt);
                }
                else
                {
                    dispPtValid = mapViewState->pointOnPlaneFromScreen(clusterLoc,modelTrans,frameBufferSize,dispPt,false);
                }

                // Note: What happens if the display point isn't valid?
                if (dispPtValid)
                {
                    clusterEntry.layoutObj.worldLoc = dispPt;
                    for (const auto &thisObj : objsForCluster)
                    {
                        clusterEntry.objectIDs.push_back(thisObj->obj.getId());
                    }
                    clusterGen->makeLayoutObject(threadInfo,cluster->clusterID, objsForCluster, clusterEntry.layoutObj);
                    if (!params.selectable)
                    {
                        clusterEntry.layoutObj.selectPts.clear();
                    }
                }
                clusterEntry.clusterParamID = (int)(outClusterParams.size() - 1);

                // Figure out if all the objects in this new cluster come from the same old cluster
                //  and assign the new cluster ID
                int whichOldCluster = -1;
                for (const auto &obj : objsForCluster)
                {
                    if (obj->currentCluster > -1 && whichOldCluster != -2)
                    {
                        if (whichOldCluster == -1)
                        {
                            whichOldCluster = obj->currentCluster;
                        }
                        else if (whichOldCluster != obj->currentCluster)
                        {
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
    for (auto clusterObj : clusterGroups)
    {
        delete clusterObj;
    }
    clusterGroups.clear();

    clusterGen->endLayoutObjects(threadInfo);
}

void LayoutManager::layoutAlongShape(const LayoutObjectEntryRef &layoutObj,
                                     const ViewStateRef &viewState,
                                     const Point2f &frameBufferSize,
                                     OverlapHelper &overlapMan,
                                     ChangeSet &changes,
                                     bool &isActive,
                                     bool &hadChanges)
{
    const float resScale = renderer->getScale();

    for (unsigned int oi=0;oi<viewState->viewMatrices.size();oi++)
    {
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

        // We need the length of the glyphs and their center
        const Mbr layoutMbr(layoutObj->obj.layoutPts);
        const float textLen = layoutMbr.ur().x();
        const float midY = layoutMbr.mid().y();

        // Storage reused for each instance
        std::vector<Eigen::Matrix3d> layoutMats;
        std::vector<Point2dVector> overlapPts;

        const auto &runs = textBuilder.getScreenVecsRef();
        for (const auto& run: runs)
        {
            //wkLog("Run %d: %d points",ri++,run.size());

            LinearWalker walk(run);

            // Figure out how many times we could lay this out
            const float textRoom = walk.getTotalLength() - 2.0f*layoutObj->obj.layoutSpacing;
            const int textInstance = std::max(0, (int)(textRoom / textLen));

            for (unsigned int ini=0;ini<textInstance;ini++)
            {
                //wkLog(" Text Instance %d",ini);

                // Start with an initial offset
                if (!walk.nextPoint(layoutObj->obj.layoutSpacing, nullptr, nullptr, true))
                    continue;

                // Check the normal right in the middle
                Point2f normAtMid;
                if (!walk.nextPoint(textLen/2.0, nullptr, &normAtMid, false))
                    continue;

                // Center around the world point on the screen
                Point2f midRun;
                if (!walk.nextPoint(resScale * layoutMbr.span().x()/2.0, &midRun, nullptr, false))
                    continue;
//                            wkLogLevel(Info, "midRun = (%f,%f)",midRun.x(),midRun.y());
                Point2f worldScreenPt = midRun;
                Point3d worldPt(0.0,0.0,0.0);
                if (!textBuilder.screenToWorld(midRun, worldPt))
                    continue;

                layoutMats.clear();
                overlapPts.clear();

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
                        normAng = std::atan2(det, dot);
                        //wkLogLevel(Debug,"normAng = %f",normAng);
                        if (normAng != 0.0 && std::abs(normAng) > 45.0 * M_PI / 180.0) {
                            failed = true;
                            break;
                        }
                    }

                    // And let's nudge it over a bit if we're looming in on the previous glyph
                    //bool nudged = false;
                    if (normAng != 0.0) {
                        if (normAng < M_PI / 180.0) {
                            const float height = span.y();
                            const auto offset = std::abs(std::sin(normAng)) * height;
                            //nudged = true;
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
                    const double ang = -(std::atan2(norm.y(),norm.x()) - M_PI_2 + (flipped ? M_PI : 0.0));
                    const Matrix2d screenRot = Eigen::Rotation2Dd(ang).matrix();
                    Matrix3d screenRotMat = Matrix3d::Identity();
                    for (int ix=0;ix<2;ix++)
                        for (int iy=0;iy<2;iy++)
                            screenRotMat(ix, iy) = screenRot(ix, iy);
                    const Matrix3d overlapMat = transPlace.matrix() * screenRotMat * transOrigin.matrix();
                    const Matrix3d scaleMat = Eigen::AlignedScaling3d(resScale,resScale,1.0);
                    const Matrix3d testMat = screenRotMat * scaleMat * transOrigin.matrix();
                    if (flipped)
                        layoutMats.insert(layoutMats.begin(),overlapMat);
                    else
                        layoutMats.push_back(overlapMat);

                    // Check for overlap
                    Point2dVector thePts;  thePts.reserve(4);
                    for (unsigned int oii=0; oii < 4; oii++) {
                        const Point3d pt = testMat * Point3d(geom.coords[oii].x(), geom.coords[oii].y(), 1.0);
                        thePts.emplace_back(pt.x() + centerPt.x(), pt.y() + centerPt.y());
                    }

//                                if (!failed) {
//                                    wkLog("  Geometry %d",ig);
//                                    for (unsigned int ip=0;ip<objPts.size();ip++) {
//                                        wkLog("    (%f,%f)",objPts[ip].x(),objPts[ip].y());
//                                    }
//                                }

                    if (!overlapMan.checkObject(thePts))
                    {
                        failed = true;
                        break;
                    }

                    overlapPts.push_back(thePts);
                }

                if (failed)
                {
                    continue;
                }
                
                if (layoutInstances.empty())
                {
                    layoutInstances.reserve(runs.size() * std::max(1, layoutObj->obj.layoutRepeat));
                }
                
                //layoutObj->obj.setRotation(textBuilder.getViewStateRotation());

                layoutModelInstances.push_back(worldPt);
                layoutInstances.push_back(layoutMats);

                // Add the individual glyphs to the overlap manager
                for (auto &glyph: overlapPts)
                {
                    overlapMan.addObject(glyph);
                }

                if (layoutObj->obj.layoutRepeat > 0 && layoutInstances.size() >= layoutObj->obj.layoutRepeat)
                    break;
            }

            if (layoutObj->obj.layoutRepeat > 0 && layoutInstances.size() >= layoutObj->obj.layoutRepeat)
                break;
        }

        if (!layoutInstances.empty())
        {
            isActive = true;
            hadChanges = true;
            layoutObj->newEnable = true;
            layoutObj->changed = true;
            layoutObj->obj.layoutPlaces = std::move(layoutInstances);
            layoutObj->obj.layoutModelPlaces = std::move(layoutModelInstances);
            layoutObj->newCluster = -1;
            layoutObj->offset = Point2d(0.0,0.0);
        }
        else
        {
            isActive = false;
        }

        if (layoutObj->currentEnable != isActive)
        {
            layoutObj->changed = true;
        }

        // Debugging visual output
        if (layoutObj->obj.layoutDebug)
        {
            const ShapeSet dispShapes = textBuilder.getVisualVecs();
            if (!dispShapes.empty())
            {
                // Turn them back into vectors to debug
                VectorInfo vecInfo;
                vecInfo.color = RGBAColor::red();
                vecInfo.lineWidth = 1.0;
                vecInfo.drawPriority = 10000000;

                vecInfo.programID = vecProgID;

                const SimpleIdentity vecId = vecManage->addVectors(&dispShapes, vecInfo, changes);
                if (vecId != EmptyIdentity)
                {
                    debugVecIDs.insert(vecId);
                }
            }
        }
    }
}

void LayoutManager::buildDrawables(ScreenSpaceBuilder &ssBuild,
                    bool doFades, bool doClusters,
                    TimeInterval curTime, TimeInterval *maxAnimTime,
                    const LayoutEntrySet &localLayoutObjects,
                    const std::vector<ClusterEntry> &oldClusters,
                    const std::vector<ClusterGenerator::ClusterClassParams> &oldClusterParams,
                    UnorderedIDSetbyUID *newUniqueDrawableMap,
                    const UnorderedIDSetbyUID *oldUniqueDrawableMap)
{
    for (const auto &layoutObj : localLayoutObjects)
    {
        if (UNLIKELY(cancelLayout || !renderer))
        {
            break;
        }

        if (doFades && !layoutObj->obj.uniqueID.empty() && oldUniqueDrawableMap)
        {
            const auto frameInfo = renderer->getFrameInfo();

            // See if this object generated any drawables in the previous run.
            const auto hit = oldUniqueDrawableMap->find(layoutObj->obj.uniqueID);
            bool isNew = (hit == oldUniqueDrawableMap->end() || hit->second.empty());
            if (!isNew && checkDrawableOn && frameInfo)
            {
                // Only consider the first drawable, assuming that they are all shown/hidden together
                if (auto draw = scene->getDrawable(*hit->second.begin()))
                {
                    if (!draw->isOn(frameInfo.get()))
                    {
                        // The drawable existed, but (probably) wasn't being shown, so treat it as not present.
                        isNew = true;
                    }
                }
            }
            if (isNew)
            {
                // It's new, fade it in
                layoutObj->obj.setFade(curTime+newObjectFadeIn, curTime);

                // Don't run again before the fades are complete
                if (maxAnimTime)
                {
                    *maxAnimTime = std::max(*maxAnimTime, curTime+newObjectFadeIn);
                }
            }
        }

        layoutObj->obj.offset = layoutObj->offset;

        // Note: The animation below doesn't handle offsets

        // Just moved into a cluster
        if (layoutObj->currentEnable && !layoutObj->newEnable && layoutObj->newCluster >= 0)
        {
            ClusterEntry *cluster = &clusters[layoutObj->newCluster];
            const auto &params = oldClusterParams[cluster->clusterParamID];

            // Animate from the old position to the new cluster position
            ScreenSpaceObject animObj = layoutObj->obj;     // NOLINT slicing LayoutObject to ScreenSpaceObject
            animObj.setMovingLoc(cluster->layoutObj.worldLoc, curTime, curTime+params.markerAnimationTime);
            animObj.setEnableTime(curTime, curTime+params.markerAnimationTime);
            animObj.setFade(curTime, curTime+params.markerAnimationTime);
            animObj.state.progID = params.motionShaderID;
            for (auto &geom : animObj.geometry)
                geom.progID = params.motionShaderID;
            ssBuild.addScreenObject(animObj,animObj.worldLoc,&animObj.geometry,nullptr);

            // Don't run again before the animations are complete
            if (maxAnimTime)
            {
                *maxAnimTime = std::max(*maxAnimTime, curTime+params.markerAnimationTime);
            }
        }
        else if (!layoutObj->currentEnable && layoutObj->newEnable && layoutObj->currentCluster > -1 && layoutObj->newCluster == -1)
        {
            // Just moved out of a cluster
            if (layoutObj->currentCluster >= oldClusters.size())
            {
                wkLogLevel(Warn,"Cluster ID mismatch");
                continue;
            }

            const ClusterEntry *oldCluster = &oldClusters[layoutObj->currentCluster];
            const auto &params = oldClusterParams[oldCluster->clusterParamID];

            // Animate from the old cluster position to the new real position
            ScreenSpaceObject animObj = layoutObj->obj; // NOLINT slicing LayoutObject to ScreenSpaceObject
            animObj.setMovingLoc(animObj.worldLoc, curTime, curTime+params.markerAnimationTime);
            animObj.worldLoc = oldCluster->layoutObj.worldLoc;
            animObj.setEnableTime(curTime, curTime+params.markerAnimationTime);
            animObj.setFade(curTime+params.markerAnimationTime,curTime);
            animObj.state.progID = params.motionShaderID;
            //animObj.setDrawOrder(?)
            for (auto &geom : animObj.geometry)
                geom.progID = params.motionShaderID;
            ssBuild.addScreenObject(animObj,animObj.worldLoc,&animObj.geometry,nullptr);

            // And hold off on adding it
            ScreenSpaceObject shortObj = layoutObj->obj;    // NOLINT slicing LayoutObject to ScreenSpaceObject
            //shortObj.setDrawOrder(?)
            shortObj.setEnableTime(curTime+params.markerAnimationTime, 0.0);
            ssBuild.addScreenObject(shortObj,shortObj.worldLoc,&shortObj.geometry,nullptr);

            // Don't run again before the animations are complete
            if (maxAnimTime)
            {
                *maxAnimTime = std::max(*maxAnimTime, curTime+params.markerAnimationTime);
            }
        }
        // It's boring, just add it
        else if (layoutObj->newEnable)
        {
            SimpleIDUnorderedSet *drawIDSet = nullptr;
            if (newUniqueDrawableMap && !layoutObj->obj.uniqueID.empty())
            {
                if (newUniqueDrawableMap->empty())
                {
                    newUniqueDrawableMap->reserve(localLayoutObjects.size());
                }

                // Look up or create the set of drawable IDs associated with this Unique ID
                const auto result = newUniqueDrawableMap->insert(std::make_pair(
                    std::ref(layoutObj->obj.uniqueID), SimpleIDUnorderedSet()));
                drawIDSet = &result.first->second;
                if (result.second)
                {
                    drawIDSet->reserve(10);  // ?
                }
            }

            // It's a single point placement
            SimpleIDUnorderedSet tempSet;
            if (layoutObj->obj.layoutShape.empty())
            {
                ssBuild.addScreenObject(layoutObj->obj, layoutObj->obj.worldLoc,
                                        &layoutObj->obj.geometry, nullptr, &tempSet);
            }
            else
            {
                // One or more placements along a path
                for (unsigned int ii=0;ii<layoutObj->obj.layoutPlaces.size();ii++)
                {
                    ssBuild.addScreenObject(layoutObj->obj, layoutObj->obj.layoutModelPlaces[ii],
                                            &layoutObj->obj.geometry, &layoutObj->obj.layoutPlaces[ii],
                                            &tempSet);
                }
            }
            if (drawIDSet)
            {
                drawIDSet->insert(tempSet.begin(), tempSet.end());
            }
        }

        layoutObj->currentEnable = layoutObj->newEnable;
        layoutObj->currentCluster = layoutObj->newCluster;

        layoutObj->changed = false;
    }

    if (cancelLayout)
    {
        cancelLayout = false;
        return;
    }

    if (!doClusters)
    {
        return;
    }

    // Add in the clusters
    for (const auto &cluster : clusters)
    {
        if (UNLIKELY(cancelLayout))
        {
            break;
        }

        // Animate from the old cluster if there is one
        if (cluster.childOfCluster > -1)
        {
            if (cluster.childOfCluster >= oldClusters.size())
            {
                wkLogLevel(Warn,"Cluster ID mismatch");
                continue;
            }
            const ClusterEntry *oldCluster = &oldClusters[cluster.childOfCluster];
            const auto &params = oldClusterParams[oldCluster->clusterParamID];

            // Animate from the old cluster to the new one
            ScreenSpaceObject animObj = cluster.layoutObj;  // NOLINT slicing LayoutObject to ScreenSpaceObject
            animObj.setMovingLoc(animObj.worldLoc, curTime, curTime+params.markerAnimationTime);
            animObj.worldLoc = oldCluster->layoutObj.worldLoc;
            animObj.setEnableTime(curTime, curTime+params.markerAnimationTime);
            animObj.state.progID = params.motionShaderID;
            //animObj.setDrawOrder(?)
            for (auto &geom : animObj.geometry)
                geom.progID = params.motionShaderID;
            ssBuild.addScreenObject(animObj, animObj.worldLoc, &animObj.geometry);

            // Hold off on adding the new one
            ScreenSpaceObject shortObj = cluster.layoutObj; // NOLINT slicing LayoutObject to ScreenSpaceObject
            //shortObj.setDrawOrder(?)
            shortObj.setEnableTime(curTime+params.markerAnimationTime, 0.0);
            ssBuild.addScreenObject(shortObj, shortObj.worldLoc, &shortObj.geometry);
        }
        else
        {
            ssBuild.addScreenObject(cluster.layoutObj, cluster.layoutObj.worldLoc, &cluster.layoutObj.geometry);
        }
    }
}

void LayoutManager::handleFadeOut(const TimeInterval curTime,
                                  TimeInterval &maxAnimTime,
                                  const LayoutEntrySet &localLayoutObjects,
                                  const SimpleIDSet &oldDrawIDs,
                                  const std::vector<BasicDrawableRef> &newDrawables,
                                  const std::vector<ClusterEntry> &oldClusters,
                                  const std::vector<ClusterGenerator::ClusterClassParams> &oldClusterParams,
                                  const UnorderedIDSetbyUID &oldUniqueDrawableMap,
                                  const UnorderedIDSetbyUID &newUniqueDrawableMap,
                                  ChangeSet &changes)
{
    if (!fadeEnabled || oldObjectFadeOut <= 0 || oldDrawIDs.empty() || prevLayoutObjects.empty())
    {
        return;
    }
    
    // Reverse the map so we can look up unique IDs from drawables
    std::unordered_map<SimpleIdentity,const std::string*> oldUniqueIDsByDrawable(oldDrawIDs.size());
    for (const auto &kv : oldUniqueDrawableMap)
    {
        for (SimpleIdentity drawID : kv.second)
        {
            oldUniqueIDsByDrawable.insert(std::make_pair(drawID, &kv.first));
        }
    }

    std::unordered_map<SimpleIdentity,BasicDrawableRef> newDrawsByID;
    
    const auto frameInfo = renderer->getFrameInfo();
    UnorderedUIDSet rebuildLayoutIDs(oldUniqueIDsByDrawable.size());
    for (const auto &drawID : oldDrawIDs)
    {
        // Find the unique ID that generated this old drawable, if any.
        const auto oldUIDMatch = oldUniqueIDsByDrawable.find(drawID);
        if (oldUIDMatch != oldUniqueIDsByDrawable.end() && !oldUIDMatch->second->empty())
        {
            // Now see if that unique ID also generated any drawables in this round.
            const auto newUIDMatch = newUniqueDrawableMap.find(*oldUIDMatch->second);
            bool hasNewDrawables = (newUIDMatch != newUniqueDrawableMap.end() && newUIDMatch->second.empty());

            bool wasOn = true;
            if (hasNewDrawables && checkDrawableOn && frameInfo)
            {
                // See if the old one was actually on. If not, we don't need to fade it out.
                if (const auto oldDraw = scene->getDrawable(drawID))
                {
                    wasOn = oldDraw->isOn(frameInfo.get());
                }
            }

            if (wasOn && hasNewDrawables && frameInfo)
            {
                if (newDrawsByID.empty())
                {
                    for (const auto &draw : newDrawables)
                    {
                        newDrawsByID.insert(std::make_pair(draw->getId(), draw));
                    }
                }

                if (checkDrawableOn)
                {
                    // Yes, it did generate drawables.  But if they're "off" they don't count.
                    // Again, assume multiple drawables are in the same state.
                    const auto hit = newDrawsByID.find(*newUIDMatch->second.begin());
                    if (hit != newDrawsByID.end() && !hit->second->isOn(frameInfo.get()))
                    {
                        hasNewDrawables = false;
                    }
                }
            }

            if (wasOn && !hasNewDrawables)
            {
                // It wasn't around in the previous frame, so fade it out.  Since some of
                // the fading and non-fading objects may have shared drawables in the previous
                // round, we need to re-construct just those for things we want to fade.
                rebuildLayoutIDs.insert(*oldUIDMatch->second);
            }
        }
    }

    // Are there items we need to rebuild to fade out?
    if (!rebuildLayoutIDs.empty() && renderer && !cancelLayout)
    {
        auto *coordAdapter = scene->getCoordAdapter();

        // Copy layout items to the new set if their ID is a match
        LayoutEntrySet rebuildLayoutObjs;
        for (const auto &entry : prevLayoutObjects)
        {
            const auto &uid = entry->obj.uniqueID;
            if (!uid.empty() && rebuildLayoutIDs.find(uid) != rebuildLayoutIDs.end())
            {
                rebuildLayoutObjs.insert(entry);
            }
        }

        // Build drawables for them...
        ScreenSpaceBuilder ssBuild(renderer,coordAdapter,renderer->getScale());
        buildDrawables(ssBuild, /*doFades*/false, /*doClusters=*/false, curTime, nullptr,
                       rebuildLayoutObjs, oldClusters, oldClusterParams, nullptr, nullptr);
        const auto newDraws = ssBuild.flushChanges(changes);

        // ... and then remove them
        const auto fade = curTime + oldObjectFadeOut;
        for (auto &draw : newDraws)
        {
            const auto drawID = draw->getId();
            changes.push_back(new FadeChangeRequest(drawID, curTime, fade));
            changes.push_back(new RemDrawableReq(drawID, fade));
        }
        // Don't run again before the fades are complete
        maxAnimTime = std::max(maxAnimTime, fade);
    }
}

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
        else if (auto theProgram = scene->findProgramByName(MaplyNoBackfaceLineShader))
        {
            vecProgID = theProgram->getId();
        }
    }

    std::unique_lock<std::mutex> extLock(lock);

    if (cancelLayout || shutdown)
    {
        return;
    }
    
    if (scene->getCurrentTime() < minLayoutTime.load(std::memory_order_relaxed))
    {
        // Animations/fades from previous layouts are still running.
        return;
    }

    // Make local copies of the layout objects
    LayoutEntrySet localLayoutObjects(layoutObjects.begin(), layoutObjects.end());
    const std::unordered_set<std::string> localOverrideUUIDs(overrideUUIDs.begin(), overrideUUIDs.end(), overrideUUIDs.size());

    // Any changes made after this will require another round of layout
    hasUpdates = false;
    const bool hadRemoves = hasRemoves;
    hasRemoves = false;

    // Release the external lock to allow objects to be added and removed while we're doing the
    // layout on our copies, and replace it with a separate lock to make sure we're only run once.
    // If we can't acquire the internal mutex instantly, we're already running on another thread.
    extLock.unlock();
    std::unique_lock<std::timed_mutex> intLock(internalLock, std::chrono::seconds(0));

    // If we couldn't acquire the lock, layout is already running on another thread.
    // Bail out immediately rather than waiting.  Even if it finished immediately, we wouldn't
    // want to run a layout pass on the now-obsolete copy of the layout items we have.
    if (!intLock.owns_lock())
    {
        wkLogLevel(Warn, "Layout called on multiple threads");
        return;
    }

    // Locking may have taken some time, check for cancellation again
    if (cancelLayout)
    {
        cancelLayout = false;
        return;
    }

    // Clear out any debug outlines we accumulated on the previous update
    if (!debugVecIDs.empty())
    {
        vecManage->removeVectors(debugVecIDs, changes);
        debugVecIDs.clear();
    }

    const std::vector<ClusterEntry> oldClusters = std::move(clusters);
    const std::vector<ClusterGenerator::ClusterClassParams> oldClusterParams = std::move(clusterParams);

    // This will recalculate the offsets and enables
    // If there were any changes, we need to regenerate
    bool layoutChanges = runLayoutRules(threadInfo, viewState,
                                        localLayoutObjects, localOverrideUUIDs,
                                        clusters,clusterParams,changes);

    // Note: check for cancellation before accessing `clusterGen`.
    // If shutdown has timed out, it will be invalid.
    if (cancelLayout)
    {
        cancelLayout = false;
        return;
    }

    // Compare old and new clusters
    if (!layoutChanges && clusters.size() != oldClusters.size())
    {
        layoutChanges = true;
    }

    if (auto clusterGen = clusterGenerator)
    if (clusterGen->hasChanges())
    {
        layoutChanges = true;
    }

    if (!layoutChanges && !hadRemoves)
    {
        return;
    }

//    if (layoutChanges)
//        NSLog(@"LayoutChanges");

    const TimeInterval curTime = scene->getCurrentTime();
    TimeInterval maxAnimTime = 0.0;

    // Save the drawable mapping from the previous iteration
    const auto oldUniqueDrawableMap = std::move(uniqueDrawableIDs);
    uniqueDrawableIDs.clear();

    // Generate the drawables.
    // Note that the renderer is not managed by a shared pointer, and will be destroyed
    // during shutdown, so we must stop using it quickly if controller shutdown is initiated.
    ScreenSpaceBuilder ssBuild(renderer,coordAdapter,renderer->getScale());

    //wkLog("Starting Layout t=%f", curTime);

    buildDrawables(ssBuild, fadeEnabled, /*doClusters=*/true, curTime, &maxAnimTime,
                   localLayoutObjects, oldClusters, oldClusterParams,
                   &uniqueDrawableIDs, &oldUniqueDrawableMap);

    if (cancelLayout)
    {
        cancelLayout = false;
        return;
    }

    // Add the new ones
    SimpleIDSet newDrawIDs;
    const auto newDraws = ssBuild.flushChanges(changes, newDrawIDs);

//        NSLog(@"Got %lu clusters",clusters.size());

    // Get rid of the last set of drawables
    for (const auto &drawID : drawIDs)
    {
        changes.push_back(new RemDrawableReq(drawID));
    }

    handleFadeOut(curTime, maxAnimTime, localLayoutObjects, drawIDs, newDraws,
                  oldClusters, oldClusterParams, oldUniqueDrawableMap, uniqueDrawableIDs, changes);

    drawIDs.clear();
    drawIDs.swap(newDrawIDs);

    prevLayoutObjects.swap(localLayoutObjects);

    // That all may have taken a while, so update some the times for animation.
    // Also add a jiffy for finishing up here and actually processing the change
    // requests, at least until we have a way to specify times in relative terms.
    constexpr auto fadeTimeFudge = 0.02;
    const auto newTime = scene->getCurrentTime() + fadeTimeFudge;

    // If we took more than a few milliseconds, bump the time values to
    // match, since they won't start until the changes are processed.
    const auto deltaT = newTime - curTime;
    if (deltaT > 0.01)
    {
        for (auto &draw : newDraws)
        {
            if (const auto dp = draw.get())
            {
                if (dp->fadeUp > 0.0) dp->fadeUp += deltaT;
                if (dp->fadeDown > 0.0) dp->fadeDown += deltaT;
                if (dp->startEnable > 0.0) dp->startEnable += deltaT;
                if (dp->endEnable > 0.0) dp->endEnable += deltaT;
            }
        }
        for (auto &change : changes)
        {
            if (change->when > 0.0) change->when += deltaT;
        }
    }

    // If we set up animations, don't run another layout pass until they're complete
    if (maxAnimTime > 0.0)
    {
        deferUntil(maxAnimTime + deltaT);
    }

    wkLogLevel(Verbose, "Layout of %d objects, %d clusters took %.4f s",
               localLayoutObjects.size(), clusters.size(), scene->getCurrentTime() - curTime);
}

}
