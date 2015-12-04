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

using namespace Eigen;

namespace WhirlyKit
{
    
OfflineTile::OfflineTile()
 : isLoading(false), placeholder(false)
{
};

OfflineTile::OfflineTile(const WhirlyKit::Quadtree::Identifier &ident)
: ident(ident), isLoading(false), placeholder(false)
{
}

OfflineTile::OfflineTile(std::vector<LoadedImage *>loadImages)
: isLoading(false), placeholder(false)
{
}

OfflineTile::~OfflineTile()
{
}

void OfflineTile::GetTileSize(int &numX,int &numY)
{
    numX = numY = 0;
    if (images.size() > 0)
    {
        numX = images[0]->getWidth();
        numY = images[0]->getHeight();
    }
}


QuadTileOfflineLoader::QuadTileOfflineLoader(const std::string &name,QuadTileImageDataSource *imageSource)
    : name(name), imageSource(imageSource), on(true), numImages(1), sizeX(1024), sizeY(1024), autoRes(true),
    period(10.0), previewLevels(-1), outputDelegate(NULL), quadControl(NULL), numFetches(0), renderScheduled(false),
    immediateScheduled(false), lastRender(0), somethingChanged(true), currentMbr(-1)
{
    theMbr.addPoint(Point2f(0,0));
    theMbr.addPoint(Point2f(1.0*M_PI/180.0, 1.0*M_PI/180.0));
}
    
QuadTileOfflineLoader::~QuadTileOfflineLoader()
{
    clear();
}

void QuadTileOfflineLoader::clear()
{
    for (OfflineTileSet::iterator it = tiles.begin();it != tiles.end();++it)
        delete *it;
    tiles.clear();
    somethingChanged = true;
}

    // Note: Porting  Needs to go on Java side
//void QuadTileOfflineLoader::setPeriod(TimeInterval newPeriod)
//{
//    period = newPeriod;
//    
//    if (!quadControl)
//        return;
//    
//    if (period > 0.0 && !renderScheduled)
//    {
//        renderScheduled = true;
//        [self performSelector:@selector(imageRenderPeriodic) withObject:nil afterDelay:_period];
//    }
//}

void QuadTileOfflineLoader::setMbr(Mbr newMbr)
{
    if (!quadControl)
        return;

    if (newMbr.ll().x() < 0 && newMbr.ur().x() > 0 && (newMbr.ur().x() - newMbr.ll().x() > M_PI))
    {
        float tmp = newMbr.ll().x();
        newMbr.ll().x() = newMbr.ur().x();
        newMbr.ur().x() = tmp;
        newMbr.ur().x() += 2*M_PI;
    }

    {
        std::lock_guard<std::mutex> lock(mut);
        theMbr = newMbr;
        currentMbr++;
    }

    somethingChanged = true;
    
    // Note: Porting
//    if (!immediateScheduled)
//    {
//        [self performSelector:@selector(imageRenderImmediate) onThread:_quadLayer.layerThread withObject:nil waitUntilDone:NO];
//        immediateScheduled = true;
//    }
}

// Note: Porting
//void QuadTileOfflineLoader::imageRenderImmediate()
//{
////    NSLog(@"Render:: Immediate");
//    immediateScheduled = false;
//    
//    if (_on)
//    {
//        [self imageRenderToLevel:_previewLevels];
//        if (_previewLevels > 0)
//        {
//            lastRender = 0;
//            if (!renderScheduled)
//            {
//                [self performSelector:@selector(imageRenderPeriodic)];
//                renderScheduled = true;
//            }
//        }
//    }
//}

