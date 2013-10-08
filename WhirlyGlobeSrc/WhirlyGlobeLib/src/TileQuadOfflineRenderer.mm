/*
 *  TileQuadOfflineRenderer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/7/13.
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

#import "TileQuadOfflineRenderer.h"

using namespace WhirlyKit;

@implementation WhirlyKitQuadTileOfflineLoader
{
    NSString *_name;
    NSObject<WhirlyKitQuadTileImageDataSource> *_imageSource;
}

- (id)initWithName:(NSString *)name dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)imageSource
{
    self = [super init];
    if (!self)
        return nil;

    _name = name;
    _imageSource = imageSource;
    _sizeX = _sizeY = 1024;
    _mbr = Mbr(GeoCoord::CoordFromDegrees(0.0,0.0),GeoCoord::CoordFromDegrees(1.0, 1.0));
    
    return self;
}

#pragma mark - WhirlyKitQuadLoader

- (void)shutdownLayer:(WhirlyKitQuadDisplayLayer *)layer scene:(WhirlyKit::Scene *)scene
{
    
}

- (bool)isReady
{
    return true;
}

- (void)setQuadLayer:(WhirlyKitQuadDisplayLayer *)layer
{
    
}

- (void)quadDisplayLayerStartUpdates:(WhirlyKitQuadDisplayLayer *)layer
{
}

- (void)quadDisplayLayerEndUpdates:(WhirlyKitQuadDisplayLayer *)layer
{
}

- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer loadTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    NSLog(@"Load tile: %d: (%d,%d)",tileInfo.ident.level,tileInfo.ident.x,tileInfo.ident.y);
}

- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer unloadTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    NSLog(@"Unload tile: %d: (%d,%d)",tileInfo.ident.level,tileInfo.ident.x,tileInfo.ident.y);
}

- (bool)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer canLoadChildrenOfTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    return true;
}

- (void)dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)dataSource loadedImage:(id)loadImage forLevel:(int)level col:(int)col row:(int)row
{
    
}

@end
