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
#import "FlatMath.h"

using namespace WhirlyKit;

@implementation WhirlyKitQuadTileOfflineImage
@end

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
    
    // Return the size of this tile
    void GetTileSize(int &numX,int &numY)
    {
        numX = numY = 0;
        if (images.size() > 0)
        {
            numX = images[0].width;
            numY = images[0].height;
        }
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
    bool renderScheduled,immediateScheduled;
    CFTimeInterval lastRender;
    bool somethingChanged;
    int currentMbr;
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
    immediateScheduled = false;
    _on = true;
    _autoRes = true;
    _previewLevels = -1;
    somethingChanged = true;
    
    return self;
}

- (void)clear
{
    for (OfflineTileSet::iterator it = tiles.begin();it != tiles.end();++it)
        delete *it;
    tiles.clear();
    somethingChanged = true;
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

    if (mbr.ll().x() < 0 && mbr.ur().x() > 0 && (mbr.ur().x() - mbr.ll().x() > M_PI))
    {
        float tmp = mbr.ll().x();
        mbr.ll().x() = mbr.ur().x();
        mbr.ur().x() = tmp;
        mbr.ur().x() += 2*M_PI;
    }
    
    @synchronized(self)
    {
        _mbr = mbr;
        currentMbr++;
    }
    somethingChanged = true;
    if (!immediateScheduled)
    {
        [self performSelector:@selector(imageRenderImmediate) onThread:_quadLayer.layerThread withObject:nil waitUntilDone:NO];
        immediateScheduled = true;
    }
}

- (void)imageRenderImmediate
{
//    NSLog(@"Render:: Immediate");
    immediateScheduled = false;
    
    if (_on)
    {
        [self imageRenderToLevel:_previewLevels];
        if (_previewLevels > 0)
        {
            lastRender = 0;
            if (!renderScheduled)
            {
                [self performSelector:@selector(imageRenderPeriodic)];
                renderScheduled = true;
            }
        }
    }
}

- (void)imageRenderPeriodic
{
    renderScheduled = false;

    if (_on && somethingChanged)
    {
//        NSLog(@"Render:: Periodic");
        CFTimeInterval now = CFAbsoluteTimeGetCurrent();
        if (now - lastRender >= _period)
            [self imageRenderToLevel:-1];
    }
    
    if (_period > 0.0 && !renderScheduled)
    {
        renderScheduled = true;
        [self performSelector:@selector(imageRenderPeriodic) withObject:nil afterDelay:_period];
    }
}

- (CGSize)calculateSize
{
    if (_autoRes)
    {
        Point2f imageRes(MAXFLOAT,MAXFLOAT);
        
        Mbr mbr;
        @synchronized(self)
        {
            mbr = _mbr;
        }

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

        // Work our way through the tiles looking for overlaps
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
                
                // Figure out how big a pixel is
                int numX,numY;
                tile->GetTileSize(numX,numY);
                if (numX <= 0 || numY <= 0)
                    continue;
                Point2f texSize = tileMbr[0].ur() - tileMbr[0].ll();
                texSize.x() /= numX;  texSize.y() /= numY;
                imageRes.x() = std::min(imageRes.x(),texSize.x());
                imageRes.y() = std::min(imageRes.y(),texSize.y());
            }
        }
        
        Point2f numPix = mbr.ur()-mbr.ll();
        numPix.x() /= imageRes.x();  numPix.y() /= imageRes.y();
        
        numPix.x() = std::min((float)_sizeX,numPix.x());  numPix.y() = std::min((float)_sizeY,numPix.y());
        numPix.x() = std::max(numPix.x(),16.f);  numPix.y() = std::max(numPix.y(),16.f);
        return CGSizeMake((int)numPix.x(), (int)numPix.y());
    } else
        return CGSizeMake(_sizeX, _sizeY);
}

