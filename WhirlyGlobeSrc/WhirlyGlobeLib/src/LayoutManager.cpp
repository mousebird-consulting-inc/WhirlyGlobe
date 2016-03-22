/*
 *  LayoutManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/15/13.
 *  Copyright 2011-2016 mousebird consulting. All rights reserved.
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

#import "Platform.h"
#import "LayoutManager.h"
#import "SceneRendererES2.h"
#import "WhirlyGeometry.h"
#import "GlobeMath.h"
#import "GlobeViewState.h"
#import "MaplyViewState.h"

using namespace Eigen;

namespace WhirlyKit
{

// We use this to avoid overlapping labels
class OverlapManager
{
public:
    OverlapManager(const Mbr &mbr,int sizeX,int sizeY)
    : mbr(mbr), sizeX(sizeX), sizeY(sizeY)
    {
        grid.resize(sizeX*sizeY);
        cellSize = Point2f((mbr.ur().x()-mbr.ll().x())/sizeX,(mbr.ur().y()-mbr.ll().y())/sizeY);
    }
    
    // Try to add an object.  Might fail (kind of the whole point).
    bool addObject(const Point2dVector &pts)
    {
        Mbr objMbr;
        for (unsigned int ii=0;ii<pts.size();ii++)
            objMbr.addPoint(pts[ii]);
        int sx = floorf((objMbr.ll().x()-mbr.ll().x())/cellSize.x());
        if (sx < 0) sx = 0;
        int sy = floorf((objMbr.ll().y()-mbr.ll().y())/cellSize.y());
        if (sy < 0) sy = 0;
        int ex = ceilf((objMbr.ur().x()-mbr.ll().x())/cellSize.x());
        if (ex >= sizeX)  ex = sizeX-1;
        int ey = ceilf((objMbr.ur().y()-mbr.ll().y())/cellSize.y());
        if (ey >= sizeY)  ey = sizeY-1;
        for (int ix=sx;ix<=ex;ix++)
            for (int iy=sy;iy<=ey;iy++)
            {
                std::vector<int> &objList = grid[iy*sizeX + ix];
                for (unsigned int ii=0;ii<objList.size();ii++)
                {
                    BoundedObject &testObj = objects[objList[ii]];
                    // Note: This will result in testing the same thing multiple times
                    if (ConvexPolyIntersect(testObj.pts,pts))
                        return false;
                }
            }
        
        // Okay, so it doesn't overlap.  Let's add it where needed.
        objects.resize(objects.size()+1);
        int newId = (int)(objects.size()-1);
        BoundedObject &newObj = objects[newId];
        newObj.pts = pts;
        for (int ix=sx;ix<=ex;ix++)
            for (int iy=sy;iy<=ey;iy++)
            {
                std::vector<int> &objList = grid[iy*sizeX + ix];
                objList.push_back(newId);
            }
        
        return true;
    }
    
protected:
    // Object and its bounds
    class BoundedObject
    {
    public:
        ~BoundedObject() { }
        Point2dVector pts;
    };
    
    Mbr mbr;
    std::vector<BoundedObject> objects;
    int sizeX,sizeY;
    Point2f cellSize;
    std::vector<std::vector<int> > grid;
};

// Default constructor for layout object
LayoutObject::LayoutObject()
    : ScreenSpaceObject(), importance(MAXFLOAT), acceptablePlacement(WhirlyKitLayoutPlacementLeft | WhirlyKitLayoutPlacementRight | WhirlyKitLayoutPlacementAbove | WhirlyKitLayoutPlacementBelow)
{
}
    
LayoutObject::LayoutObject(SimpleIdentity theId) : ScreenSpaceObject(theId),
     importance(MAXFLOAT), acceptablePlacement(WhirlyKitLayoutPlacementLeft | WhirlyKitLayoutPlacementRight | WhirlyKitLayoutPlacementAbove | WhirlyKitLayoutPlacementBelow)
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
    
LayoutManager::LayoutManager()
    : maxDisplayObjects(0), hasUpdates(false)
{
    pthread_mutex_init(&layoutLock, NULL);
}
    
LayoutManager::~LayoutManager()
{
    for (LayoutEntrySet::iterator it = layoutObjects.begin();
         it != layoutObjects.end(); ++it)
        delete *it;
    layoutObjects.clear();
    
    pthread_mutex_destroy(&layoutLock);
}
    
void LayoutManager::setMaxDisplayObjects(int numObjects)
{
    maxDisplayObjects = numObjects;
}
    
void LayoutManager::addLayoutObjects(const std::vector<LayoutObject> &newObjects)
{
    for (unsigned int ii=0;ii<newObjects.size();ii++)
    {
        const LayoutObject &layoutObj = newObjects[ii];
        LayoutObjectEntry *entry = new LayoutObjectEntry(layoutObj.getId());
        entry->obj = newObjects[ii];
        layoutObjects.insert(entry);
    }
    hasUpdates = true;
}
    
    
void LayoutManager::addLayoutObjects(const std::vector<LayoutObject *> &newObjects)
{
    for (unsigned int ii=0;ii<newObjects.size();ii++)
    {
        const LayoutObject *layoutObj = newObjects[ii];
        LayoutObjectEntry *entry = new LayoutObjectEntry(layoutObj->getId());
        entry->obj = *(newObjects[ii]);
        layoutObjects.insert(entry);
    }
    hasUpdates = true;
}
    
/// Enable/disable layout objects
void LayoutManager::enableLayoutObjects(const SimpleIDSet &theObjects,bool enable)
{
    for (SimpleIDSet::const_iterator it = theObjects.begin();
         it != theObjects.end(); ++it)
    {
        LayoutObjectEntry entry(*it);
        LayoutEntrySet::iterator eit = layoutObjects.find(&entry);
        if (eit != layoutObjects.end())
            (*eit)->obj.enable = enable;
    }
    hasUpdates = true;    
}
    
void LayoutManager::removeLayoutObjects(const SimpleIDSet &oldObjects)
{
    for (SimpleIDSet::const_iterator it = oldObjects.begin();
         it != oldObjects.end(); ++it)
    {
        LayoutObjectEntry entry(*it);
        LayoutEntrySet::iterator eit = layoutObjects.find(&entry);
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
    bool ret = false;
    
    pthread_mutex_lock(&layoutLock);
    
    ret = hasUpdates;
    
    pthread_mutex_unlock(&layoutLock);
    
    return ret;
}
    
// Sort more important things to the front
typedef struct
{
    bool operator () (const LayoutObjectEntry *a,const LayoutObjectEntry *b)
    {
        if (a->obj.importance == b->obj.importance)
            return a > b;
        return a->obj.importance > b->obj.importance;
    }
} LayoutEntrySorter;
typedef std::set<LayoutObjectEntry *,LayoutEntrySorter> LayoutSortingSet;
    
    
// Return the screen space objects in a form the selection manager can understand
void LayoutManager::getScreenSpaceObjects(const SelectionManager::PlacementInfo &pInfo,std::vector<ScreenSpaceObjectLocation> &screenSpaceObjs)
{
    pthread_mutex_lock(&layoutLock);
    
    for (LayoutEntrySet::iterator it = layoutObjects.begin();
         it != layoutObjects.end(); ++it)
    {
        LayoutObjectEntry *entry = *it;
        if (entry->currentEnable)
        {
            ScreenSpaceObjectLocation ssObj;
            ssObj.shapeID = entry->obj.getId();
            ssObj.dispLoc = entry->obj.worldLoc;
            ssObj.offset = entry->offset;
            ssObj.pts = entry->obj.selectPts;
            for (unsigned int ii=0;ii<entry->obj.selectPts.size();ii++)
                ssObj.mbr.addPoint(entry->obj.selectPts[ii]);
            
            screenSpaceObjs.push_back(ssObj);
        }
    }
    
    pthread_mutex_unlock(&layoutLock);
}
    
// Size of the overlap sampler
static const int OverlapSampleX = 10;
static const int OverlapSampleY = 60;

// Now much around the screen we'll take into account
static const float ScreenBuffer = 0.1;
    
// Do the actual layout logic.  We'll modify the offset and on value in place.
bool LayoutManager::runLayoutRules(ViewState *viewState)
{
    if (layoutObjects.empty())
        return false;
    
    bool hadChanges = false;
    
    LayoutSortingSet layoutObjs;
    
    // Turn everything off and sort by importance
    WhirlyGlobe::GlobeViewState *globeViewState = dynamic_cast<WhirlyGlobe::GlobeViewState *>(viewState);
    for (LayoutEntrySet::iterator it = layoutObjects.begin();
         it != layoutObjects.end(); ++it)
    {
        if ((*it)->obj.enable)
        {
            LayoutObjectEntry *obj = *it;
            bool use = false;
            if (globeViewState)
            {
                if (obj->obj.state.minVis == DrawVisibleInvalid || obj->obj.state.maxVis == DrawVisibleInvalid ||
                    (obj->obj.state.minVis < globeViewState->heightAboveGlobe && globeViewState->heightAboveGlobe < obj->obj.state.maxVis))
                    use = true;
            } else
                use = true;
            if (use)
                layoutObjs.insert(*it);
            if ((use && !obj->currentEnable) || (!use && obj->currentEnable))
                hadChanges = true;
        }
    }
    
//    NSLog(@"----Starting Layout----");
    
    // Need to scale for retina displays
    float resScale = renderer->getScale();
    
    // Set up the overlap sampler
    Point2f frameBufferSize = renderer->getFramebufferSize();
    Mbr screenMbr(Point2f(-ScreenBuffer * frameBufferSize.x(),-ScreenBuffer * frameBufferSize.y()),frameBufferSize * (1.0 + ScreenBuffer));
    OverlapManager overlapMan(screenMbr,OverlapSampleX,OverlapSampleY);
    
    Matrix4d modelTrans = viewState->fullMatrices[0];
    Matrix4f fullMatrix4f = Matrix4dToMatrix4f(viewState->fullMatrices[0]);
    Matrix4f fullNormalMatrix4f = Matrix4dToMatrix4f(viewState->fullNormalMatrices[0]);
    int numSoFar = 0;
    bool isActive;
    Point2d objOffset(0.0,0.0);
    float screenRot;
    LayoutObjectEntry *layoutObj;
    Point2dVector objPts(4);

    for (LayoutSortingSet::iterator it = layoutObjs.begin();
         it != layoutObjs.end(); ++it)
    {
        layoutObj = *it;
        
        // Start with a max objects check
        isActive = true;
        if (maxDisplayObjects != 0 && (numSoFar >= maxDisplayObjects))
            isActive = false;
        // Start with a back face check
        if (isActive && globeViewState)
        {
            // Make sure this one is facing toward the viewer
            isActive = CheckPointAndNormFacing(Vector3dToVector3f(layoutObj->obj.worldLoc),Vector3dToVector3f(layoutObj->obj.worldLoc.normalized()),fullMatrix4f,fullNormalMatrix4f) > 0.0;
        }
        
        // Figure out the rotation situation
        screenRot = 0.0;
        Matrix2d screenRotMat;
        if (isActive)
        {
            // Figure out where this will land
            bool isInside = false;
            Point2d objPt;
            for (unsigned int offi=0;offi<viewState->viewMatrices.size();offi++)
            {
                Eigen::Matrix4d modelTrans = viewState->fullMatrices[offi];
                Point2f thisObjPt = viewState->pointOnScreenFromDisplay(layoutObj->obj.worldLoc,&modelTrans,frameBufferSize);
                if (screenMbr.inside(Point2f(thisObjPt.x(),thisObjPt.y())))
                {
                    isInside = true;
                    objPt = Vector2fToVector2d(thisObjPt);
                }
            }
            isActive &= isInside;
            
            // Deal with the rotation
            if (layoutObj->obj.rotation != 0.0)
            {
                Point3d norm,right,up;
                
                if (globeViewState)
                {
                    Point3d simpleUp(0,0,1);
                    norm = layoutObj->obj.worldLoc;
                    norm.normalize();
                    right = simpleUp.cross(norm);
                    up = norm.cross(right);
                    right.normalize();
                    up.normalize();
                } else {
                    right = Point3d(1,0,0);
                    norm = Point3d(0,0,1);
                    up = Point3d(0,1,0);
                }
                // Note: Check if the axes made any sense.  We might be at a pole.
                Point3d rightDir = right * sinf(layoutObj->obj.rotation);
                Point3d upDir = up * cosf(layoutObj->obj.rotation);
                
                Point3d outPt = rightDir * 1.0 + upDir * 1.0 + layoutObj->obj.worldLoc;
                Point2f outScreenPt;
                outScreenPt = viewState->pointOnScreenFromDisplay(outPt,&modelTrans,frameBufferSize);
                screenRot = M_PI/2.0-atan2f(objPt.y()-outScreenPt.y(),outScreenPt.x()-objPt.x());
                // Keep the labels upright
                if (layoutObj->obj.keepUpright)
                    if (screenRot > M_PI/2 && screenRot < 3*M_PI/2)
                        screenRot = screenRot + M_PI;
                screenRotMat = Eigen::Rotation2Dd(screenRot);
            }
            
            // Now for the overlap checks
            if (isActive)
            {
                // Try the four different orientations
                if (!layoutObj->obj.layoutPts.empty())
                {
                    bool validOrient = false;
                    for (unsigned int orient=0;orient<5;orient++)
                    {
                        // May only want to be placed certain ways.  Fair enough.
                        if (!(layoutObj->obj.acceptablePlacement & (1<<orient)))
                            continue;
                        const Point2dVector &layoutPts = layoutObj->obj.layoutPts;
                        Mbr layoutMbr;
                        for (unsigned int li=0;li<layoutPts.size();li++)
                            layoutMbr.addPoint(layoutPts[li]);
                        Point2f layoutSpan(layoutMbr.ur().x()-layoutMbr.ll().x(),layoutMbr.ur().y()-layoutMbr.ll().y());
                        
                        // Set up the offset for this orientation
                        // Note: This is all wrong for markers now
                        switch (orient)
                        {
                                //center
                            case 0:
                                objOffset = Point2d(-layoutSpan.x()/2.0,layoutSpan.y()/2.0);
                                break;
                                // Right
                            case 1:
                                objOffset = Point2d(0.0,layoutSpan.y()/2.0);
                                break;
                                // Left
                            case 2:
                                objOffset = Point2d(-(layoutSpan.x()),layoutSpan.y()/2.0);
                                break;
                                // Above
                            case 3:
                                objOffset = Point2d(-layoutSpan.x()/2.0,0);
                                break;
                                // Below
                            case 4:
                                objOffset = Point2d(-layoutSpan.x()/2.0,layoutSpan.y());
                                break;
                        }
                        
                        // Rotate the rectangle
                        if (screenRot == 0.0)
                        {
                            objPts[0] = Point2d(objPt.x(),objPt.y()) + objOffset*resScale;
                            objPts[1] = objPts[0] + Point2d(layoutSpan.x()*resScale,0.0);
                            objPts[2] = objPts[0] + Point2d(layoutSpan.x()*resScale,layoutSpan.y()*resScale);
                            objPts[3] = objPts[0] + Point2d(0.0,layoutSpan.y()*resScale);
                        } else {
                            Point2d center(objPt.x(),objPt.y());
                            objPts[0] = objOffset;
                            objPts[1] = objOffset + Point2d(layoutSpan.x(),0.0);
                            objPts[2] = objOffset + Point2d(layoutSpan.x(),layoutSpan.y());
                            objPts[3] = objOffset + Point2d(0.0,layoutSpan.y());
                            for (unsigned int oi=0;oi<4;oi++)
                            {
                                Point2d &thisObjPt = objPts[oi];
                                thisObjPt = screenRotMat * thisObjPt;
                                thisObjPt = Point2d(thisObjPt.x()*resScale,thisObjPt.y()*resScale)+center;
                            }
                        }
                        
                        // Now try it
                        if (overlapMan.addObject(objPts))
                        {
                            validOrient = true;
                            break;
                        }
                    }
                    
                    isActive = validOrient;
                }
            }

//            NSLog(@" Valid (%s): %@, pos = (%f,%f), size = (%f, %f), offset = (%f,%f)",(isActive ? "yes" : "no"),layoutObj->obj.hint,objPt.x,objPt.y,layoutObj->obj.size.x(),layoutObj->obj.size.y(),
//                  layoutObj->offset.x(),layoutObj->offset.y());
        }
        
        if (isActive)
            numSoFar++;
        
        // See if we've changed any of the state
        layoutObj->changed = (layoutObj->currentEnable != isActive);
        if (!layoutObj->changed && layoutObj->newEnable &&
            (layoutObj->offset.x() != objOffset.x() || layoutObj->offset.y() != objOffset.y()))
        {
            layoutObj->changed = true;
        }
        hadChanges |= layoutObj->changed;
        layoutObj->newEnable = isActive;
        layoutObj->offset = objOffset;
    }
    
//    NSLog(@"----Finished layout----");
    
    return hadChanges;
}

// Time we'll take to disappear objects
static float const DisappearFade = 0.1;

// Layout all the objects we're tracking
void LayoutManager::updateLayout(WhirlyKit::ViewState *viewState,ChangeSet &changes)
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    
    pthread_mutex_lock(&layoutLock);

    TimeInterval curTime = TimeGetCurrent();
    
    // This will recalculate the offsets and enables
    // If there were any changes, we need to regenerate
    bool layoutChanges = runLayoutRules(viewState);
    if (hasUpdates || layoutChanges)
    {
        // Get rid of the last set of drawables
        for (SimpleIDSet::iterator it = drawIDs.begin(); it != drawIDs.end(); ++it)
            changes.push_back(new RemDrawableReq(*it));
        drawIDs.clear();

        // Generate the drawables
        ScreenSpaceBuilder ssBuild(coordAdapter,renderer->getScale());
        for (LayoutEntrySet::iterator it = layoutObjects.begin();
             it != layoutObjects.end(); ++it)
        {
            LayoutObjectEntry *layoutObj = *it;

            layoutObj->obj.offset = Point2d(layoutObj->offset.x(),-layoutObj->offset.y());
            if (!layoutObj->currentEnable)
            {
                layoutObj->obj.state.fadeDown = curTime;
                layoutObj->obj.state.fadeUp = curTime+DisappearFade;
            }
            layoutObj->currentEnable = layoutObj->newEnable;
            
            layoutObj->changed = false;
            
            if (layoutObj->currentEnable)
                ssBuild.addScreenObject(layoutObj->obj);
        }
        
        ssBuild.flushChanges(changes, drawIDs);
    }
    
    hasUpdates = false;
    
    pthread_mutex_unlock(&layoutLock);
}
    
}
