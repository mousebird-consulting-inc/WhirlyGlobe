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
    
    /// Nodes being evaluated for loading
    WhirlyKit::QuadNodeInfoSet nodesForEval;
    
    /// If set the eval step gets very aggressive about loading tiles.
    /// This will slow down the layer thread, but makes the quad layer appear faster
    bool greedyMode;
    
    /// State of the view the last time we were called
    WhirlyKitViewState *viewState;
    
    /// Frame times for metered mode
    NSTimeInterval frameStart,frameInterval;
    
    /// Set if we're waiting for local loads (e.g. a reload)
    bool waitForLocalLoads;
    
    // In metered mode, the last time we flushed data to the scene
    NSTimeInterval lastFlush;
    
    // In metered mode, we'll only flush if something happened
    bool somethingHappened;
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
        _renderer = inRenderer;
        _lineMode = false;
        _drawEmpty = false;
        _debugMode = false;
        greedyMode = false;
        _meteredMode = true;
        _fullLoad = false;
        _fullLoadTimeout = 4.0;
        waitForLocalLoads = false;
        somethingHappened = false;
    }
    
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    if (_quadtree)
        delete _quadtree;
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
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(frameStart:) name:kWKFrameMessage object:nil];
        [_loader quadDisplayLayerStartUpdates:self];
    }

    if (_fullLoad)
        waitForLocalLoads = true;
}

