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

namespace WhirlyKit
{

class OfflineTile
{
public:
    OfflineTile() : isLoading(false), placeholder(false) { };
    OfflineTile(const WhirlyKit::Quadtree::Identifier &ident) : ident(ident), isLoading(false), placeholder(false) { }
    OfflineTile(std::vector<WhirlyKitLoadedImage *>loadImages) : isLoading(false), placeholder(false) { }
    ~OfflineTile()
    {
    }
    
    // Details of which node we're representing
    WhirlyKit::Quadtree::Identifier ident;

    /// Set if this is just a placeholder (no geometry)
    bool placeholder;
    /// Set if this tile is in the process of loading
    bool isLoading;
    
    std::vector<WhirlyKitLoadedImage *> images;
};

typedef struct
{
    /// Comparison operator based on node identifier
    bool operator() (const OfflineTile *a,const OfflineTile *b)
    {
        return a->ident < b->ident;
    }
} OfflineTileSorter;

/// A set that sorts loaded MB Tiles by Quad tree identifier
typedef std::set<OfflineTile *,OfflineTileSorter> OfflineTileSet;

}

@implementation WhirlyKitQuadTileOfflineLoader
{
    WhirlyKitQuadDisplayLayer *_quadLayer;
    NSString *_name;
    NSObject<WhirlyKitQuadTileImageDataSource> *_imageSource;
    OfflineTileSet tiles;
    int numFetches;
    bool renderScheduled;
}

- (id)initWithName:(NSString *)name dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)imageSource
{
    self = [super init];
    if (!self)
        return nil;

    _name = name;
    _imageSource = imageSource;
    _numImages = 1;
    _sizeX = _sizeY = 1024;
    _mbr = Mbr(GeoCoord::CoordFromDegrees(0.0,0.0),GeoCoord::CoordFromDegrees(1.0, 1.0));
    renderScheduled = false;
    
    return self;
}

- (void)clear
{
    for (OfflineTileSet::iterator it = tiles.begin();it != tiles.end();++it)
        delete *it;
    tiles.clear();
}

- (void)dealloc
{
    [self clear];
}

- (void)setPeriod:(NSTimeInterval)period
{
    _period = period;
    
    if (!_quadLayer)
        return;
    
    if (_period > 0.0 && !renderScheduled)
    {
        renderScheduled = true;
        [self performSelector:@selector(imageRenderPeriodic) withObject:nil afterDelay:_period];
    }
}

- (void)setMbr:(WhirlyKit::Mbr &)mbr
{
    if (!_quadLayer)
        return;

    if (mbr.ll().x() < 0 && mbr.ur().x() > 0)
    {
        float tmp = mbr.ll().x();
        mbr.ll().x() = mbr.ur().x();
        mbr.ur().x() = tmp;
        mbr.ur().x() += 2*M_PI;
    }
    
    _mbr = mbr;
    [self performSelector:@selector(imageRenderImmediate) onThread:_quadLayer.layerThread withObject:nil waitUntilDone:NO];
}

- (void)imageRenderImmediate
{
    [self imageRender];
}

- (void)imageRenderPeriodic
{
    renderScheduled = false;

    [self imageRender];
    
    if (_period > 0.0 && !renderScheduled)
    {
        renderScheduled = true;
        [self performSelector:@selector(imageRenderPeriodic) withObject:nil afterDelay:_period];
    }
}

