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

#import "QuadDisplayLayer.h"
#import "GlobeMath.h"
#import "GlobeLayerViewWatcher.h"
#import "MaplyLayerViewWatcher.h"
#import "UIImage+Stuff.h"
#import "FlatMath.h"
#import "VectorData.h"
#import "SceneRendererES2.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation WhirlyKitQuadDisplayLayer
{
    /// [minZoom,maxZoom] range
    int minZoom,maxZoom;
    
    // Nodes to turn into phantoms.  We like to wait a bit
    WhirlyKit::QuadIdentSet toPhantom;

    /// If set the eval step gets very aggressive about loading tiles.
    /// This will slow down the layer thread, but makes the quad layer appear faster
    bool greedyMode;
    
    /// State of the view the last time we were called
    WhirlyKitViewState *viewState;
    
    /// Frame times for metered mode
    NSTimeInterval frameStart,frameInterval,frameEndTime;
    
    /// Set if we're waiting for local loads (e.g. a reload)
    bool waitForLocalLoads;
    
    // In metered mode, the last time we flushed data to the scene
    NSTimeInterval lastFlush;
    
    // In metered mode, we'll only flush if something happened
    bool somethingHappened;
    
    // The loader can load individual frames of an animation
    bool canLoadFrames;
    
    // Number of frames we'll try load per tile
    int numFrames;

    // Current entry in the frame priority list (not the actual frame) if we're loading frames
    int curFrameEntry;
    
    // If we're loading frames, this is the order we load them in
    std::vector<int> frameLoadingPriority;
    
    // If we're loading frames, the frame loading status last time through the eval loop
    std::vector<FrameLoadStatus> frameLoadStats;
}

- (id)initWithDataSource:(NSObject<WhirlyKitQuadDataStructure> *)inDataStructure loader:(NSObject<WhirlyKitQuadLoader> *)inLoader renderer:(WhirlyKitSceneRendererES *)inRenderer;
{
    self = [super init];
    if (self)
    {
        _dataStructure = inDataStructure;
        _loader = inLoader;
        _coordSys = [_dataStructure coordSystem];
        _mbr = [_dataStructure validExtents];
        minZoom = [_dataStructure minZoom];
        maxZoom = [_dataStructure maxZoom];
        _maxTiles = 128;
        _minImportance = 1.0;
        _viewUpdatePeriod = 0.1;
        _quadtree = new Quadtree([_dataStructure totalExtents],minZoom,maxZoom,_maxTiles,_minImportance,self);
        _renderer = (WhirlyKitSceneRendererES2 *)inRenderer;
        _lineMode = false;
        _drawEmpty = false;
        _debugMode = false;
        greedyMode = false;
        _meteredMode = true;
        _fullLoad = false;
        _fullLoadTimeout = 4.0;
        numFrames = 1;
        waitForLocalLoads = false;
        somethingHappened = false;
        canLoadFrames = false;
        curFrameEntry = -1;
        _enable = true;
    }
    
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    if (_quadtree)
        delete _quadtree;
    _quadtree = NULL;
}

- (void)setMaxTiles:(int)newMaxTiles
{
    _maxTiles = newMaxTiles;
    _quadtree->setMaxNodes(newMaxTiles);
}

- (void)setMinImportance:(float)newMinImportance
{
    _minImportance = newMinImportance;
    _quadtree->setMinImportance(newMinImportance);
}

- (void)setFrameLoadingPriorities:(std::vector<int> &)priorities
{
    curFrameEntry = 0;
    frameLoadingPriority = priorities;
    [self resetEvaluation];
}

- (void)setEnable:(bool)enable
{
    if ([NSThread currentThread] != _layerThread)
    {
        [self performSelector:@selector(runSetEnable:) onThread:_layerThread withObject:@(enable) waitUntilDone:NO];
        return;
    }
    
    [self runSetEnable:@(enable)];
}