- (void)imageRenderToLevel:(int)deep
{
    if (_outputDelegate)
    {
        lastRender = CFAbsoluteTimeGetCurrent();
        int whichMbr;
        
        Mbr mbr;
        @synchronized(self)
        {
            mbr = _mbr;
            whichMbr = currentMbr;
        }

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
        
        CGSize texSize = [self calculateSize];
//        NSLog(@"Tex Size = (%f,%f)",texSize.width,texSize.height);
        
        // Draw each entry in the image stack individually
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        CGContextRef theContext = CGBitmapContextCreate(NULL, texSize.width, texSize.height, 8, texSize.width * 4, colorSpace, kCGImageAlphaPremultipliedLast);
        int numRenderedTiles = 0;
        for (unsigned int ii=0;ii<_numImages;ii++)
        {
            // Note: Debugging
            CGContextSetRGBFillColor(theContext, 1, 0, 0, 1);
            CGContextFillRect(theContext, CGRectMake(0, 0, texSize.width, texSize.height));
            // Work through the tiles, drawing as we go
            for (OfflineTileSet::iterator it = tiles.begin(); it != tiles.end(); ++it)
            {
                OfflineTile *tile = *it;
                if (tile->isLoading)
                    continue;
                if (deep > 0 && tile->ident.level > deep)
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
                    org.x() = texSize.width * (tileMbr[jj].ll().x() - mbr.ll().x()) / (mbr.ur().x()-mbr.ll().x());
                    org.y() = texSize.height * (tileMbr[jj].ll().y() - mbr.ll().y()) / (mbr.ur().y()-mbr.ll().y());
                    Point2f span;
                    span.x() = texSize.width * (tileMbr[jj].ur().x()-tileMbr[jj].ll().x()) / (mbr.ur().x()-mbr.ll().x());
                    span.y() = texSize.height * (tileMbr[jj].ur().y()-tileMbr[jj].ll().y()) / (mbr.ur().y()-mbr.ll().y());
                    
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
                numRenderedTiles++;
                
                // If this happens, they've changed the MBR while we were working on this one.  Punt.
                if (whichMbr != currentMbr)
                {
                    CGContextRelease(theContext);
                    CGColorSpaceRelease(colorSpace);
//                    NSLog(@"Aborted render");
                    return;
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
        
//        NSLog(@"Rendered %d tiles of %d",numRenderedTiles,(int)tiles.size());
        
        // Convert the images into OpenGL ES textures
        bool aborted = false;
        ChangeSet changes;
        std::vector<WhirlyKit::SimpleIdentity> texIDs;
        for (UIImage *image in images)
        {
            Texture *tex = new Texture("TileQuadOfflineRenderer",image,true);
            texIDs.push_back(tex->getId());
            tex->createInGL(_quadLayer.scene->getMemManager());
            changes.push_back(new AddTextureReq(tex));
            if (whichMbr != currentMbr)
            {
                aborted = true;
                break;
            }
        }
        
        // The texture setup can take a while, so let's be ready to abort here
        if (aborted)
        {
            for (unsigned int ii=0;ii<changes.size();ii++)
            {
                AddTextureReq *texReq = (AddTextureReq *)changes[ii];
                texReq->getTex()->destroyInGL(_quadLayer.scene->getMemManager());
                delete changes[ii];
            }
            changes.clear();
            return;
        }
        
        [_quadLayer.layerThread addChangeRequests:changes];
        [_quadLayer.layerThread flushChangeRequests];
        
        WhirlyKitQuadTileOfflineImage *image = [[WhirlyKitQuadTileOfflineImage alloc] init];
        image.textures = texIDs;
        image.mbr = mbr;
        image.centerSize = [self pixelSizeForMbr:mbr texSize:texSize texel:CGPointMake(texSize.width/2.0, texSize.height/2.0)];
        image->cornerSizes[0] = [self pixelSizeForMbr:mbr texSize:texSize texel:CGPointMake(0.0, 0.0)];
        image->cornerSizes[1] = [self pixelSizeForMbr:mbr texSize:texSize texel:CGPointMake(texSize.width, 0.0)];
        image->cornerSizes[2] = [self pixelSizeForMbr:mbr texSize:texSize texel:CGPointMake(texSize.width, texSize.height)];
        image->cornerSizes[3] = [self pixelSizeForMbr:mbr texSize:texSize texel:CGPointMake(0.0, texSize.height)];
        [_outputDelegate loader:self image:image];
    }
    
    // If we did a quick render, we need to go back again
    if (deep > 0)
        somethingChanged = true;
    else
        somethingChanged = false;
}

// Calculate the real world size of a given pixel
- (CGSize) pixelSizeForMbr:(Mbr)theMbr texSize:(CGSize)texSize texel:(CGPoint)texel
{
    Point2f texelSize((theMbr.ur().x()-theMbr.ll().x())/texSize.width,(theMbr.ur().y()-theMbr.ll().y())/texSize.height);
    
    // Coordinates in the local space
    Point2f l[3];
    l[0] = theMbr.ll() + Point2f(texel.x*texelSize.x(),texel.y*texelSize.y());
    l[1] = theMbr.ll() + Point2f((texel.x+1)*texelSize.x(),texel.y*texelSize.y());
    l[2] = theMbr.ll() + Point2f(texel.x*texelSize.x(),(texel.y+1)*texelSize.y());
    
    // Project the points into display space
    CoordSystemDisplayAdapter *coordAdapter = _quadLayer.scene->getCoordAdapter();
    CoordSystem *localCoordSys = coordAdapter->getCoordSystem();
    CoordSystem *srcCoordSys = _quadLayer.coordSys;
    Point3d d[3];
    for (unsigned int ii=0;ii<3;ii++)
        d[ii] = coordAdapter->localToDisplay(localCoordSys->geocentricToLocal(srcCoordSys->localToGeocentric(Point3d(l[ii].x(),l[ii].y(),0.0))));

    double da = (d[1] - d[0]).norm() * EarthRadius;
    double db = (d[2] - d[0]).norm() * EarthRadius;
    
    return CGSizeMake(da, db);
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
    somethingChanged = true;
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
    somethingChanged = true;
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

    // Assemble the images
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
    somethingChanged = true;
}


@end
