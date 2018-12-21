/*
 *  QuadDisplayLayerNew.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/26/18.
 *  Copyright 2011-2018 mousebird consulting
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

#import "QuadDisplayLayerNew.h"

using namespace WhirlyKit;

@interface WhirlyKitQuadDisplayLayerNew()

- (double)importanceFor:(const QuadTreeNew::Node &)node;
- (double)visibility:(const QuadTreeNew::Node &)node;

@end

namespace WhirlyKit
{

// Display layer version of the quad tree
class DispLayerQuadTree : public QuadTreeNew
{
public:
    DispLayerQuadTree(const MbrD &mbr,int minLevel,int maxLevel,WhirlyKitQuadDisplayLayerNew *dispLayer)
    : QuadTreeNew(mbr,minLevel,maxLevel), dispLayer(dispLayer)
    {
    }
    
    ~DispLayerQuadTree()
    {
        
    }

protected:
    WhirlyKitQuadDisplayLayerNew * __weak dispLayer;
    
    // Calculate importance for a given node
    double importance(const Node &node)
    {
        return [dispLayer importanceFor:node];
    }
    
    // Pure visibility check
    virtual bool visible(const Node &node) {
        return [dispLayer visibility:node];
    }
};

}

@implementation WhirlyKitQuadDisplayLayerNew
{
    int minZoom,maxZoom;
    WhirlyKitViewState *viewState;
    QuadTreeNew::ImportantNodeSet currentNodes;
    std::vector<int> levelsToLoad;
}

- (id)initWithDataSource:(NSObject<WhirlyKitQuadDataStructure> *)inDataStructure loader:(NSObject<WhirlyKitQuadLoaderNew> *)loader renderer:(WhirlyKitSceneRendererES *)inRenderer
{
    self = [super init];
    if (self)
    {
        _dataStructure = inDataStructure;
        _loader = loader;
        _coordSys = [_dataStructure coordSystem];
        _mbr = [_dataStructure validExtents];
        minZoom = [_dataStructure minZoom];
        maxZoom = [_dataStructure maxZoom];
        _maxTiles = 128;
        _viewUpdatePeriod = 0.1;
        Mbr mbr = [_dataStructure totalExtents];
        MbrD mbrD(mbr);
        _quadtree = new DispLayerQuadTree(mbrD,minZoom,maxZoom,self);
        _renderer = (WhirlyKitSceneRendererES2 *)inRenderer;
        _singleLevel = false;
        _levelLoads = nil;
        _enable = true;
    }

    return self;
}

- (void)dealloc
{
    if (_quadtree)
        delete _quadtree;
    _quadtree = NULL;
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(Scene *)inScene
{
    _layerThread = inLayerThread;
    _scene = inScene;

    // We want view updates, but only every so often
    if (_layerThread.viewWatcher)
        [_layerThread.viewWatcher addWatcherTarget:self selector:@selector(viewUpdate:) minTime:_viewUpdatePeriod minDist:0.0 maxLagTime:10.0];
    
    [_loader setQuadLayer:self];

    for (NSNumber *num in _levelLoads)
        levelsToLoad.push_back([num intValue]);
}

- (void)teardown
{
    [_loader quadDisplayLayerShutdown:self];
    
    [_dataStructure teardown];
    _dataStructure = nil;

    _scene = NULL;

    if (_quadtree)
        delete _quadtree;
    _quadtree = NULL;
}

static const float DelayPeriod = 0.1;

// Called after some period to sweep up removes
- (void)delayCheck
{
    if (!viewState)
        return;

    [self viewUpdate:viewState];
}

// Called periodically when the user moves, but not too often
- (void)viewUpdate:(WhirlyKitViewState *)inViewState
{
    if (!_scene)
        return;
 
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
    
    if ([_dataStructure respondsToSelector:@selector(newViewState:)])
        [_dataStructure newViewState:inViewState];

    viewState = inViewState;

    // Nodes to load are different for single level vs regular loading
    QuadTreeNew::ImportantNodeSet newNodes;
    int targetLevel = -1;
    if (_singleLevel) {
        std::tie(targetLevel,newNodes) = _quadtree->calcCoverageVisible(_minImportancePerLevel, _maxTiles, levelsToLoad);
    } else {
        newNodes = _quadtree->calcCoverageImportance(_minImportancePerLevel,_maxTiles,true);
        // Just take the highest level as target
        for (auto node : newNodes)
            targetLevel = std::max(targetLevel,node.level);
    }
    
//    NSLog(@"Selected level %d for %d nodes",targetLevel,(int)newNodes.size());
//    for (auto node: newNodes) {
//        NSLog(@" %d: (%d,%d), import = %f",node.level,node.x,node.y,node.importance);
//    }

    QuadTreeNew::ImportantNodeSet toAdd,toUpdate;
    QuadTreeNew::NodeSet toRemove;

    // Need a version of new and old that has no importance values, since those change
    QuadTreeNew::NodeSet testNewNodes;
    for (auto node : newNodes)
        testNewNodes.insert(node);
    QuadTreeNew::NodeSet testCurrentNodes;
    for (auto node : currentNodes)
        testCurrentNodes.insert(node);

    // Nodes to remove
    for (auto node : currentNodes)
        if (testNewNodes.find(node) == testNewNodes.end())
            toRemove.insert(node);

    // Nodes to add and nodes to update importance for
    for (auto node : newNodes)
        if (testCurrentNodes.find(node) == testCurrentNodes.end())
            toAdd.insert(node);
        else
            toUpdate.insert(node);
    
    QuadTreeNew::NodeSet removesToKeep;
    removesToKeep = [_loader quadDisplayLayer:self loadTiles:toAdd unLoadTiles:toRemove updateTiles:toUpdate targetLevel:targetLevel];

    // If the load is sitting on unloads, check back with it in a bit
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(delayCheck) object:nil];
    if (!removesToKeep.empty())
        [self performSelector:@selector(delayCheck) withObject:nil afterDelay:DelayPeriod];
    
    currentNodes = newNodes;
    for (auto node : removesToKeep) {
        currentNodes.insert(QuadTreeNew::ImportantNode(node,0.0));
    }
}

- (void)preSceneFlush:(WhirlyKitLayerThread *)layerThread
{
    [_loader quadDisplayLayerPreSceneFlush:self];
}

- (double)importanceFor:(const QuadTreeNew::Node &)node
{
    Quadtree::Identifier ident;
    ident.level = node.level;  ident.x = node.x;  ident.y = node.y;
    Point2d ll,ur;
    MbrD mbrD = _quadtree->generateMbrForNode(node);
    Mbr mbr(mbrD);
    
    // Is this a valid tile?
    if (!_mbr.inside(mbr.mid())) {
        return -1.0;
    }

    // Note: Add back the mutable attributes?
    return [_dataStructure importanceForTile:ident mbr:mbr viewInfo:viewState frameSize:Point2f(_renderer.framebufferWidth,_renderer.framebufferHeight) attrs:nil];
}

- (double)visibility:(const QuadTreeNew::Node &)node
{
    Quadtree::Identifier ident;
    ident.level = node.level;  ident.x = node.x;  ident.y = node.y;
    Point2d ll,ur;
    MbrD mbrD = _quadtree->generateMbrForNode(node);
    Mbr mbr(mbrD);
    
    // Is this a valid tile?
    if (!_mbr.inside(mbr.mid())) {
        return 0.0;
    }
    
    // Note: Add back the mutable attributes?
    return [_dataStructure visibilityForTile:ident mbr:mbr viewInfo:viewState frameSize:Point2f(_renderer.framebufferWidth,_renderer.framebufferHeight) attrs:nil];
}

@end
