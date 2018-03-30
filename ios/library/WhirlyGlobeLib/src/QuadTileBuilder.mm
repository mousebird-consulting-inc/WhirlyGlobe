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
    _coordSys = coordSys;
    
    return self;
}

- (void)setQuadLayer:(WhirlyKitQuadDisplayLayerNew *)inLayer
{
    layer = inLayer;
    geomSettings.lineMode = true;
    geomManage.coverPoles = false;
    MbrD mbr = MbrD([layer.dataStructure validExtents]);
    geomManage.setup(layer.quadtree, layer.scene->getCoordAdapter(),_coordSys,mbr);
}

- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayerNew *)layer loadTiles:(const QuadTreeNew::NodeSet &)tiles
{
    ChangeSet changes;
    geomManage.addTiles(geomSettings, tiles, changes);
    [layer.layerThread addChangeRequests:changes];
}

- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayerNew *)layer unLoadTiles:(const QuadTreeNew::NodeSet &)tiles
{
    ChangeSet changes;
    geomManage.removeTiles(tiles, changes);
    [layer.layerThread addChangeRequests:changes];
}

@end
