/*
 *  QuadTileBuilder.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/29/18.
 *  Copyright 2012-2018 Saildrone Inc
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

#import "QuadTileBuilder.h"
#import "LoadedTileNew.h"

using namespace WhirlyKit;

@implementation WhirlyKitQuadTileBuilder
{
    WhirlyKitQuadDisplayLayerNew * __weak layer;
    
    TileGeomManager geomManage;
    TileGeomSettings geomSettings;
}

- (id)initWithCoordSys:(WhirlyKit::CoordSystem *)coordSys
{
    self = [super init];
    _debugMode = false;
    _coordSys = coordSys;
    _baseDrawPriority = 0;
    _drawPriorityPerLevel = 1;
    geomSettings.sampleX = 20;
    geomSettings.sampleY = 20;
    geomSettings.topSampleX = 30;
    geomSettings.topSampleY = 40;
    geomSettings.enableGeom = false;
    geomSettings.singleLevel = false;

    return self;
}

- (void)setCoverPoles:(bool)coverPoles
{
    geomManage.coverPoles = coverPoles;
}

- (bool)coverPoles
{
    return geomManage.coverPoles;
}

- (void)setEdgeMatching:(bool)edgeMatch
{
    geomManage.buildSkirts = edgeMatch;
}

- (void)setBaseDrawPriority:(int)baseDrawPriority
{
    geomSettings.baseDrawPriority = baseDrawPriority;
}

- (void)setDrawPriorityPerLevel:(int)drawPriorityPerLevel
{
    geomSettings.drawPriorityPerLevel = drawPriorityPerLevel;
}

- (void)setSingleLevel:(bool)singleLevel
{
    geomSettings.singleLevel = singleLevel;
}

- (bool)edgeMatching
{
    return geomManage.buildSkirts;
}

- (WhirlyKit::LoadedTileNewRef)getLoadedTile:(WhirlyKit::QuadTreeNew::Node)ident
{
    return geomManage.getTile(ident);
}

// MARK: WhirlyKitQuadLoaderNew delegate

- (void)setQuadLayer:(WhirlyKitQuadDisplayLayerNew *)inLayer
{
    layer = inLayer;
    MbrD mbr = MbrD([layer.dataStructure validExtents]);
    geomManage.setup(geomSettings,layer.quadtree, layer.scene->getCoordAdapter(),_coordSys,mbr);
    
    [_delegate setQuadBuilder:self layer:layer];
}

- (WhirlyKit::QuadTreeNew::NodeSet)quadDisplayLayer:(WhirlyKitQuadDisplayLayerNew *)layer loadTiles:(const WhirlyKit::QuadTreeNew::ImportantNodeSet &)loadTiles unLoadTiles:(const WhirlyKit::QuadTreeNew::NodeSet &)unloadTiles updateTiles:(const WhirlyKit::QuadTreeNew::ImportantNodeSet &)updateTiles targetLevel:(int)targetLevel
{
    ChangeSet changes;
    
    TileBuilderDelegateInfo info;
    info.unloadTiles = unloadTiles;
    info.changeTiles = updateTiles;

    QuadTreeNew::NodeSet toKeep;
    if (!unloadTiles.empty()) {
        toKeep = [_delegate quadBuilder:self loadTiles:loadTiles unloadTilesToCheck:unloadTiles targetLevel:targetLevel];
        // Remove the keep nodes and add them to update with very little importance
        for (const QuadTreeNew::Node &node: toKeep) {
            info.unloadTiles.erase(node);
            info.changeTiles.insert(QuadTreeNew::ImportantNode(node,0.0));
        }
    }
    
    // Have the geometry manager add/remove the tiles and deal with changes
    auto tileChanges = geomManage.addRemoveTiles(loadTiles,info.unloadTiles,changes);

    // Tell the delegate what we're up to
    info.targetLevel = targetLevel;
    info.loadTiles = tileChanges.addedTiles;
    info.enableTiles = tileChanges.enabledTiles;
    info.disableTiles = tileChanges.disabledTiles;
    [_delegate quadBuilder:self update:info changes:changes];
    
    if (_debugMode)
    {
        NSLog(@"----- Tiles to add ------");
        for (auto tile : loadTiles)
            NSLog(@"  %d: (%d,%d)",tile.level,tile.x,tile.y);
        NSLog(@"----- Tiles to remove ------");
        for (auto tile : unloadTiles)
            NSLog(@"  %d: (%d,%d)",tile.level,tile.x,tile.y);
        NSLog(@"----- Tiles that changed importance ------");
        for (auto tile : updateTiles)
            NSLog(@"  %d: (%d,%d)",tile.level,tile.x,tile.y);
        NSLog(@"----- Nodes to enable ------");
        for (auto tile : tileChanges.enabledTiles)
            NSLog(@"  %d: (%d,%d)",tile->ident.level,tile->ident.x,tile->ident.y);
        NSLog(@"----- Nodes to disable ------");
        for (auto tile : tileChanges.disabledTiles)
            NSLog(@"  %d: (%d,%d)",tile->ident.level,tile->ident.x,tile->ident.y);
        NSLog(@"----- Tiles to keep -----");
        for (auto tile : toKeep)
            NSLog(@"  %d: (%d,%d)",tile.level,tile.x,tile.y);
        NSLog(@"----- ------------- ------");
    }

    // We need the layer flush to run if we're holding on to nodes
    if (!toKeep.empty() && changes.empty())
        changes.push_back(NULL);

    // Flush out any visual changes
    [layer.layerThread addChangeRequests:changes];
    
    return toKeep;
}

- (void)quadDisplayLayerPreSceneFlush:(WhirlyKitQuadDisplayLayerNew *)layer
{
    [_delegate quadBuilderPreSceneFlush:self];
}

- (void)quadDisplayLayerShutdown:(WhirlyKitQuadDisplayLayerNew * _Nonnull)layer
{
    ChangeSet changes;
    
    geomManage.cleanup(changes);
    [_delegate quadBuilderShutdown:self];

    [layer.layerThread addChangeRequests:changes];
    
    layer = nil;
    _coordSys = nil;
    _delegate = nil;
}


- (TileBuilderDelegateInfo)getLoadingState
{
    TileBuilderDelegateInfo info;
    
    info.loadTiles = geomManage.getAllTiles();
    
    return info;
}

@end
