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
    WhirlyKitQuadDisplayLayerNew *dispLayer;
    
    // Calculate importance for a given node
    double importance(const Node &node)
    {
        return [dispLayer importanceFor:node];
    }
};

}

@implementation WhirlyKitQuadDisplayLayerNew
{
    int minZoom,maxZoom;
    WhirlyKitViewState *viewState;
    QuadTreeNew::ImportantNodeSet currentNodes;
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
        _minImportance = 1.0;
        _viewUpdatePeriod = 0.1;
        Mbr mbr = [_dataStructure totalExtents];
        MbrD mbrD(mbr);
        _quadtree = new DispLayerQuadTree(mbrD,minZoom,maxZoom,self);
        _renderer = (WhirlyKitSceneRendererES2 *)inRenderer;
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
}

- (void)teardown
{
    [_dataStructure teardown];
    _dataStructure = nil;

    _scene = NULL;
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

    // What should be present
    auto newNodes = _quadtree->calcCoverage(_minImportance,_maxTiles,true);
    QuadTreeNew::ImportantNodeSet toAdd;
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
    
    // Nodes to add
    for (auto node : newNodes)
        if (testCurrentNodes.find(node) == testCurrentNodes.end())
            toAdd.insert(node);

    if (!toRemove.empty())
        [_loader quadDisplayLayer:self unLoadTiles:toRemove];
    if (!toAdd.empty())
        [_loader quadDisplayLayer:self loadTiles:toAdd];
    
    currentNodes = newNodes;
}

- (double)importanceFor:(const QuadTreeNew::Node &)node
{
    Quadtree::Identifier ident;
    ident.level = node.level;  ident.x = node.x;  ident.y = node.y;
    Point2d ll,ur;
    MbrD mbrD = _quadtree->generateMbrForNode(node);
    Mbr mbr(mbrD);

    // Note: Add back the mutable attributes?
    return [_dataStructure importanceForTile:ident mbr:mbr viewInfo:viewState frameSize:Point2f(_renderer.framebufferWidth,_renderer.framebufferHeight) attrs:nil];
}

@end
