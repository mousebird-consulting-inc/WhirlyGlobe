/*
 *  LayoutManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/15/13.
 *  Copyright 2011-2013 mousebird consulting. All rights reserved.
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

#import "LayoutManager.h"
#import "SceneRendererES2.h"
#import "WhirlyGeometry.h"
#import "GlobeMath.h"
#import "ScreenSpaceGenerator.h"

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
    bool addObject(const std::vector<Point2f> &pts)
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
        int newId = objects.size()-1;
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
        std::vector<Point2f> pts;
    };
    
    Mbr mbr;
    std::vector<BoundedObject> objects;
    int sizeX,sizeY;
    Point2f cellSize;
    std::vector<std::vector<int> > grid;
};

// Default constructor for layout object
LayoutObject::LayoutObject()
    : Identifiable(),
        enable(true), dispLoc(0,0,0), size(0,0), iconSize(0,0), rotation(0.0), minVis(DrawVisibleInvalid),
        maxVis(DrawVisibleInvalid), importance(MAXFLOAT), acceptablePlacement(WhirlyKitLayoutPlacementLeft | WhirlyKitLayoutPlacementRight | WhirlyKitLayoutPlacementAbove | WhirlyKitLayoutPlacementBelow)
{
}    
    
LayoutObject::LayoutObject(SimpleIdentity theId) : Identifiable(theId),
    enable(true), dispLoc(0,0,0), size(0,0), iconSize(0,0), rotation(0.0), minVis(DrawVisibleInvalid),
    maxVis(DrawVisibleInvalid), importance(MAXFLOAT), acceptablePlacement(WhirlyKitLayoutPlacementLeft | WhirlyKitLayoutPlacementRight | WhirlyKitLayoutPlacementAbove | WhirlyKitLayoutPlacementBelow)
{
    
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
    
// Size of the overlap sampler
static const int OverlapSampleX = 10;
static const int OverlapSampleY = 60;

// Now much around the screen we'll take into account
static const float ScreenBuffer = 0.1;
    
// Do the actual layout logic.  We'll modify the offset and on value in place.
void LayoutManager::runLayoutRules(WhirlyKitViewState *viewState)
{
    if (layoutObjects.empty())
        return;
    
    LayoutSortingSet layoutObjs;
    
    // Turn everything off and sort by importance
    WhirlyGlobeViewState *globeViewState = nil;
    if ([viewState isKindOfClass:[WhirlyGlobeViewState class]])
        globeViewState = (WhirlyGlobeViewState *)viewState;
    for (LayoutEntrySet::iterator it = layoutObjects.begin();
         it != layoutObjects.end(); ++it)
    {
        if ((*it)->obj.enable)
        {
            LayoutObjectEntry *obj = *it;
            bool use = false;
            if (globeViewState)
            {
                if (obj->obj.minVis == DrawVisibleInvalid || obj->obj.maxVis == DrawVisibleInvalid ||
                    (obj->obj.minVis < globeViewState.heightAboveGlobe && globeViewState.heightAboveGlobe < obj->obj.maxVis))
                    use = true;
            } else
                use = true;
            if (use)
                layoutObjs.insert(*it);
        }
    }
    
//    NSLog(@"----Starting Layout----");
    
    // Need to scale for retina displays
    float resScale = renderer.scale;
    
    // Set up the overlap sampler
    Point2f frameBufferSize;
    frameBufferSize.x() = renderer.framebufferWidth;
    frameBufferSize.y() = renderer.framebufferHeight;
    Mbr screenMbr(Point2f(-ScreenBuffer * frameBufferSize.x(),-ScreenBuffer * frameBufferSize.y()),frameBufferSize * (1.0 + ScreenBuffer));
    OverlapManager overlapMan(screenMbr,OverlapSampleX,OverlapSampleY);
    
    Matrix4d modelTrans = viewState.fullMatrix;
    Matrix4f fullMatrix4f = Matrix4dToMatrix4f(viewState.fullMatrix);
    Matrix4f fullNormalMatrix4f = Matrix4dToMatrix4f(viewState.fullNormalMatrix);
    int numSoFar = 0;
    for (LayoutSortingSet::iterator it = layoutObjs.begin();
         it != layoutObjs.end(); ++it)
    {
        LayoutObjectEntry *layoutObj = *it;
        
        // Start with a max objects check
        bool isActive = true;
        if (maxDisplayObjects != 0 && (numSoFar >= maxDisplayObjects))
            isActive = false;
        // Start with a back face check
        if (isActive && globeViewState)
        {
            // Make sure this one is facing toward the viewer
            isActive = CheckPointAndNormFacing(layoutObj->obj.dispLoc,layoutObj->obj.dispLoc.normalized(),fullMatrix4f,fullNormalMatrix4f) > 0.0;
        }
        Point2f objOffset(0.0,0.0);
        
        // Figure out the rotation situation
        float screenRot = 0.0;
        Matrix2f screenRotMat;
        if (isActive)
        {
            // Figure out where this will land
            CGPoint objPt = [viewState pointOnScreenFromDisplay:Vector3fToVector3d(layoutObj->obj.dispLoc) transform:&modelTrans frameSize:frameBufferSize];
            isActive = screenMbr.inside(Point2f(objPt.x,objPt.y));
            
            // Deal with the rotation
            if (layoutObj->obj.rotation != 0.0)
            {
                Point3d norm,right,up;
                
                if (globeViewState)
                {
                    Point3d simpleUp(0,0,1);
                    norm = Vector3fToVector3d(layoutObj->obj.dispLoc);
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
                
                Point3d outPt = rightDir * 1.0 + upDir * 1.0 + Vector3fToVector3d(layoutObj->obj.dispLoc);
                CGPoint outScreenPt;
                outScreenPt = [viewState pointOnScreenFromDisplay:outPt transform:&modelTrans frameSize:frameBufferSize];
                screenRot = M_PI/2.0-atan2f(objPt.y-outScreenPt.y,outScreenPt.x-objPt.x);
                screenRotMat = Eigen::Rotation2Df(screenRot);
            }
            
            // Now for the overlap checks
            if (isActive)
            {
                // Try the four diffierent orientations
                if (layoutObj->obj.size.x() != 0.0 && layoutObj->obj.size.y() != 0.0)
                {
                    bool validOrient = false;
                    for (unsigned int orient=0;orient<4;orient++)
                    {
                        // May only want to be placed certain ways.  Fair enough.
                        if (!(layoutObj->obj.acceptablePlacement & (1<<orient)))
                            continue;
                        
                        // Set up the offset for this orientation
                        switch (orient)
                        {
                                // Right
                            case 0:
                                objOffset = Point2f(layoutObj->obj.iconSize.x(),0.0);
                                break;
                                // Left
                            case 1:
                                objOffset = Point2f(-(layoutObj->obj.size.x()+layoutObj->obj.iconSize.x()/2.0),0.0);
                                break;
                                // Above
                            case 2:
                                objOffset = Point2f(-layoutObj->obj.size.x()/2.0,-(layoutObj->obj.size.y()+layoutObj->obj.iconSize.y())/2.0);
                                break;
                                // Below
                            case 3:
                                objOffset = Point2f(-layoutObj->obj.size.x()/2.0,(layoutObj->obj.size.y()+layoutObj->obj.iconSize.y())/2.0);
                                break;
                        }
                        
                        // Rotate the rectangle
                        std::vector<Point2f> objPts(4);
                        if (screenRot == 0.0)
                        {
                            objPts[0] = Point2f(objPt.x,objPt.y) + objOffset*resScale;
                            objPts[1] = objPts[0] + Point2f(layoutObj->obj.size.x()*resScale,0.0);
                            objPts[2] = objPts[0] + Point2f(layoutObj->obj.size.x()*resScale,layoutObj->obj.size.y()*resScale);
                            objPts[3] = objPts[0] + Point2f(0.0,layoutObj->obj.size.y()*resScale);
                        } else {
                            objPts[0] = Point2f(objPt.x,objPt.y) + objOffset*resScale;
                            objPts[1] = objPts[0] + screenRotMat * Point2f(layoutObj->obj.size.x()*resScale,0.0);
                            objPts[2] = objPts[0] + screenRotMat * Point2f(layoutObj->obj.size.x()*resScale,layoutObj->obj.size.y()*resScale);
                            objPts[3] = objPts[0] + screenRotMat * Point2f(0.0,layoutObj->obj.size.y()*resScale);
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
            layoutObj->changed = true;
        layoutObj->newEnable = isActive;
        layoutObj->offset = objOffset;
    }
    
//    NSLog(@"----Finished layout----");
}

// Time we'll take to disappear objects
static float const DisappearFade = 0.1;

// Layout all the objects we're tracking
void LayoutManager::updateLayout(WhirlyKitViewState *viewState,ChangeSet &changes)
{
    pthread_mutex_lock(&layoutLock);

    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
    
    // This will recalulate the offsets and enables
    runLayoutRules(viewState);
    
    std::vector<ScreenSpaceGeneratorGangChangeRequest::ShapeChange> shapeChanges;
    changes.reserve(layoutObjects.size());
    
    for (LayoutEntrySet::iterator it = layoutObjects.begin();
         it != layoutObjects.end(); ++it)
    {
        LayoutObjectEntry *layoutObj = *it;
        if (layoutObj->changed)
        {
            // Put in the change for the main object
            ScreenSpaceGeneratorGangChangeRequest::ShapeChange change;
            change.shapeID = layoutObj->obj.getId();
            if (layoutObj->newEnable)
                change.offset = layoutObj->offset;
            else
                // This is a stealth enable
                change.offset = Point2f(MAXFLOAT,MAXFLOAT);
            // Fade in when we add them
            if (!layoutObj->currentEnable)
            {
                change.fadeDown = curTime;
                change.fadeUp = curTime+DisappearFade;
            }
            layoutObj->currentEnable = layoutObj->newEnable;
            shapeChanges.push_back(change);
            
            // And auxiliary objects
            for (SimpleIDSet::iterator sit = layoutObj->obj.auxIDs.begin();
                 sit != layoutObj->obj.auxIDs.end(); ++sit)
            {
                ScreenSpaceGeneratorGangChangeRequest::ShapeChange change;
                change.shapeID = *sit;
                if (!layoutObj->currentEnable)
                    change.offset = Point2f(MAXFLOAT,MAXFLOAT);
                shapeChanges.push_back(change);
            }
            
            layoutObj->changed = false;
        }
    }
    
    changes.push_back(new ScreenSpaceGeneratorGangChangeRequest(scene->getScreenSpaceGeneratorID(),shapeChanges));
    hasUpdates = false;
    
    pthread_mutex_unlock(&layoutLock);
}
    
}
