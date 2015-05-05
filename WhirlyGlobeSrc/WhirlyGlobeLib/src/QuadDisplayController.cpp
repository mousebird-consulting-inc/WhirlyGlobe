/*
 *  QuadDisplayLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/17/12.
 *  Copyright 2011-2013 mousebird consulting
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

//#import <android/log.h>

#import "QuadDisplayController.h"
#import "GlobeMath.h"
#import "FlatMath.h"
#import "VectorData.h"

using namespace Eigen;

namespace WhirlyKit
{
        
QuadDisplayController::QuadDisplayController(QuadDataStructure *dataStructure,QuadLoader *loader,QuadDisplayControllerAdapter *adapter)
: adapter(adapter), dataStructure(dataStructure), loader(loader), quadtree(NULL),
    scene(NULL), renderer(NULL), coordSys(dataStructure->getCoordSystem()), mbr(dataStructure->getValidExtents()),
    minImportance(1.0), maxTiles(128), minZoom(dataStructure->getMinZoom()), maxZoom(dataStructure->getMaxZoom()),
    greedyMode(false), meteredMode(true), waitForLocalLoads(false),fullLoad(false), fullLoadTimeout(4.0), viewUpdatePeriod(0.1),
    minUpdateDist(0.0), lineMode(false), debugMode(false), lastFlush(0.0), somethingHappened(false), firstUpdate(true), numFrames(1), canLoadFrames(false), curFrameEntry(-1), enable(true)
{
    // Note: Debugging
    greedyMode = true;
    meteredMode = false;
    pthread_mutex_init(&frameLoadingLock, NULL);
}

QuadDisplayController::~QuadDisplayController()
{
    if (quadtree)
        delete quadtree;
    quadtree = NULL;
}
    
void QuadDisplayController::init(Scene *inScene,SceneRendererES *inRenderer)
{
    scene = inScene;
    renderer = inRenderer;

    quadtree = new Quadtree(dataStructure->getTotalExtents(),minZoom,maxZoom,maxTiles,minImportance,this);
    loader->init(this,scene);

    canLoadFrames = loader->canLoadFrames();
    numFrames = loader->numFrames();
    if (numFrames == 1)
        canLoadFrames = false;

    if (canLoadFrames)
    {
        // Make up some reasonable frame priorities
        if (frameLoadingPriority.size() != numFrames)
        {
            for (unsigned int ii=0;ii<numFrames;ii++)
                frameLoadingPriority.push_back(ii);
        }
        curFrameEntry = frameLoadingPriority[0];
    }

    if (meteredMode)
    {
        ChangeSet changes;
        loader->startUpdates(changes);
        // Note: We'll never get any changes here
    }
}
    
void QuadDisplayController::setFrameLoadingPriorities(const std::vector<int> &priorities)
{
    // Note: Porting  Needs some thread logic above this level
    curFrameEntry = 0;
    frameLoadingPriority = priorities;
    resetEvaluation();
}
    
void QuadDisplayController::setEnable(bool newEnable)
{
    // Note: Porting  Needs some thread logic above this level
    if (enable != newEnable)
    {
        enable = newEnable;
        
        if (enable)
        {
            resetEvaluation();
            // Note: Porting Cancel the evalStep: and reset it above this level
        } else {
            // Note: Porting Cancel the evalStep: above this level
        }
    }
}
    
void QuadDisplayController::frameEnd(ChangeSet &changes)
{
    TimeInterval now = TimeGetCurrent();
    
    // We'll hold off for local loads...up to a point
    bool forcedFlush = false;
    if (now - lastFlush < fullLoadTimeout)
    {
        if (waitingForLocalLoads())
            return;
    } else
        forcedFlush = true;
    
    // Flush out the updates and immediately start new ones
    loader->endUpdates(changes);
    loader->startUpdates(changes);
    // If we forced out a flush, we can wait for more local loads
    if (!forcedFlush)
        waitForLocalLoads = false;
    lastFlush = now;
   
    somethingHappened = false;
}
    
void QuadDisplayController::viewUpdate(ViewState *inViewState)
{
    // We'll just ignore changes
    if (!enable)
    {
        viewState = *inViewState;
        return;
    }
    
    // Check if we should even be doing an update
    if (!loader->shouldUpdate(inViewState,firstUpdate))
        return;
    firstUpdate = false;
    
    dataStructure->newViewState(inViewState);
    
    viewState = *inViewState;
    // Start loading at frame zero again (if we're doing frame loading)
    if (!frameLoadingPriority.empty())
    {
        curFrameEntry = 0;
        //        NSLog(@"Frame reset: position 0");
    }
    
    resetEvaluation();
    // Note: Porting Above this level, reset the evalStep:

    if (fullLoad)
        waitForLocalLoads = true;
}
    
void QuadDisplayController::resetEvaluation()
{
    quadtree->clearEvals();
    toPhantom.clear();
    quadtree->reevaluateNodes();
    
    // Add everything at the minLevel back in
    for (int ix=0;ix<1<<minZoom;ix++)
        for (int iy=0;iy<1<<minZoom;iy++)
            quadtree->addTile(Quadtree::Identifier(ix,iy,minZoom),true,false);
}

// Dump out info about what we've got loaded in
void QuadDisplayController::dumpInfo()
{
//    NSLog(@"***Loaded Tiles***");
//    for (LoadedTileSet::iterator it = tileSet.begin();
//         it != tileSet.end(); ++it)
//    {
//        (*it)->Print(quadtree);
//    }
//    NSLog(@"******");
    
//    quadtree->Print();
}

// Check if we're waiting for local (e.g. fast) loads to finish
bool QuadDisplayController::waitingForLocalLoads()
{
    if (!waitForLocalLoads)
        return false;
    
    // Check for local fetches ongoing
    bool localActivity = quadtree->numEvals() != 0;
    if (!localActivity)
        localActivity = loader->numLocalFetches() != 0;
        
        if (!localActivity)
            return false;
    
    // Note: Cut short if there's network activity
    
    return true;
}

// Run the evaluation step for outstanding nodes
bool QuadDisplayController::evalStep(TimeInterval frameStart,TimeInterval frameInterval,float availableFrame,ChangeSet &changes)
{
	//__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "In QuadDisplayController::evalStep, availableFrame: %f", availableFrame);
    bool didSomething = false;
    somethingHappened = false;
    
    // Might have been turned off
    if (!enable)
        return false;
    
    // If the loader isn't ready, it's up to it to wake us up when it is
    if (!loader->isReady())
        return false;
    
    if (!meteredMode)
        loader->startUpdates(changes);
    
    // If we're doing frame loading, figure out which frame
    int curFrame = -1;
    if (!frameLoadingPriority.empty())
        curFrame = frameLoadingPriority[curFrameEntry];
	//__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Current frame entry: %d", curFrameEntry);
	//__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Current frame: %d", curFrame);
	
    
    if (quadtree->numEvals() != 0)
    {
        // Let the loader know we're about to do some updates
        while (quadtree->numEvals() != 0)
        {
            // Grab the node
            Quadtree::NodeInfo nodeInfo;
            bool nodeInfoValid = quadtree->popLastEval(nodeInfo);
            if (!nodeInfoValid)
                break;
            
            //            NSLog(@"Evaluating: %d: (%d,%d)", nodeInfo.ident.level, nodeInfo.ident.x, nodeInfo.ident.y);
            
            // Various actions we'll take after evaluation
            bool makePhantom = false;
            bool shouldLoad = false;
            bool shouldUnload = false;
            bool addChildren = false;
            
            // Quad tree loading mode
            if (targetLevels.empty())
            {
                // If it's loading, just leave things alone
                if (!nodeInfo.isFrameLoading(curFrame))
                {
                    if (nodeInfo.phantom || (canLoadFrames && !nodeInfo.isFrameLoaded(curFrame) && !nodeInfo.isFrameLoading(curFrame)))
                        shouldLoad = nodeInfo.ident.level <= maxZoom && quadtree->shouldLoadTile(nodeInfo.ident,curFrame) && !nodeInfo.failed;
                    else if (nodeInfo.ident.level < maxZoom)
                        addChildren = true;
                }
            } else {
                bool isInTargetLevels = targetLevels.find(nodeInfo.ident.level) != targetLevels.end();
                bool singleTargetLevel = targetLevels.size() == 1;
                int minTargetLevel = *(targetLevels.begin());
                int maxTargetLevel = *(--(targetLevels.end()));
                // Single level loading mode
                if (nodeInfo.phantom)
                {
                    if (nodeInfo.ident.level < minTargetLevel)
                    {
                        addChildren = true;
                        if (quadtree->childFailed(nodeInfo.ident))
                            shouldLoad = quadtree->shouldLoadTile(nodeInfo.ident,curFrame);
                    } else if (isInTargetLevels && !nodeInfo.failed)
                    {
                        if (nodeInfo.childCoverage && nodeInfo.ident.level != maxTargetLevel)
                            addChildren = true;
                        else
                            shouldLoad = quadtree->shouldLoadTile(nodeInfo.ident,curFrame);
                    } else if (nodeInfo.ident.level > maxTargetLevel)
                        shouldUnload = true;
                    else if (!isInTargetLevels)
                        addChildren = true;
                } else {
                    bool shouldLoadFrame = canLoadFrames && !nodeInfo.isFrameLoaded(curFrame);
                    if (isInTargetLevels && shouldLoadFrame)
                    {
                        shouldLoad = !nodeInfo.isFrameLoading(curFrame);
                    } else {
                        if (nodeInfo.ident.level < maxTargetLevel)
                        {
                            addChildren = true;
                            if (nodeInfo.childCoverage || singleTargetLevel)
                                makePhantom = true;
                            else
                                shouldLoad = shouldLoadFrame && !nodeInfo.isFrameLoading(curFrame);
                        }
                    }
                }
            }
            
            // Evaluate children at some point soon
            if (addChildren)
            {
                //                NSLog(@"Adding children for tile: %d: (%d,%d)",nodeInfo.ident.level,nodeInfo.ident.x,nodeInfo.ident.y);
                std::vector<Quadtree::Identifier> childNodes;
                quadtree->childrenForNode(nodeInfo.ident, childNodes);
                for (unsigned int ic=0;ic<childNodes.size();ic++)
                    if (!quadtree->didFail(childNodes[ic]))
                        quadtree->addTile(childNodes[ic], true, true);
            }
            
            // Actually load a tile
            if (shouldLoad)
            {
                // We may have to force one out first
                if (quadtree->isFull())
                {
                    Quadtree::NodeInfo remNodeInfo;
                    quadtree->leastImportantNode(remNodeInfo,true);
                    //                    NSLog(@"Forcing unload tile: %d: (%d,%d) phantom = %@, import = %f",remNodeInfo.ident.level,remNodeInfo.ident.x,remNodeInfo.ident.y,(remNodeInfo.phantom ? @"YES" : @"NO"), remNodeInfo.importance);
                    quadtree->removeTile(remNodeInfo.ident);
                    
                    loader->unloadTile(remNodeInfo);
                }
                
                quadtree->setPhantom(nodeInfo.ident, false);
                quadtree->setLoading(nodeInfo.ident, curFrame, true);
                
                // Take it out of the phantom list
                QuadIdentSet::iterator it = toPhantom.find(nodeInfo.ident);
                if (it != toPhantom.end())
                    toPhantom.erase(it);
                
                if (canLoadFrames)
                {
                    int frameId = frameLoadingPriority[curFrameEntry];
                    //                    NSLog(@"Loading tile: %d: (%d,%d), frame = %d",nodeInfo.ident.level,nodeInfo.ident.x,nodeInfo.ident.y,frameId);
                    loader->loadTile(nodeInfo,frameId);
                } else {
                    //                    NSLog(@"Loading tile: %d: (%d,%d)",nodeInfo.ident.level,nodeInfo.ident.x,nodeInfo.ident.y);
                    loader->loadTile(nodeInfo,-1);
                }
            }
            
            // Unload a tile
            if (shouldUnload)
            {
                quadtree->removeTile(nodeInfo.ident);
                // Take it out of the phantom list
                QuadIdentSet::iterator it = toPhantom.find(nodeInfo.ident);
                if (it != toPhantom.end())
                    toPhantom.erase(it);
                //                NSLog(@"Unload tile: %d: (%d,%d)",nodeInfo.ident.level,nodeInfo.ident.x,nodeInfo.ident.y);
                loader->unloadTile(nodeInfo);
            }
            
            // Turn this into a phantom node
            if (makePhantom)
                toPhantom.insert(nodeInfo.ident);
            
            // If we're not in greedy mode, we're only doing this for a certain time period, then we'll hand off
            TimeInterval now = TimeGetCurrent();
            if (!greedyMode && meteredMode)
            {
                if (now-frameStart > availableFrame*frameInterval || !loader->isReady())
                    break;
            }
        }
        
        didSomething = true;
    }
    
    // Clean out old ndoes
    Quadtree::NodeInfo remNodeInfo;
    while (quadtree->leastImportantNode(remNodeInfo,false))
    {
        //        NSLog(@"Unload tile: %d: (%d,%d) phantom = %@, import = %f",remNodeInfo.ident.level,remNodeInfo.ident.x,remNodeInfo.ident.y,(remNodeInfo.phantom ? @"YES" : @"NO"), remNodeInfo.importance);
        quadtree->removeTile(remNodeInfo.ident);
        if (!remNodeInfo.phantom)
            loader->unloadTile(remNodeInfo);
        
        didSomething = true;
    }
    
    // Deal with outstanding phantom nodes
    if (!toPhantom.empty())
    {
        std::set<Quadtree::Identifier> toKeep;
        
        for (std::set<Quadtree::Identifier>::iterator it = toPhantom.begin();it != toPhantom.end(); ++it)
        {
            Quadtree::Identifier ident = *it;
            
            if (!quadtree->childrenEvaluating(ident) && !quadtree->childrenLoading(ident))
            {
                //                NSLog(@"Flushing phantom tile: %d: (%d,%d)",ident.level,ident.x,ident.y);
                const Quadtree::NodeInfo *nodeInfo = quadtree->getNodeInfo(ident);
                if (nodeInfo)
                {
                    loader->unloadTile(*nodeInfo);
                    quadtree->setPhantom(ident, true);
                    quadtree->setLoading(ident, -1, false);
                    didSomething = true;
                }
            } else {
                toKeep.insert(ident);
                //                NSLog(@"Children loading");
            }
        }
        
        toPhantom = toKeep;
    }
    
    // Let the loader know we're done with this eval step
    if (meteredMode || waitingForLocalLoads() || didSomething)
    {
        loader->updateWithoutFlush();
    } else
        loader->endUpdates(changes);
    
//    if (_debugMode)
//        [self dumpInfo];
    
    // See if we can move on to the next frame (if we're frame loading
    if (!didSomething && !frameLoadingPriority.empty() && quadtree->frameIsLoaded(frameLoadingPriority[curFrameEntry],NULL))
    {
        for (int ii=1;ii<numFrames;ii++)
        {
            int newFrameEntry = (curFrameEntry+ii)%numFrames;
            if (!quadtree->frameIsLoaded(frameLoadingPriority[newFrameEntry],NULL))
            {
                curFrameEntry = newFrameEntry;
                resetEvaluation();
                didSomething = true;
                //                NSLog(@"Switching to frame entry: %d",curFrameEntry);
                break;
            }
        }
    }
    
    // Porting: Check the metering logic one level lup
//    if (didSomething)
//        [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
//    else {
//        // If we're not waiting for local reloads, we may be done
//        if (!_meteredMode && ![self waitingForLocalLoads])
//            [_loader quadDisplayLayerEndUpdates:self];
//        
//        // We're done waiting for local fetches.  Let the next frame boundary catch it
//        if (waitForLocalLoads && ![self waitingForLocalLoads])
//        {
//            waitForLocalLoads = false;
//        }
//    }
    
    somethingHappened |= didSomething;
    
    // If we're in metered mode, make sure we've got a flush here
//    if (_meteredMode)
//    {
//        NSTimeInterval howLong = frameEndTime-CFAbsoluteTimeGetCurrent();
//        if (howLong < 0.0)
//            howLong = 0.0;
//        [self performSelector:@selector(frameEndThread) withObject:nil afterDelay:howLong];
//    }
    
    // Update the frame load status.
    // We expect this to be accessed outside of the thread
    // Note: Might be nice to only do this when necessary
//    @synchronized(self)
    {
        pthread_mutex_lock(&frameLoadingLock);
        if (!frameLoadingPriority.empty())
        {
            frameLoadStats.resize(numFrames);
            
            for (unsigned int ii=0;ii<numFrames;ii++)
            {
                FrameLoadStatus &status = frameLoadStats[ii];
                status.complete = quadtree->frameIsLoaded(ii, &status.numTilesLoaded);
                status.currentFrame = ii == frameLoadingPriority[curFrameEntry];
            }
        }
        pthread_mutex_unlock(&frameLoadingLock);
    }
    
    return somethingHappened;
}

void QuadDisplayController::tileDidLoad(const WhirlyKit::Quadtree::Identifier &tileIdent,int frame)
{
	//__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "In QuadDisplayController::tileDidLoad, frame %d", frame);
    //    NSLog(@"Tile did load: %d: (%d,%d), %d",tileIdent.level,tileIdent.x,tileIdent.y,frame);
    
    // Make sure we still want this one
    const Quadtree::NodeInfo *node = quadtree->getNodeInfo(tileIdent);
    if (!node)
        return;
    
    quadtree->didLoad(tileIdent,frame);
    
    // Update the parent coverage and then make those tiles phantoms if
    //  they're now fully covered
    if (!targetLevels.empty())
    {
        std::vector<Quadtree::Identifier> tilesCovered,tilesUncovered;
        quadtree->updateParentCoverage(tileIdent,tilesCovered,tilesUncovered);
        for (unsigned int ii=0;ii<tilesCovered.size();ii++)
        {
            const Quadtree::Identifier &ident = tilesCovered[ii];
            if (!quadtree->isPhantom(ident))
            {
                //                NSLog(@"Adding to phantom due to coverage: %d: (%d,%d)",ident.level,ident.x,ident.y);
                toPhantom.insert(ident);
            }
        }
    }
    
    // May want to consider the children next
    if (tileIdent.level < maxZoom)
    {
        int maxTargetLevel = targetLevels.empty() ? maxZoom : *(--(targetLevels.end()));
        if (targetLevels.empty() || tileIdent.level < maxTargetLevel)
        {
            if (tileIdent.level < maxTargetLevel)
            {
                // Now try the children
                std::vector<Quadtree::Identifier> childNodes;
                quadtree->childrenForNode(tileIdent, childNodes);
                for (unsigned int ic=0;ic<childNodes.size();ic++)
                    if (!quadtree->didFail(childNodes[ic]))
                    {
                        quadtree->addTile(childNodes[ic], true, true);
                        
                        // Take it out of the phantom list
                        QuadIdentSet::iterator it = toPhantom.find(childNodes[ic]);
                        if (it != toPhantom.end())
                            toPhantom.erase(it);
                    }
            }
        }
    }
    
    somethingHappened = true;
    
    // Porting: Reschedule evalStep()
}
    
// Tile failed to load.
// At the moment we don't care, but we won't look at the children
void QuadDisplayController::tileDidNotLoad(const Quadtree::Identifier &tileIdent,int frame)
{
    //    NSLog(@"Tile failed to load: %d: (%d,%d) %d",tileIdent.level,tileIdent.x,tileIdent.y,frame);
    
    quadtree->setLoading(tileIdent, frame, false);
    quadtree->setPhantom(tileIdent, true);
    quadtree->setFailed(tileIdent, true);
    
    // Let's try to load the parent if we're in target level mode
    if (!targetLevels.empty() && tileIdent.level > 0)
    {
        Quadtree::Identifier parentIdent(tileIdent.x/2,tileIdent.y/2,tileIdent.level-1);
        quadtree->addTile(parentIdent, true, true);
        
        // Take it out of the phantom list
        QuadIdentSet::iterator it = toPhantom.find(parentIdent);
        if (it != toPhantom.end())
            toPhantom.erase(it);
    }

    // Porting: Reset the evalStep() on level up
    
    somethingHappened = true;
}
    
// Clear out all the existing tiles and start over
void QuadDisplayController::refresh(ChangeSet &changes)
{
    // We're still dealing with the last one
    if (waitForLocalLoads)
        return;

    quadtree->clearEvals();
    quadtree->clearFails();

    // Remove nodes until we run out
    Quadtree::NodeInfo remNodeInfo;
    loader->startUpdates(changes);
    while (quadtree->leastImportantNode(remNodeInfo,true))
    {
        
        quadtree->removeTile(remNodeInfo.ident);
        loader->unloadTile(remNodeInfo);
    }
    waitForLocalLoads = true;
    
    // Add everything at the minLevel back in
    for (int ix=0;ix<1<<minZoom;ix++)
        for (int iy=0;iy<1<<minZoom;iy++)
            quadtree->addTile(Quadtree::Identifier(ix,iy,minZoom), true, false);
    
    loader->startUpdates(changes);

    somethingHappened = true;
}
    
void QuadDisplayController::shutdown(ChangeSet &changes)
{
    loader->endUpdates(changes);
    
    dataStructure->shutdown();
    loader->shutdownLayer(changes);
}

void QuadDisplayController::reset(ChangeSet &changes)
{    
    waitForLocalLoads = false;
    
    quadtree->clearEvals();
    quadtree->clearFails();
    
    // Remove nodes until we run out
    Quadtree::NodeInfo remNodeInfo;
    loader->startUpdates(changes);
    while (quadtree->leastImportantNode(remNodeInfo,true))
    {
        quadtree->removeTile(remNodeInfo.ident);
    }
    
    // Tell the tile loader to reset
    loader->reset(changes);
    
    // Now start loading again
    poke();
}

void QuadDisplayController::poke()
{
    viewUpdate(&viewState);
}


void QuadDisplayController::wakeUp()
{
    somethingHappened = true;
    adapter->adapterWakeUp();
}

// Importance callback for quad tree
double QuadDisplayController::importanceForTile(const Quadtree::Identifier &ident,const Mbr &theMbr,Quadtree *tree,Dictionary *attrs)
{
    return dataStructure->importanceForTile(ident,theMbr,&viewState,renderer->getFramebufferSize(),attrs);
}

}