    // Note: Porting
//- (void)imageRenderPeriodic
//{
//    renderScheduled = false;
//
//    if (_on && somethingChanged)
//    {
////        NSLog(@"Render:: Periodic");
//        CFTimeInterval now = CFAbsoluteTimeGetCurrent();
//        if (now - lastRender >= _period)
//            [self imageRenderToLevel:-1];
//    }
//    
//    if (_period > 0.0 && !renderScheduled)
//    {
//        renderScheduled = true;
//        [self performSelector:@selector(imageRenderPeriodic) withObject:nil afterDelay:_period];
//    }
//}

Point2d QuadTileOfflineLoader::calculateSize()
{
    if (autoRes)
    {
        Point2f imageRes(MAXFLOAT,MAXFLOAT);
        
        Mbr mbr;
        {
            std::lock_guard<std::mutex> lock(mut);
            mbr = theMbr;
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
            tileMbr[0] = quadControl->getQuadtree()->generateMbrForNode(tile->ident);
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
        
        numPix.x() = std::min((float)sizeX,numPix.x());  numPix.y() = std::min((float)sizeY,numPix.y());
        numPix.x() = std::max(numPix.x(),16.f);  numPix.y() = std::max(numPix.y(),16.f);
        return Point2d((int)numPix.x(), (int)numPix.y());
    } else
        return Point2d(sizeX, sizeY);
}

void QuadTileOfflineLoader::imageRenderToLevel(int deep)
{
    if (outputDelegate)
    {
        lastRender = TimeGetCurrent();
        int whichMbr;
        
        Mbr mbr;
        {
            std::lock_guard<std::mutex> lock(mut);
            mbr = theMbr;
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

        // Note: Porting
//        NSMutableArray *images = [NSMutableArray array];
        
        Point2d texSize = calculateSize();
//        NSLog(@"Tex Size = (%f,%f)",texSize.width,texSize.height);
        
        // Draw each entry in the image stack individually
        // Note: Porting
//        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
//        CGContextRef theContext = CGBitmapContextCreate(NULL, texSize.width, texSize.height, 8, texSize.width * 4, colorSpace, kCGImageAlphaPremultipliedLast);
        int numRenderedTiles = 0;
        for (unsigned int ii=0;ii<numImages;ii++)
        {
            // Note: Debugging
            // Note: Porting
//            CGContextSetRGBFillColor(theContext, 1, 0, 0, 1);
//            CGContextFillRect(theContext, CGRectMake(0, 0, texSize.width, texSize.height));
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
                tileMbr[0] = quadControl->getQuadtree()->generateMbrForNode(tile->ident);
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
                    org.x() = texSize.x() * (tileMbr[jj].ll().x() - mbr.ll().x()) / (mbr.ur().x()-mbr.ll().x());
                    org.y() = texSize.y() * (tileMbr[jj].ll().y() - mbr.ll().y()) / (mbr.ur().y()-mbr.ll().y());
                    Point2f span;
                    span.x() = texSize.x() * (tileMbr[jj].ur().x()-tileMbr[jj].ll().x()) / (mbr.ur().x()-mbr.ll().x());
                    span.y() = texSize.y() * (tileMbr[jj].ur().y()-tileMbr[jj].ll().y()) / (mbr.ur().y()-mbr.ll().y());
                    
                    // Find the right input image
                    // Note: Porting
//                    UIImage *imageToDraw = nil;
                    if (ii < tile->images.size())
                    {
                        // Note: Porting
//                        imageToDraw = (UIImage *)tile->images[ii].imageData;
//                        if ([imageToDraw isKindOfClass:[NSData class]])
//                            imageToDraw = [UIImage imageWithData:(NSData *)imageToDraw];
//                        if (![imageToDraw isKindOfClass:[UIImage class]])
//                            imageToDraw = nil;
                    }

                    // Note: Porting
//                    if (imageToDraw)
//                        CGContextDrawImage(theContext, CGRectMake(org.x(),org.y(),span.x(),span.y()), imageToDraw.CGImage);
                }
                numRenderedTiles++;
                
                // If this happens, they've changed the MBR while we were working on this one.  Punt.
                if (whichMbr != currentMbr)
                {
                    // Note: Porting
//                    CGContextRelease(theContext);
//                    CGColorSpaceRelease(colorSpace);
//                    NSLog(@"Aborted render");
                    return;
                }
            }
            
            // Note: Porting
//            CGImageRef imageRef = CGBitmapContextCreateImage(theContext);
//            UIImage *image = [UIImage imageWithCGImage:imageRef];
//            CGImageRelease(imageRef);
//            if (image)
//                [images addObject:image];
        }
        // Note: Porting
//        CGContextRelease(theContext);
//        CGColorSpaceRelease(colorSpace);
        
//        NSLog(@"Rendered %d tiles of %d",numRenderedTiles,(int)tiles.size());
        
        // Convert the images into OpenGL ES textures
        bool aborted = false;
        ChangeSet changes;
        std::vector<WhirlyKit::SimpleIdentity> texIDs;
        // Note: Porting
//        for (UIImage *image in images)
//        {
//            Texture *tex = new Texture("TileQuadOfflineRenderer",image,true);
//            texIDs.push_back(tex->getId());
//            tex->createInGL(_quadLayer.scene->getMemManager());
//            changes.push_back(new AddTextureReq(tex));
//            if (whichMbr != currentMbr)
//            {
//                aborted = true;
//                break;
//            }
//        }
        
        // The texture setup can take a while, so let's be ready to abort here
        if (aborted)
        {
            for (unsigned int ii=0;ii<changes.size();ii++)
            {
                AddTextureReq *texReq = (AddTextureReq *)changes[ii];
                texReq->getTex()->destroyInGL(quadControl->getScene()->getMemManager());
                delete changes[ii];
            }
            changes.clear();
            return;
        }

        // Note: Porting
//        [_quadLayer.layerThread addChangeRequests:changes];
//        [_quadLayer.layerThread flushChangeRequests];
        
        QuadTileOfflineImage *image = new QuadTileOfflineImage();
        image->textures = texIDs;
        image->mbr = mbr;
        image->centerSize = pixelSizeForMbr(mbr,texSize,Point2d(texSize.x()/2.0, texSize.y()/2.0));
        image->cornerSizes[0] = pixelSizeForMbr(mbr,texSize,Point2d(0.0, 0.0));
        image->cornerSizes[1] = pixelSizeForMbr(mbr,texSize,Point2d(texSize.x(), 0.0));
        image->cornerSizes[2] = pixelSizeForMbr(mbr,texSize,Point2d(texSize.x(), texSize.y()));
        image->cornerSizes[3] = pixelSizeForMbr(mbr,texSize,Point2d(0.0, texSize.y()));
        outputDelegate->offlineRender(this, image);
    }
    
    // If we did a quick render, we need to go back again
    if (deep > 0)
        somethingChanged = true;
    else
        somethingChanged = false;
}

// Calculate the real world size of a given pixel
Point2d QuadTileOfflineLoader::pixelSizeForMbr(const Mbr &theMbr,const Point2d &texSize,const Point2d &texel)
{
    Point2f texelSize((theMbr.ur().x()-theMbr.ll().x())/texSize.x(),(theMbr.ur().y()-theMbr.ll().y())/texSize.y());
    
    // Coordinates in the local space
    Point2f l[3];
    l[0] = theMbr.ll() + Point2f(texel.x()*texelSize.x(),texel.y()*texelSize.y());
    l[1] = theMbr.ll() + Point2f((texel.x()+1)*texelSize.x(),texel.y()*texelSize.y());
    l[2] = theMbr.ll() + Point2f(texel.x()*texelSize.x(),(texel.y()+1)*texelSize.y());
    
    // Project the points into display space
    CoordSystemDisplayAdapter *coordAdapter = quadControl->getScene()->getCoordAdapter();
    CoordSystem *localCoordSys = coordAdapter->getCoordSystem();
    CoordSystem *srcCoordSys = quadControl->getCoordSys();
    Point3d d[3];
    for (unsigned int ii=0;ii<3;ii++)
        d[ii] = coordAdapter->localToDisplay(localCoordSys->geocentricToLocal(srcCoordSys->localToGeocentric(Point3d(l[ii].x(),l[ii].y(),0.0))));

    double da = (d[1] - d[0]).norm() * EarthRadius;
    double db = (d[2] - d[0]).norm() * EarthRadius;
    
    return Point2d(da, db);
}

#pragma mark - WhirlyKitQuadLoader

void QuadTileOfflineLoader::shutdownLayer(ChangeSet &changes)
{
    clear();
}
    
void QuadTileOfflineLoader::reset(ChangeSet &changes)
{
    clear();
}

bool QuadTileOfflineLoader::isReady()
{
    return (numFetches <= imageSource->maxSimultaneousFetches());
}

// Note: Porting
//- (void)setQuadLayer:(WhirlyKitQuadDisplayLayer *)layer
//{
//    _quadLayer = layer;
//    
//    if (_period > 0.0 && !renderScheduled) {
//        renderScheduled = true;
//        [self performSelector:@selector(imageRenderPeriodic) withObject:nil afterDelay:_period];
//    }
//}

void QuadTileOfflineLoader::loadTile(const Quadtree::NodeInfo &tileInfo,int frame)
{
    OfflineTile *newTile = new OfflineTile(tileInfo.ident);
    newTile->isLoading = true;
    
    tiles.insert(newTile);
    
    // Note: Porting
//    [_imageSource quadTileLoader:self startFetchForLevel:tileInfo.ident.level col:tileInfo.ident.x row:tileInfo.ident.y attrs:tileInfo.attrs];
    numFetches++;
    somethingChanged = true;
}

OfflineTile *QuadTileOfflineLoader::getTile(const WhirlyKit::Quadtree::Identifier &ident)
{
    OfflineTile dummyTile(ident);
    OfflineTileSet::iterator it = tiles.find(&dummyTile);
    if (it == tiles.end())
        return NULL;
    
    return *it;
}

void QuadTileOfflineLoader::unloadTile(const Quadtree::NodeInfo &tileInfo)
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

bool QuadTileOfflineLoader::canLoadChildrenOfTile(const Quadtree::NodeInfo &tileInfo)
{
    OfflineTile *tile = getTile(tileInfo.ident);
    if (!tile)
        return false;
    
    return !tile->isLoading;
}

void QuadTileOfflineLoader::loadedImage(QuadTileImageDataSource *dataSource,LoadedImage *loadImage,int level,int col,int row,int frame,ChangeSet &changes)
{
    numFetches--;
    Quadtree::Identifier tileIdent(col,row,level);
    OfflineTile *tile = getTile(tileIdent);
    if (!tile)
        return;

    tile->isLoading = false;

    // Assemble the images
    std::vector<LoadedImage *> loadImages;
    loadImages.push_back(loadImage);
    // Note: Porting
//    if (_numImages != loadImages.size())
//    {
//        if (loadTile)
//            NSLog(@"TileQuadLoader: Got %ld images in callback, but was expecting %d.  Punting tile.",loadImages.size(),_numImages);
//        [_quadLayer loader:self tileDidNotLoad:tileIdent];
//        return;
//    }
    tile->images = loadImages;

//    NSLog(@"Loaded tile %d: (%d,%d)",level,col,row);
    quadControl->tileDidLoad(tileIdent, frame);
    somethingChanged = true;
}

}
