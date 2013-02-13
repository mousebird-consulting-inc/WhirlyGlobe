/*
 *  LayoutLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 12/4/12.
 *  Copyright 2011-2012 mousebird consulting. All rights reserved.
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

#import <map>
#import "LayoutLayer.h"
#import "GlobeLayerViewWatcher.h"
#import "ScreenSpaceGenerator.h"

using namespace Eigen;
using namespace WhirlyKit;

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
    bool addObject(const Mbr &objMbr,NSObject *obj)
    {
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
                    if (testObj.mbr.overlaps(objMbr))
                        return false;
                }
            }
        
        // Okay, so it doesn't overlap.  Let's added it where needed.
        objects.resize(objects.size()+1);
        int newId = objects.size()-1;
        BoundedObject &newObj = objects[newId];
        newObj.mbr = objMbr;
        newObj.obj = obj;
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
        Mbr mbr;
        NSObject *obj;
    };
    
    Mbr mbr;
    std::vector<BoundedObject> objects;
    int sizeX,sizeY;
    Point2f cellSize;
    std::vector<std::vector<int> > grid;
};

namespace WhirlyKit
{
typedef std::map<SimpleIdentity,WhirlyKitLayoutObject * __strong> LayoutObjectMap;
}

@implementation WhirlyKitLayoutObject
{
@public
    // Set if it's currently on
    bool currentEnable;
    // Set if it's going to be on
    bool newEnable;
    // The offset, as calculated
    WhirlyKit::Point2f offset;
    // Set if we changed something during evaluation
    bool changed;
}

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    currentEnable = newEnable = false;
    offset = Point2f(0.0,0.0);
    minVis = maxVis = DrawVisibleInvalid;
    changed = true;
    
    return self;
}

@end

@implementation WhirlyKitLayoutLayer
{
    // Layer thread we're on
    WhirlyKitLayerThread * __weak layerThread;
    // Scene we're updating
    Scene *scene;
    // Screen space generator we're sending data to
    SimpleIdentity ssGenId;
    // Set if we haven't moved for a while
    bool stopped;
    // Last view state we've seen
    WhirlyKitViewState *viewState;
    // Objects we're controlling the placement for
    LayoutObjectMap layoutObjects;
    // Used for sizing info
    WhirlyKitSceneRendererES *renderer;
}

@synthesize maxDisplayObjects;

- (id)initWithRenderer:(WhirlyKitSceneRendererES *)inRenderer
{
    self = [super init];
    if (!self)
        return nil;
    renderer = inRenderer;
    maxDisplayObjects = 0;
    
    return self;
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
    layerThread = inLayerThread;
    scene = inScene;
    ssGenId = scene->getScreenSpaceGeneratorID();
    
    // Get us view updates, but we'll filter them
    if (layerThread.viewWatcher)
        [layerThread.viewWatcher addWatcherTarget:self selector:@selector(viewUpdate:) minTime:0.0];
}

- (void)shutdown
{
    scene = NULL;
    layoutObjects.clear();
    if (layerThread.viewWatcher)
        [layerThread.viewWatcher removeWatcherTarget:self selector:@selector(viewUpdate:)];
}

// How long we'll wait to see if the user has stopped twitching
static const float DelayPeriod = 0.1;

// We're getting called for absolutely every update here
- (void)viewUpdate:(WhirlyKitViewState *)inViewState
{
    if (!scene)
        return;
    
    if (viewState && [viewState isKindOfClass:[WhirlyKitViewState class]] && [inViewState isSameAs:viewState])
        return;
    viewState = inViewState;
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(delayCheck) object:nil];
    
    if (stopped)
    {
//        NSLog(@"Started moving");
//        [self disableObjects];
        stopped = false;
    }
    
    // Set a timer to see if we've stopped in a bit
    [self performSelector:@selector(delayCheck) withObject:nil afterDelay:DelayPeriod];
}

// Called after some period to check if we've stopped moving
- (void)delayCheck
{
    if (!viewState)
        return;
    
    stopped = true;
//    NSLog(@"Stopped moving");
    
    [self updateLayout];
}

// Turn off all the objects we're tracking
- (void)disableObjects
{
//    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();

    std::vector<ScreenSpaceGeneratorGangChangeRequest::ShapeChange> changes;
    changes.reserve(layoutObjects.size());

    for (LayoutObjectMap::iterator it = layoutObjects.begin();
         it != layoutObjects.end(); ++it)
    {
        // Put in the change for the main object
        ScreenSpaceGeneratorGangChangeRequest::ShapeChange change;
        WhirlyKitLayoutObject *layoutObj = it->second;
        change.shapeID = layoutObj->ssID;
        // Note: Not fading yet
        change.fadeUp = 0.0;
        change.fadeDown = 0.0;
        change.enable = false;
        changes.push_back(change);
        
        // And auxiliary objects
        for (SimpleIDSet::iterator sit = layoutObj->auxIDs.begin();
             sit != layoutObj->auxIDs.end(); ++sit)
        {
            ScreenSpaceGeneratorGangChangeRequest::ShapeChange change;
            change.shapeID = *sit;
            change.enable = false;
            changes.push_back(change);
        }
    }
    
    scene->addChangeRequest(new ScreenSpaceGeneratorGangChangeRequest(ssGenId,changes));
}

// Sort more important things to the front
typedef struct
{
    bool operator () (const WhirlyKitLayoutObject *a,const WhirlyKitLayoutObject *b)
    {
        if (a->importance == b->importance)
            return a > b;
        return a->importance > b->importance;
    }
} WhirlyKitLayoutObjectSorter;
typedef std::set<WhirlyKitLayoutObject *,WhirlyKitLayoutObjectSorter> WhirlyKitLayoutObjectSet;

// Size of the overlap sampler
static const int OverlapSampleX = 10;
static const int OverlapSampleY = 60;

// Now much around the screen we'll take into account
static const float ScreenBuffer = 0.1;

// Do the actual layout logic.  We'll modify the offset and on value in place.
- (void)runLayoutRules
{
    if (layoutObjects.empty())
        return;
    
    WhirlyKitLayoutObjectSet layoutObjs;

    // Turn everything off and sort by importance
    WhirlyGlobeViewState *globeViewState = nil;
    if ([viewState isKindOfClass:[WhirlyGlobeViewState class]])
        globeViewState = (WhirlyGlobeViewState *)viewState;
    for (LayoutObjectMap::iterator it = layoutObjects.begin();
         it != layoutObjects.end(); ++it)
    {
        WhirlyKitLayoutObject *obj = it->second;
        bool use = false;
        if (globeViewState)
        {
            if (obj->minVis == DrawVisibleInvalid || obj->maxVis == DrawVisibleInvalid ||
                (obj->minVis < globeViewState->heightAboveGlobe && globeViewState->heightAboveGlobe < obj->maxVis))
                use = true;
        } else
            use = true;
        if (use)
            layoutObjs.insert(it->second);
    }
    
    // Need to scale for retina displays
    float resScale = renderer.scale;

    // Set up the overlap sampler
    Point2f frameBufferSize;
    frameBufferSize.x() = renderer.framebufferWidth;
    frameBufferSize.y() = renderer.framebufferHeight;
    Mbr screenMbr(Point2f(-ScreenBuffer * frameBufferSize.x(),-ScreenBuffer * frameBufferSize.y()),frameBufferSize * (1.0 + ScreenBuffer));
    OverlapManager overlapMan(screenMbr,OverlapSampleX,OverlapSampleY);

    Matrix4f modelTrans = viewState->fullMatrix;
    int numSoFar = 0;
    for (WhirlyKitLayoutObjectSet::iterator it = layoutObjs.begin();
         it != layoutObjs.end(); ++it)
    {
        WhirlyKitLayoutObject *layoutObj = *it;
        
        // Start with a max objects check
        bool isActive = true;
        if (maxDisplayObjects != 0 && (numSoFar >= maxDisplayObjects))
            isActive = false;
        // Start with a back face check
        // Note: Doesn't take projection into account, but close enough
        if (isActive && globeViewState)
            isActive = layoutObj->dispLoc.dot(viewState->eyeVec) > 0.0;
        Point2f objOffset(0.0,0.0);
        if (isActive)
        {
            // Figure out where this will land
            CGPoint objPt = [viewState pointOnScreenFromDisplay:layoutObj->dispLoc transform:&modelTrans frameSize:frameBufferSize];
            isActive = screenMbr.inside(Point2f(objPt.x,objPt.y));
            // Now for the overlap checks
            if (isActive)
            {
                // Try the four diffierent orientations
                if (layoutObj->size.x() != 0.0 && layoutObj->size.y() != 0.0)
                {
                    bool validOrient = false;
                    Mbr objMbr = Mbr(Point2f(objPt.x,objPt.y),Point2f((objPt.x+layoutObj->size.x()*resScale),(objPt.y+layoutObj->size.y()*resScale)));
                    for (unsigned int orient=0;orient<4;orient++)
                    {
                        // May only want to be placed certain ways.  Fair enough.
                        if (!(layoutObj->acceptablePlacement & (1<<orient)))
                            continue;
                        
                        // Set up the offset for this orientation
                        switch (orient)
                        {
                            // Right
                            case 0:
                                objOffset = Point2f(layoutObj->iconSize.x(),0.0);
                                break;
                            // Left
                            case 1:
                                objOffset = Point2f(-(layoutObj->size.x()+layoutObj->iconSize.x()/2.0),0.0);
                                break;
                            // Above
                            case 2:
                                objOffset = Point2f(-layoutObj->size.x()/2.0,-(layoutObj->size.y()+layoutObj->iconSize.y())/2.0);
                                break;
                            // Below
                            case 3:
                                objOffset = Point2f(-layoutObj->size.x()/2.0,(layoutObj->size.y()+layoutObj->iconSize.y())/2.0);
                                break;
                        }
                        
                        // Now try it
                        Mbr tryMbr(objMbr.ll()+objOffset*resScale,objMbr.ur()+objOffset*resScale);
                        if (overlapMan.addObject(tryMbr, layoutObj))
                        {
                            validOrient = true;
                            break;
                        }
                    }
                    
                    isActive = validOrient;
                }
            }
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
}

// Time we'll take to disappear objects
static float const DisappearFade = 0.1;

// Layout all the objects we're tracking
- (void)updateLayout
{
    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
    
    // This will recalulate the offsets and enables
    [self runLayoutRules];

    std::vector<ScreenSpaceGeneratorGangChangeRequest::ShapeChange> changes;
    changes.reserve(layoutObjects.size());
    
    for (LayoutObjectMap::iterator it = layoutObjects.begin();
         it != layoutObjects.end(); ++it)
    {
        WhirlyKitLayoutObject *layoutObj = it->second;
        if (layoutObj->changed)
        {
            // Put in the change for the main object
            ScreenSpaceGeneratorGangChangeRequest::ShapeChange change;
            change.shapeID = layoutObj->ssID;
            change.offset = layoutObj->offset;
            change.enable = layoutObj->newEnable;
            // Fade in when we add them
            if (!layoutObj->currentEnable)
            {
                change.fadeDown = curTime;
                change.fadeUp = curTime+DisappearFade;
            }
            layoutObj->currentEnable = layoutObj->newEnable;
            changes.push_back(change);
            
            // And auxiliary objects
            for (SimpleIDSet::iterator sit = layoutObj->auxIDs.begin();
                 sit != layoutObj->auxIDs.end(); ++sit)
            {
                ScreenSpaceGeneratorGangChangeRequest::ShapeChange change;
                change.shapeID = *sit;
                change.enable = layoutObj->currentEnable;
                changes.push_back(change);
            }
            
            layoutObj->changed = false;
        }
    }
    
    scene->addChangeRequest(new ScreenSpaceGeneratorGangChangeRequest(ssGenId,changes));
}

- (void)addLayoutObjects:(NSArray *)newObjects
{
    if ([NSThread currentThread] != layerThread)
    {
        NSLog(@"WhirlyKitLayoutLayer: Called addLayoutObjects from outside the layer thread.  Ignoring data.");
        return;
    }
    
    for (WhirlyKitLayoutObject *obj in newObjects)
        layoutObjects[obj->ssID] = obj;

    // Note: This is too often.  Need a better way of notifying need for an update.
    [self updateLayout];
}

- (void)removeLayoutObjects:(const SimpleIDSet &)objectIDs
{
    if ([NSThread currentThread] != layerThread)
    {
        NSLog(@"WhirlyKitLayoutLayer: Called removeLayoutObjects from outside the layer thread.  Ignoring data.");
        return;
    }
    
    for (SimpleIDSet::iterator it = objectIDs.begin();it != objectIDs.end(); ++it)
    {
        LayoutObjectMap::iterator lit = layoutObjects.find(*it);
        if (lit != layoutObjects.end())
            layoutObjects.erase(lit);
    }
    
    // Note: This is too often.  Need a better way of notifying need for an update.
    [self updateLayout];
}

@end