- (void)imageRender
{
    if (_outputDelegate)
    {
        int sizeX = _sizeX;
        int sizeY = _sizeY;
        Mbr mbr = _mbr;

        // Note: Assuming geographic or spherical mercator
        GeoMbr geoMbr(GeoCoord(mbr.ll().x(), mbr.ll().y()),GeoCoord(mbr.ur().x(),mbr.ur().y()));
        std::vector<Mbr> testMbrs;
        if (geoMbr.ur().x() > M_PI)
        {
            testMbrs.push_back(Mbr(geoMbr.ll(),Point2f((float)M_PI,geoMbr.ur().y())));
            testMbrs.push_back(Mbr(Point2f((float)(M_PI),geoMbr.ll().y()),geoMbr.ur()));
        } else {
            testMbrs.push_back(Mbr(geoMbr.ll(),geoMbr.ur()));
        }
        
        NSMutableArray *images = [NSMutableArray array];
        
        // Draw each entry in the image stack individually
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        CGContextRef theContext = CGBitmapContextCreate(NULL, sizeX, sizeY, 8, sizeX * 4, colorSpace, kCGImageAlphaPremultipliedLast);
        for (unsigned int ii=0;ii<_numImages;ii++)
        {
            CGContextSetRGBFillColor(theContext, 1, 0, 0, 1);
            CGContextFillRect(theContext, CGRectMake(0, 0, sizeX, sizeY));
            // Work through the tiles, drawing as we go
            for (OfflineTileSet::iterator it = tiles.begin(); it != tiles.end(); ++it)
            {
                OfflineTile *tile = *it;
                if (tile->isLoading)
                    continue;
                // Scale the extents to the output image
                Mbr tileMbr[2];
                tileMbr[0] = _quadLayer.quadtree->generateMbrForNode(tile->ident);
                bool overlaps = tileMbr[0].overlaps(testMbrs[0]);
                if (testMbrs.size() > 1 && !overlaps)
                {
                    tileMbr[1] = tileMbr[0];
                    tileMbr[1].ll().x() += 2*M_PI;
                    tileMbr[1].ur().x() += 2*M_PI;
                    overlaps = tileMbr[1].overlaps(testMbrs[1]);
                }
                if (!overlaps)
                    continue;
                
                for (unsigned int jj=0;jj<testMbrs.size();jj++)
                {
                    Mbr &testMbr = testMbrs[jj];
                    if (!tileMbr[jj].overlaps(testMbr))
                        continue;
                    
                    Point2f org;
                    org.x() = sizeX * (tileMbr[jj].ll().x() - mbr.ll().x()) / (mbr.ur().x()-mbr.ll().x());
                    org.y() = sizeY * (tileMbr[jj].ll().y() - mbr.ll().y()) / (mbr.ur().y()-mbr.ll().y());
                    Point2f span;
                    span.x() = sizeX * (tileMbr[jj].ur().x()-tileMbr[jj].ll().x()) / (mbr.ur().x()-mbr.ll().x());
                    span.y() = sizeY * (tileMbr[jj].ur().y()-tileMbr[jj].ll().y()) / (mbr.ur().y()-mbr.ll().y());
                    
                    // Find the right input image
                    UIImage *imageToDraw = nil;
                    if (ii < tile->images.size())
                    {
                        imageToDraw = (UIImage *)tile->images[ii].imageData;
                        if ([imageToDraw isKindOfClass:[NSData class]])
                            imageToDraw = [UIImage imageWithData:(NSData *)imageToDraw];
                        if (![imageToDraw isKindOfClass:[UIImage class]])
                            imageToDraw = nil;
                    }
                    
                    if (imageToDraw)
                        CGContextDrawImage(theContext, CGRectMake(org.x(),org.y(),span.x(),span.y()), imageToDraw.CGImage);
                }
            }
            
            CGImageRef imageRef = CGBitmapContextCreateImage(theContext);
            UIImage *image = [UIImage imageWithCGImage:imageRef];
            CGImageRelease(imageRef);
            if (image)
                [images addObject:image];
        }
        CGContextRelease(theContext);
        CGColorSpaceRelease(colorSpace);
        
        [_outputDelegate loader:self image:images mbr:mbr];
    }
}

#pragma mark - WhirlyKitQuadLoader

- (void)shutdownLayer:(WhirlyKitQuadDisplayLayer *)layer scene:(WhirlyKit::Scene *)scene
{
    [self clear];
}

- (bool)isReady
{
    return (numFetches <= [_imageSource maxSimultaneousFetches]);
}

- (void)setQuadLayer:(WhirlyKitQuadDisplayLayer *)layer
{
    _quadLayer = layer;
    
    if (_period > 0.0 && !renderScheduled) {
        renderScheduled = true;
        [self performSelector:@selector(imageRenderPeriodic) withObject:nil afterDelay:_period];
    }
}

- (void)quadDisplayLayerStartUpdates:(WhirlyKitQuadDisplayLayer *)layer
{
}

- (void)quadDisplayLayerEndUpdates:(WhirlyKitQuadDisplayLayer *)layer
{
}

- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer loadTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    OfflineTile *newTile = new OfflineTile(tileInfo.ident);
    newTile->isLoading = true;
    
    tiles.insert(newTile);
    
    [_imageSource quadTileLoader:self startFetchForLevel:tileInfo.ident.level col:tileInfo.ident.x row:tileInfo.ident.y attrs:tileInfo.attrs];
    numFetches++;
}

- (OfflineTile *)getTile:(const WhirlyKit::Quadtree::Identifier)ident
{
    OfflineTile dummyTile(ident);
    OfflineTileSet::iterator it = tiles.find(&dummyTile);
    if (it == tiles.end())
        return nil;
    
    return *it;
}

- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer unloadTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    OfflineTile dummyTile(tileInfo.ident);
    OfflineTileSet::iterator it = tiles.find(&dummyTile);
    if (it != tiles.end())
    {
        OfflineTile *theTile = *it;
        delete theTile;
        tiles.erase(it);
    }
}

- (bool)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer canLoadChildrenOfTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    OfflineTile *tile = [self getTile:tileInfo.ident];
    if (!tile)
        return false;
    
    return !tile->isLoading;
}

- (void)dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)dataSource loadedImage:(id)loadTile forLevel:(int)level col:(int)col row:(int)row
{
    numFetches--;
    Quadtree::Identifier tileIdent(col,row,level);
    OfflineTile *tile = [self getTile:tileIdent];
    if (!tile)
        return;

    tile->isLoading = false;

    // Assemble the iamges
    std::vector<WhirlyKitLoadedImage *> loadImages;
    if ([loadTile isKindOfClass:[WhirlyKitLoadedImage class]])
        loadImages.push_back(loadTile);
    else if ([loadTile isKindOfClass:[WhirlyKitLoadedTile class]])
    {
        WhirlyKitLoadedTile *toLoad = loadTile;
        
        for (WhirlyKitLoadedImage *loadImage in toLoad.images)
            loadImages.push_back(loadImage);
    }
    if (_numImages != loadImages.size())
    {
        if (loadTile)
            NSLog(@"TileQuadLoader: Got %ld images in callback, but was expecting %d.  Punting tile.",loadImages.size(),_numImages);
        [_quadLayer loader:self tileDidNotLoad:tileIdent];
        return;
    }
    tile->images = loadImages;

//    NSLog(@"Loaded tile %d: (%d,%d)",level,col,row);
    [_quadLayer loader:self tileDidLoad:tileIdent];
}


@end