- (void)runSetEnable:(NSNumber *)enable
{
    bool newEnable = [enable boolValue];
    if (_enable != newEnable)
    {
        _enable = newEnable;
        
        if (_enable)
        {
            [self resetEvaluation];
            [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
            [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
        } else {
            [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
        }
    }
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(Scene *)inScene
{
    _layerThread = inLayerThread;
	_scene = inScene;
    [_loader setQuadLayer:self];
    
    // We want view updates, but only 1s in frequency
    if (_layerThread.viewWatcher)
        [(WhirlyGlobeLayerViewWatcher *)_layerThread.viewWatcher addWatcherTarget:self selector:@selector(viewUpdate:) minTime:_viewUpdatePeriod minDist:_minUpdateDist maxLagTime:10.0];
    
    if (_meteredMode)
    {
        [_renderer addFrameObserver:self];
        [_loader quadDisplayLayerStartUpdates:self];
    }

    if (_fullLoad)
        waitForLocalLoads = true;
    
    canLoadFrames = [_loader respondsToSelector:@selector(quadDisplayLayer:loadTile:frame:)];
    numFrames = [_loader numFrames];
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
    
//    [self performSelector:@selector(dumpInfo) withObject:nil afterDelay:15.0];
}

- (void)shutdown
{
    [_renderer removeFrameObserver:self];
    [_loader quadDisplayLayerEndUpdates:self];

    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    if (_layerThread.viewWatcher) {
        [(WhirlyGlobeLayerViewWatcher *)_layerThread.viewWatcher removeWatcherTarget:self selector:@selector(viewUpdate:)];
    }
    
    [_dataStructure shutdown];
    _dataStructure = nil;
    [_loader shutdownLayer:self scene:_scene];
    _loader = nil;
    
    _scene = NULL;
}

// Called by the renderer (in that thread, so be careful)
- (void)frameStart:(WhirlyKitFrameMessage *)msg;
{        
    frameStart = msg.frameStart;
    frameInterval = msg.frameInterval;
    
    if (_meteredMode)
        frameEndTime = frameStart+AvailableFrame*frameInterval;
}

- (void)frameEndThread
{
    if (!somethingHappened)
        return;
    
    NSTimeInterval now = CFAbsoluteTimeGetCurrent();

    // We'll hold off for local loads...up to a point
    bool forcedFlush = false;
    if (now - lastFlush < _fullLoadTimeout)
    {
        if ([self waitingForLocalLoads])
            return;
    } else
        forcedFlush = true;
    
    // Flush out the updates and immediately start new ones
    [_loader quadDisplayLayerEndUpdates:self];
    [_loader quadDisplayLayerStartUpdates:self];
    // If we forced out a flush, we can wait for more local loads
    if (!forcedFlush)
        waitForLocalLoads = false;
    lastFlush = now;
    
    somethingHappened = false;
}

// Called every so often by the view watcher
// It's here that we evaluate what to load
- (void)viewUpdate:(WhirlyKitViewState *)inViewState
{
    if (!_scene)
    {
        NSLog(@"GlobeQuadDisplayLayer: Called viewUpdate: after being shutdown.");
        return;
    }
    
    // Just put ourselves on hold for a while
    if (!inViewState)
        return;
    
    // We'll just ignore changes
    if (!_enable)
    {
        // We do need an initial view state, though
        viewState = inViewState;
        return;
    }
    
    // Check if we should even be doing an update
    if ([_loader respondsToSelector:@selector(shouldUpdate:initial:)])
        if (![_loader shouldUpdate:inViewState initial:(viewState == nil)])
            return;
    
    if ([_dataStructure respondsToSelector:@selector(newViewState:)])
        [_dataStructure newViewState:inViewState];
    
    if (_fullLoad)
        waitForLocalLoads = true;
        
    viewState = inViewState;
    
    // Start loading at frame zero again (if we're doing frame loading)
    if (!frameLoadingPriority.empty())
    {
        curFrameEntry = 0;
//        NSLog(@"Frame reset: position 0");
    }
    
    [self resetEvaluation];
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
}

- (void)resetEvaluation
{
    _quadtree->clearEvals();
    toPhantom.clear();
    _quadtree->reevaluateNodes();
    
    // Add everything at the minLevel back in
    for (int ix=0;ix<1<<minZoom;ix++)
        for (int iy=0;iy<1<<minZoom;iy++)
            _quadtree->addTile(Quadtree::Identifier(ix,iy,minZoom),true,false);
}

// Dump out info about what we've got loaded in
- (void)dumpInfo
{
    [_loader log];
    
    _quadtree->Print();
    
    [self performSelector:@selector(dumpInfo) withObject:nil afterDelay:15.0];
}

// Information about what's currently loaded in
// Less detail than dumpInfo (which was for debugging)
- (void)log
{
    if ([_loader respondsToSelector:@selector(log)])
        [_loader log];
}

// Check if we're waiting for local (e.g. fast) loads to finish
- (bool)waitingForLocalLoads
{
    if (!waitForLocalLoads)
        return false;
    
    // Check for local fetches ongoing
    bool localActivity = _quadtree->numEvals() != 0;
    if (!localActivity && [_loader respondsToSelector:@selector(localFetches)])
        localActivity = [_loader localFetches] != 0;
    
    if (!localActivity)
        return false;

    // Note: Cut short if there's network activity
    
    return true;
}

// How much of the frame time we're willing to spend
static const NSTimeInterval AvailableFrame = 4.0/5.0;

// Run the evaluation step for outstanding nodes
- (void)evalStep:(id)Sender
{
    bool didSomething = false;
    
    // Might have been turned off
    if (!_enable)
        return;
    
    // If the renderer hasn't been set up, punt and try again later
    if (_renderer.framebufferWidth == 0 || _renderer.framebufferHeight == 0 || viewState == nil)
    {
        [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.1];
        return;
    }    

    // If the loader isn't ready, it's up to it to wake us up when it is
    if (![_loader isReady])
    {
        return;
    }

    if (!_meteredMode)
        [_loader quadDisplayLayerStartUpdates:self];
    
    // If we're doing frame loading, figure out which frame
    int curFrame = -1;
    if (!frameLoadingPriority.empty())
        curFrame = frameLoadingPriority[curFrameEntry];
    
    if (_quadtree->numEvals() != 0)
    {
        // Let the loader know we're about to do some updates        
        while (_quadtree->numEvals() != 0)
        {
            // Grab the node
            Quadtree::NodeInfo nodeInfo;
            bool nodeInfoValid = _quadtree->popLastEval(nodeInfo);
            if (!nodeInfoValid)
                break;
            
//            NSLog(@"Evaluating: %d: (%d,%d)", nodeInfo.ident.level, nodeInfo.ident.x, nodeInfo.ident.y);

            // Various actions we'll take after evaluation
            bool makePhantom = false;
            bool shouldLoad = false;
            bool shouldUnload = false;
            bool addChildren = false;
            
            // Quad tree loading mode
            if (_targetLevels.empty())
            {
                // If it's loading, just leave things alone
                if (!nodeInfo.isFrameLoading(curFrame))
                {
                    if (nodeInfo.phantom || (canLoadFrames && !nodeInfo.isFrameLoaded(curFrame) && !nodeInfo.isFrameLoading(curFrame)))
                        shouldLoad = nodeInfo.ident.level <= maxZoom && _quadtree->shouldLoadTile(nodeInfo.ident,curFrame) && !nodeInfo.failed;
                    else if (nodeInfo.ident.level < maxZoom)
                        addChildren = true;
                }
            } else {
                bool isInTargetLevels = _targetLevels.find(nodeInfo.ident.level) != _targetLevels.end();
                bool singleTargetLevel = _targetLevels.size() == 1;
                int minTargetLevel = *(_targetLevels.begin());
                int maxTargetLevel = *(--(_targetLevels.end()));
                // Single level loading mode
                if (nodeInfo.phantom)
                {
                    if (nodeInfo.ident.level < minTargetLevel)
                    {
                        addChildren = true;
                        if (_quadtree->childFailed(nodeInfo.ident))
                            shouldLoad = _quadtree->shouldLoadTile(nodeInfo.ident,curFrame);
                    } else if (isInTargetLevels && !nodeInfo.failed)
                    {
                        if (nodeInfo.childCoverage && nodeInfo.ident.level != maxTargetLevel)
                            addChildren = true;
                        else
                            shouldLoad = _quadtree->shouldLoadTile(nodeInfo.ident,curFrame);
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
                _quadtree->childrenForNode(nodeInfo.ident, childNodes);
                for (unsigned int ic=0;ic<childNodes.size();ic++)
                    if (!_quadtree->didFail(childNodes[ic]))
                        _quadtree->addTile(childNodes[ic], true, true);
            }
            
            // Actually load a tile
            if (shouldLoad)
            {
                // We may have to force one out first
                if (_quadtree->isFull())
                {
                    Quadtree::NodeInfo remNodeInfo;
                    _quadtree->leastImportantNode(remNodeInfo,true);
//                    NSLog(@"Forcing unload tile: %d: (%d,%d) phantom = %@, import = %f",remNodeInfo.ident.level,remNodeInfo.ident.x,remNodeInfo.ident.y,(remNodeInfo.phantom ? @"YES" : @"NO"), remNodeInfo.importance);
                    _quadtree->removeTile(remNodeInfo.ident);
                    
                    [_loader quadDisplayLayer:self unloadTile:&remNodeInfo];
                }
                
                _quadtree->setPhantom(nodeInfo.ident, false);
                _quadtree->setLoading(nodeInfo.ident, curFrame, true);
                
                // Take it out of the phantom list
                QuadIdentSet::iterator it = toPhantom.find(nodeInfo.ident);
                if (it != toPhantom.end())
                    toPhantom.erase(it);

                if (canLoadFrames)
                {
                    int frameId = frameLoadingPriority[curFrameEntry];
//                    NSLog(@"Loading tile: %d: (%d,%d), frame = %d",nodeInfo.ident.level,nodeInfo.ident.x,nodeInfo.ident.y,frameId);
                    [_loader quadDisplayLayer:self loadTile:&nodeInfo frame:frameId];
                } else {
//                    NSLog(@"Loading tile: %d: (%d,%d)",nodeInfo.ident.level,nodeInfo.ident.x,nodeInfo.ident.y);
                    [_loader quadDisplayLayer:self loadTile:&nodeInfo];
                }
            }
            
            // Unload a tile
            if (shouldUnload)
            {
                _quadtree->removeTile(nodeInfo.ident);
                // Take it out of the phantom list
                QuadIdentSet::iterator it = toPhantom.find(nodeInfo.ident);
                if (it != toPhantom.end())
                    toPhantom.erase(it);
//                NSLog(@"Unload tile: %d: (%d,%d)",nodeInfo.ident.level,nodeInfo.ident.x,nodeInfo.ident.y);
                [_loader quadDisplayLayer:self unloadTile:&nodeInfo];
            }
            
            // Turn this into a phantom node
            if (makePhantom)
                toPhantom.insert(nodeInfo.ident);
            
            // If we're not in greedy mode, we're only doing this for a certain time period, then we'll hand off
            NSTimeInterval now = CFAbsoluteTimeGetCurrent();
            if (!greedyMode && _meteredMode)
            {
                if (now-frameStart > AvailableFrame*frameInterval || ![_loader isReady])
                    break;
            }
        }
        
        didSomething = true;
    }
    
    // Clean out old ndoes
    Quadtree::NodeInfo remNodeInfo;
    while (_quadtree->leastImportantNode(remNodeInfo,false))
    {
//        NSLog(@"Unload tile: %d: (%d,%d) phantom = %@, import = %f",remNodeInfo.ident.level,remNodeInfo.ident.x,remNodeInfo.ident.y,(remNodeInfo.phantom ? @"YES" : @"NO"), remNodeInfo.importance);
        _quadtree->removeTile(remNodeInfo.ident);
        if (!remNodeInfo.phantom)
            [_loader quadDisplayLayer:self unloadTile:&remNodeInfo];
        
        didSomething = true;
    }

    // Deal with outstanding phantom nodes
    if (!toPhantom.empty())
    {
        std::set<Quadtree::Identifier> toKeep;
        
        for (std::set<Quadtree::Identifier>::iterator it = toPhantom.begin();it != toPhantom.end(); ++it)
        {
            Quadtree::Identifier ident = *it;
            
            if (!_quadtree->childrenEvaluating(ident) && !_quadtree->childrenLoading(ident))
            {
//                NSLog(@"Flushing phantom tile: %d: (%d,%d)",ident.level,ident.x,ident.y);
                const Quadtree::NodeInfo *nodeInfo = _quadtree->getNodeInfo(ident);
                if (nodeInfo)
                {
                    [_loader quadDisplayLayer:self unloadTile:nodeInfo];
                    _quadtree->setPhantom(ident, true);
                    _quadtree->setLoading(ident, -1, false);
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
    if (_meteredMode || [self waitingForLocalLoads] || didSomething)
    {
        if ([_loader respondsToSelector:@selector(updateWithoutFlush)])
            [_loader updateWithoutFlush];
    } else
        [_loader quadDisplayLayerEndUpdates:self];

    if (_debugMode)
        [self dumpInfo];

    // See if we can move on to the next frame (if we're frame loading
    if (!didSomething && !frameLoadingPriority.empty() && _quadtree->frameIsLoaded(frameLoadingPriority[curFrameEntry],NULL))
    {
        for (int ii=1;ii<numFrames;ii++)
        {
            int newFrameEntry = (curFrameEntry+ii)%numFrames;
            if (!_quadtree->frameIsLoaded(frameLoadingPriority[newFrameEntry],NULL))
            {
                curFrameEntry = newFrameEntry;
                [self resetEvaluation];
                didSomething = true;
//                NSLog(@"Switching to frame entry: %d",curFrameEntry);
                break;
            }
        }
    }
    
    if (didSomething)
        [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
    else {
        // If we're not waiting for local reloads, we may be done
        if (!_meteredMode && ![self waitingForLocalLoads])
            [_loader quadDisplayLayerEndUpdates:self];
        
        // We're done waiting for local fetches.  Let the next frame boundary catch it
        if (waitForLocalLoads && ![self waitingForLocalLoads])
        {
            waitForLocalLoads = false;
        }
    }
    
    somethingHappened |= didSomething;
    
    // If we're in metered mode, make sure we've got a flush here
    if (_meteredMode)
    {
        NSTimeInterval howLong = frameEndTime-CFAbsoluteTimeGetCurrent();
        if (howLong < 0.0)
            howLong = 0.0;
        [self performSelector:@selector(frameEndThread) withObject:nil afterDelay:howLong];
    }
    
    // Update the frame load status.
    // We expect this to be accessed outside of the thread
    // Note: Might be nice to only do this when necessary
    @synchronized(self)
    {
        if (!frameLoadingPriority.empty())
        {
            frameLoadStats.resize(numFrames);
            
            for (unsigned int ii=0;ii<numFrames;ii++)
            {
                FrameLoadStatus &status = frameLoadStats[ii];
                status.complete = _quadtree->frameIsLoaded(ii, &status.numTilesLoaded);
                status.currentFrame = ii == frameLoadingPriority[curFrameEntry];
            }
        }
    }
}

- (void)getFrameLoadStatus:(std::vector<WhirlyKit::FrameLoadStatus> &)retFrameLoadStats
{
    @synchronized(self)
    {
        retFrameLoadStats = frameLoadStats;
    }
}




// This is called bydd the loader when it finished loading a tile
// Once loaded we can try the children
- (void)loader:(NSObject<WhirlyKitQuadLoader> *)loader tileDidLoad:(WhirlyKit::Quadtree::Identifier)tileIdent frame:(int)frame
{
//    NSLog(@"Tile did load: %d: (%d,%d), %d",tileIdent.level,tileIdent.x,tileIdent.y,frame);
    
    // Make sure we still want this one
    const Quadtree::NodeInfo *node = _quadtree->getNodeInfo(tileIdent);
    if (!node)
        return;
    
    _quadtree->didLoad(tileIdent,frame);

    // Update the parent coverage and then make those tiles phantoms if
    //  they're now fully covered
    if (!_targetLevels.empty())
    {
        std::vector<Quadtree::Identifier> tilesCovered,tilesUncovered;
        _quadtree->updateParentCoverage(tileIdent,tilesCovered,tilesUncovered);
        for (unsigned int ii=0;ii<tilesCovered.size();ii++)
        {
            const Quadtree::Identifier &ident = tilesCovered[ii];
            if (!_quadtree->isPhantom(ident))
            {
                //                NSLog(@"Adding to phantom due to coverage: %d: (%d,%d)",ident.level,ident.x,ident.y);
                toPhantom.insert(ident);
            }
        }
    }

    // May want to consider the children next
    if (tileIdent.level < maxZoom)
    {
        int maxTargetLevel = _targetLevels.empty() ? maxZoom : *(--(_targetLevels.end()));
        if (_targetLevels.empty() || tileIdent.level < maxTargetLevel)
        {
            if (tileIdent.level < maxTargetLevel)
            {
                // Now try the children
                std::vector<Quadtree::Identifier> childNodes;
                _quadtree->childrenForNode(tileIdent, childNodes);
                for (unsigned int ic=0;ic<childNodes.size();ic++)
                    if (!_quadtree->didFail(childNodes[ic]))
                    {
                        _quadtree->addTile(childNodes[ic], true, true);

                        // Take it out of the phantom list
                        QuadIdentSet::iterator it = toPhantom.find(childNodes[ic]);
                        if (it != toPhantom.end())
                            toPhantom.erase(it);
                    }
            }
        }
    }

    // Make sure we actually evaluate them
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
    
    somethingHappened = true;
}

// Tile failed to load.
// At the moment we don't care, but we won't look at the children
- (void)loader:(NSObject<WhirlyKitQuadLoader> *)loader tileDidNotLoad:(WhirlyKit::Quadtree::Identifier)tileIdent frame:(int)frame
{
//    NSLog(@"Tile failed to load: %d: (%d,%d) %d",tileIdent.level,tileIdent.x,tileIdent.y,frame);

    _quadtree->setLoading(tileIdent, frame, false);
    _quadtree->setPhantom(tileIdent, true);
    _quadtree->setFailed(tileIdent, true);

    // Let's try to load the parent if we're in target level mode
    if (!_targetLevels.empty() && tileIdent.level > 0)
    {
        Quadtree::Identifier parentIdent(tileIdent.x/2,tileIdent.y/2,tileIdent.level-1);
        _quadtree->addTile(parentIdent, true, true);

        // Take it out of the phantom list
        QuadIdentSet::iterator it = toPhantom.find(parentIdent);
        if (it != toPhantom.end())
            toPhantom.erase(it);
    }

    // Might get stuck here
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];    

    somethingHappened = true;
}

// Clear out all the existing tiles and start over
- (void)refresh
{
    if ([NSThread currentThread] != _layerThread)
    {
        [self performSelector:@selector(refresh) onThread:_layerThread withObject:nil waitUntilDone:NO];
        return;
    }
    
    // We're still dealing with the last one
    if (waitForLocalLoads)
        return;
    
    // Clean out anything we might be currently evaluating
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
    _quadtree->clearEvals();
    _quadtree->clearFails();

    // Remove nodes until we run out
    Quadtree::NodeInfo remNodeInfo;
    [_loader quadDisplayLayerStartUpdates:self];
    while (_quadtree->leastImportantNode(remNodeInfo,true))
    {
        
        _quadtree->removeTile(remNodeInfo.ident);
        [_loader quadDisplayLayer:self unloadTile:&remNodeInfo];
    }
    waitForLocalLoads = true;
    
    // Add everything at the minLevel back in
    for (int ix=0;ix<1<<minZoom;ix++)
        for (int iy=0;iy<1<<minZoom;iy++)
            _quadtree->addTile(Quadtree::Identifier(ix,iy,minZoom), true, false);
    
    [_loader quadDisplayLayerStartUpdates:self];

    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];

    somethingHappened = true;
}

- (void)reset
{
    // Can only do this if the loader supports it
    if (![_loader respondsToSelector:@selector(reset:scene:)])
        return;
    
    if ([NSThread currentThread] != _layerThread)
    {
        [self performSelector:@selector(reset) onThread:_layerThread withObject:nil waitUntilDone:NO];
        return;
    }
    
    waitForLocalLoads = false;
    
    // Clean out anything we might be currently evaluating
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
    _quadtree->clearEvals();
    _quadtree->clearFails();
    
    // Remove nodes until we run out
    Quadtree::NodeInfo remNodeInfo;
    [_loader quadDisplayLayerStartUpdates:self];
    while (_quadtree->leastImportantNode(remNodeInfo,true))
    {
        _quadtree->removeTile(remNodeInfo.ident);
    }
    
    // Tell the tile loader to reset
    [_loader reset:self scene:_scene];
    
    // Now start loading again
    [self poke];
}

- (void)poke
{
    if ([NSThread currentThread] != _layerThread)
    {
        [self performSelector:@selector(poke) onThread:_layerThread withObject:nil waitUntilDone:NO];
        return;
    }

    [self viewUpdate:viewState];
}

- (void)wakeUp
{
    if ([NSThread currentThread] != _layerThread)
    {
        [self performSelector:@selector(wakeUp) onThread:_layerThread withObject:nil waitUntilDone:NO];
        return;
    }

    // Note: Might be better to check if an eval is scheduled, rather than cancel it
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
    somethingHappened = true;
}

#pragma mark - Quad Tree Importance Delegate

- (double)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(Mbr)theMbr tree:(WhirlyKit::Quadtree *)tree attrs:(NSMutableDictionary *)attrs
{
    return [_dataStructure importanceForTile:ident mbr:theMbr viewInfo:viewState frameSize:Point2f(_renderer.framebufferWidth,_renderer.framebufferHeight) attrs:attrs];
}

@end