- (void)shutdown
{
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
- (void)frameStart:(NSNotification *)note
{
    WhirlyKitFrameMessage *msg = note.object;
    
    // If it's not coming from our renderer, we can ignore it
    if (msg.renderer != _renderer)
        return;
    
    frameStart = msg.frameStart;
    frameInterval = msg.frameInterval;
    
    if (_meteredMode)
        [self performSelector:@selector(frameStartThread) onThread:_layerThread withObject:nil waitUntilDone:NO];
}

- (void)frameStartThread
{
    NSTimeInterval howLong = CFAbsoluteTimeGetCurrent()-frameStart+AvailableFrame*frameInterval;
    if (howLong > 0.0)
    {
        [_loader quadDisplayLayerStartUpdates:self];
        [self performSelector:@selector(frameEndThread) withObject:nil afterDelay:howLong];
    }
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
    
    // Check if we should even be doing an update
    if ([_loader respondsToSelector:@selector(shouldUpdate:initial:)])
        if (![_loader shouldUpdate:inViewState initial:(viewState == nil)])
            return;
    
    if ([_dataStructure respondsToSelector:@selector(newViewState:)])
        [_dataStructure newViewState:inViewState];
    
    if (_fullLoad)
        waitForLocalLoads = true;
        
    viewState = inViewState;
    nodesForEval.clear();
    _quadtree->reevaluateNodes();
    
    // Add everything at the minLevel back in
    for (int ix=0;ix<1<<minZoom;ix++)
        for (int iy=0;iy<1<<minZoom;iy++)
        {
            Quadtree::NodeInfo thisNode = _quadtree->generateNode(Quadtree::Identifier(ix,iy,minZoom));
            nodesForEval.insert(thisNode);
        }
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
}

// Dump out info about what we've got loaded in
- (void)dumpInfo
{
//    NSLog(@"***Loaded Tiles***");
//    for (LoadedTileSet::iterator it = tileSet.begin();
//         it != tileSet.end(); ++it)
//    {
//        (*it)->Print(quadtree);
//    }
//    NSLog(@"******");
    
    _quadtree->Print();
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
    bool localActivity = !nodesForEval.empty();
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

    // Look for nodes to remove
    Quadtree::NodeInfo remNodeInfo;
    while (_quadtree->leastImportantNode(remNodeInfo))
    {
        _quadtree->removeTile(remNodeInfo.ident);
        [_loader quadDisplayLayer:self unloadTile:remNodeInfo];
        
        didSomething = true;
    }
    
    if (!nodesForEval.empty())
    {
        // Let the loader know we're about to do some updates        
        while (!nodesForEval.empty())
        {
            // Grab the first node.
            QuadNodeInfoSet::iterator nodeIt = nodesForEval.end();
            nodeIt--;
            Quadtree::NodeInfo nodeInfo = *nodeIt;
            nodesForEval.erase(nodeIt);
            
            // The quad tree will take this node over an existing one
            bool isLoaded = _quadtree->isTileLoaded(nodeInfo.ident);
            if (isLoaded || _quadtree->willAcceptTile(nodeInfo))
            {
                if (!isLoaded)
                {
                    // Tell the quad tree what we're up to
                    std::vector<Quadtree::Identifier> tilesToRemove;
                    _quadtree->addTile(nodeInfo, tilesToRemove);
                                
                    [_loader quadDisplayLayer:self loadTile:nodeInfo ];
                                    
                    // Remove the old tiles
                    for (unsigned int ii=0;ii<tilesToRemove.size();ii++)
                    {
                        Quadtree::Identifier &thisIdent = tilesToRemove[ii];
//                    NSLog(@"Quad tree removed (%d,%d,%d)",thisIdent.x,thisIdent.y,thisIdent.level);
                        
                        Quadtree::NodeInfo remNodeInfo = _quadtree->generateNode(thisIdent);
                        [_loader quadDisplayLayer:self unloadTile:remNodeInfo];           
                    }
//            NSLog(@"Quad loaded node (%d,%d,%d) = %.4f",nodeInfo.ident.x,nodeInfo.ident.y,nodeInfo.ident.level,nodeInfo.importance);            
                } else {
                    // It is loaded (as far as we're concerned), so we need to know if we can traverse below that
                    if (nodeInfo.ident.level < maxZoom && [_loader quadDisplayLayer:self canLoadChildrenOfTile:nodeInfo])
                    {
                        std::vector<Quadtree::NodeInfo> childNodes;
                        _quadtree->generateChildren(nodeInfo.ident, childNodes);
                        nodesForEval.insert(childNodes.begin(),childNodes.end());                
                    }
                }
            } else
            {
//        NSLog(@"Quad rejecting node (%d,%d,%d) = %.4f",nodeInfo.ident.x,nodeInfo.ident.y,nodeInfo.ident.level,nodeInfo.importance);
            }

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

    // Let the loader know we're done with this eval step
    if (_meteredMode || [self waitingForLocalLoads] || didSomething)
    {
        if ([_loader respondsToSelector:@selector(updateWithoutFlush)])
            [_loader updateWithoutFlush];
    } else
        [_loader quadDisplayLayerEndUpdates:self];

    if (_debugMode)
        [self dumpInfo];
    
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
}

// This is called by the loader when it finished loading a tile
// Once loaded we can try the children
- (void)loader:(NSObject<WhirlyKitQuadLoader> *)loader tileDidLoad:(WhirlyKit::Quadtree::Identifier)tileIdent
{
    // Note: Need to check that we still care about this load
    
    if (tileIdent.level < maxZoom)
    {
        // Make sure we still want this one
        if (!_quadtree->isTileLoaded(tileIdent))
            return;
        
        // Now try the children
        std::vector<Quadtree::NodeInfo> childNodes;
        _quadtree->generateChildren(tileIdent, childNodes);
        nodesForEval.insert(childNodes.begin(),childNodes.end());
    }

    // Make sure we actually evaluate them
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
    
    somethingHappened = true;
}

// Tile failed to load.
// At the moment we don't care, but we won't look at the children
- (void)loader:(NSObject<WhirlyKitQuadLoader> *)loader tileDidNotLoad:(WhirlyKit::Quadtree::Identifier)tileIdent
{
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
    nodesForEval.clear();

    // Remove nodes until we run out
    Quadtree::NodeInfo remNodeInfo;
    [_loader quadDisplayLayerStartUpdates:self];
    while (_quadtree->leastImportantNode(remNodeInfo,true))
    {
        
        _quadtree->removeTile(remNodeInfo.ident);
        [_loader quadDisplayLayer:self unloadTile:remNodeInfo];        
    }
    waitForLocalLoads = true;
    
    // Add everything at the minLevel back in
    for (int ix=0;ix<1<<minZoom;ix++)
        for (int iy=0;iy<1<<minZoom;iy++)
        {
            Quadtree::NodeInfo thisNode = _quadtree->generateNode(Quadtree::Identifier(ix,iy,minZoom));
            nodesForEval.insert(thisNode);
        }
    
    [_loader quadDisplayLayerStartUpdates:self];

    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];

    somethingHappened = true;
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

