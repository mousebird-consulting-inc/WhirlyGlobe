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
#if defined(__ANDROID__)
#import <android/log.h>
#endif
#import "GLUtils.h"

using namespace Eigen;

namespace WhirlyKit
{
    
OfflineTile::OfflineTile()
 : numLoading(0), placeholder(false)
{
};

OfflineTile::OfflineTile(const WhirlyKit::Quadtree::Identifier &ident)
: ident(ident), numLoading(0), placeholder(false)
{
}
    
OfflineTile::OfflineTile(const WhirlyKit::Quadtree::Identifier &ident,int numImages)
    : ident(ident), numLoading(0), placeholder(false)
{
    images.resize(numImages);
}

OfflineTile::~OfflineTile()
{
    for (LoadedImage *image : images)
        delete image;
    images.clear();
}

void OfflineTile::GetTileSize(int &numX,int &numY)
{
    numX = numY = 0;
    LoadedImage *exampleImage = NULL;
    for (unsigned int ii=0;ii<images.size();ii++)
        if (images[ii])
        {
            exampleImage = images[ii];
            break;
        }
    if (exampleImage)
    {
        numX = exampleImage->getWidth();
        numY = exampleImage->getHeight();
    }
}
    
// Return the number of loaded frames
int OfflineTile::getNumLoaded()
{
    int numLoad = 0;
    for (unsigned int ii=0;ii<images.size();ii++)
        if (images[ii])
            numLoad++;
    
    return numLoad;
}

QuadTileOfflineLoader::QuadTileOfflineLoader(const std::string &name,QuadTileImageDataSource *imageSource)
    : name(name), imageSource(imageSource), on(true), numImages(1), sizeX(1024), sizeY(1024), autoRes(true),
    period(10.0), previewLevels(-1), outputDelegate(NULL), numFetches(0), lastRender(0), somethingChanged(true), currentMbr(-1)
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
    
int QuadTileOfflineLoader::numFrames()
{
    // Note: Why are we doing this here?
    {
        std::lock_guard<std::mutex> lock(mut);
        for (unsigned int ii=0;ii<numImages;ii++)
            updatedFrames.insert(ii);
    }
    
    return numImages;
}

void QuadTileOfflineLoader::setMbr(Mbr newMbr)
{
    if (!control)
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
        updatedFrames.clear();
        for (unsigned int ii=0;ii<numImages;ii++)
            updatedFrames.insert(ii);
    }

    somethingChanged = true;
}

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
            // Scale the extents to the output image
            Mbr tileMbr[2];
            tileMbr[0] = control->getQuadtree()->generateMbrForNode(tile->ident);
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

void QuadTileOfflineLoader::imageRenderToLevel(int deep,ChangeSet &changes)
{
    if (!outputDelegate)
        return;
    
    std::set<int> framesToRender;
    int whichMbr;
    Mbr mbr;
    {
        std::lock_guard<std::mutex> lock(mut);
        framesToRender = updatedFrames;
        updatedFrames.clear();
        mbr = theMbr;
        whichMbr = currentMbr;
    }
    
    if (framesToRender.empty())
        return;
    
    lastRender = TimeGetCurrent();
    
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
    
    Point2d texSize = calculateSize();
    int outSizeX = texSize.x(), outSizeY = texSize.y();
    if (outSizeX == 0 || outSizeY == 0)
        return;
    
    // Set up an OpenGL render buffer to draw to
    GLuint frameBuf;
    glGenFramebuffers(1, &frameBuf);
    CheckGLError("Offline glGenFramebuffers");
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuf);
    CheckGLError("Offline glBindFramebuffer");
    
    // Color renderbuffer and backing store
    GLuint colorBuffer;
    glGenRenderbuffers(1, &colorBuffer);
    CheckGLError("Offline glGenRenderbuffers");
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    CheckGLError("Offline glBindRenderbuffer");
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, outSizeX, outSizeY);
    CheckGLError("Offline glRenderbufferStorage");
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
    CheckGLError("Offline glFramebufferRenderbuffer");
    
    //        NSLog(@"Tex Size = (%f,%f)",texSize.width,texSize.height);
    
    int numRenderedTiles = 0;
    
    // We'll just re-render the frames that were updated
    for (std::set<int>::iterator it = framesToRender.begin(); it != framesToRender.end(); ++it)
    {
        if (whichMbr != currentMbr)
            break;
        
        int whichFrame = *it;
        
        // And a texture to render to
        GLuint renderTex;
        glGenTextures(1, &renderTex);
        CheckGLError("Offline glGenTextures");
        glBindTexture(GL_TEXTURE_2D, renderTex);
        CheckGLError("Offline glBindTexture");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, outSizeX, outSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        CheckGLError("Offline glTexImage2D");

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex, 0);
        CheckGLError("Offline glFramebufferTexture2D");

        // Work through the tiles, drawing as we go
        for (OfflineTileSet::iterator it = tiles.begin(); it != tiles.end(); ++it)
        {
            // If this happens, they've changed the MBR while we were working on this one.  Punt.
            if (whichMbr != currentMbr)
            {
                glDeleteTextures(1, &renderTex);
                
                break;
            }

            // Clear output texture
            // Note: Test color
            glClearColor(0.0, 1.0, 0.0, 1.0);
            CheckGLError("Offline glClearColor");
            glClear(GL_COLOR_BUFFER_BIT);
            CheckGLError("Offline glClear");

            OfflineTile *tile = *it;
            if (tile->images[whichFrame] == NULL)
                continue;
            if (deep > 0 && tile->ident.level > deep)
                continue;
            
            // Scale the extents to the output image
            Mbr tileMbr[2];
            tileMbr[0] = control->getQuadtree()->generateMbrForNode(tile->ident);
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
//                    UIImage *imageToDraw = nil;
                void *imageToDraw = NULL;
                if (whichFrame < tile->images.size())
                {
//                        imageToDraw = (UIImage *)tile->images[whichFrame].imageData;
//                        if ([imageToDraw isKindOfClass:[NSData class]])
//                            imageToDraw = [UIImage imageWithData:(NSData *)imageToDraw];
//                        if (![imageToDraw isKindOfClass:[UIImage class]])
//                        {
//                            // Note: Debugging.  Should put this back
//                            //                            NSLog(@"Found bad image in offline renderer.");
//                            imageToDraw = nil;
//                        }
                }
                
                if (imageToDraw)
                {
                    // Note: This draws a green square for debugging
                    //                        CGContextBeginPath(theContext);
                    //                        CGContextAddRect(theContext, CGRectMake(org.x(),org.y(),span.x(),span.y()));
                    //                        CGFloat green[4] = {0,1,0,1};
                    //                        CGContextSetStrokeColor(theContext, green);
                    //                        CGContextDrawPath(theContext, kCGPathStroke);
//                        CGContextDrawImage(theContext, CGRectMake(org.x(),org.y(),span.x(),span.y()), imageToDraw.CGImage);
                }
            }
            numRenderedTiles++;
        }
        
//            CGImageRef imageRef = CGBitmapContextCreateImage(theContext);
//            UIImage *image = [UIImage imageWithCGImage:imageRef];
//            CGImageRelease(imageRef);
        
        //            NSLog(@"Offline: Rendered frame %d, Tex Size = (%f,%f)",whichFrame,texSize.width,texSize.height);
        
        // Register the texture we've already created
        TextureWrapper *tex = new TextureWrapper("TileQuadOfflineRenderer",renderTex);
        SimpleIdentity texID = tex->getId();
        scene->addTexture(tex);
        
//            [_quadLayer.layerThread addChangeRequests:changes];
//            [_quadLayer.layerThread flushChangeRequests];
        
        QuadTileOfflineImage offImage;
        offImage.texture = texID;
        offImage.frame = whichFrame;
        offImage.mbr = mbr;
        offImage.texSize = texSize;
        offImage.centerSize = pixelSizeForMbr(mbr,texSize,Point2d(texSize.x()/2.0, texSize.y()/2.0));
        offImage.cornerSizes[0] = pixelSizeForMbr(mbr,texSize,Point2d(0.0, 0.0));
        offImage.cornerSizes[1] = pixelSizeForMbr(mbr,texSize,Point2d(texSize.x(), 0.0));
        offImage.cornerSizes[2] = pixelSizeForMbr(mbr,texSize,Point2d(texSize.x(), texSize.y()));
        offImage.cornerSizes[3] = pixelSizeForMbr(mbr,texSize,Point2d(0.0, texSize.y()));
        
        if (outputDelegate)
            outputDelegate->offlineRender(this, &offImage);
    }
//        CGContextRelease(theContext);
//        CGColorSpaceRelease(colorSpace);
    
    //        NSLog(@"Rendered %d tiles of %d, depth = %d",numRenderedTiles,(int)tiles.size(),deep);
    
    
    //        NSLog(@"CenterSize = (%f,%f), texSize = (%d,%d)",image.centerSize.width,image.centerSize.height,(int)texSize.width,(int)texSize.height);
    
    // Shut down state we used for rendering, except the texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CheckGLError("Offline glBindFramebuffer clear");
    glBindTexture(GL_TEXTURE_2D, 0);
    CheckGLError("Offline glBindTexture clear");
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    CheckGLError("Offline glBindRenderbuffer clear");
    glDeleteRenderbuffers(1, &colorBuffer);
    CheckGLError("Offline glDeleteRenderbuffers");
    glDeleteFramebuffers(1, &frameBuf);
    CheckGLError("Offline glDeleteFramebuffers");
    
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
    CoordSystemDisplayAdapter *coordAdapter = control->getScene()->getCoordAdapter();
    CoordSystem *localCoordSys = coordAdapter->getCoordSystem();
    CoordSystem *srcCoordSys = control->getCoordSys();
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
    OfflineTile *theTile = getTile(tileInfo.ident);
    if (!theTile)
    {
        theTile = new OfflineTile(tileInfo.ident,numImages);
        tiles.insert(theTile);
    }
    theTile->numLoading++;

    numFetches++;
    imageSource->startFetch(this, tileInfo.ident.level, tileInfo.ident.x, tileInfo.ident.y, frame, const_cast<Dictionary *>(&tileInfo.attrs));
    
    OfflineTile *newTile = new OfflineTile(tileInfo.ident);
    
    tiles.insert(newTile);
    somethingChanged = true;
    
#if defined(__ANDROID__)
    __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Offline loadTile() %d: (%d,%d) %d", tileInfo.ident.level,tileInfo.ident.x,tileInfo.ident.y,frame);
#endif
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
        numFetches -= theTile->numLoading;
        delete theTile;
        tiles.erase(it);
    }
    somethingChanged = true;
    
#if defined(__ANDROID__)
    __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Offline unloadTile() %d: (%d,%d)", tileInfo.ident.level,tileInfo.ident.x,tileInfo.ident.y);
#endif
}

bool QuadTileOfflineLoader::canLoadChildrenOfTile(const Quadtree::NodeInfo &tileInfo)
{
    return true;
}

void QuadTileOfflineLoader::loadedImage(QuadTileImageDataSource *dataSource,LoadedImage *loadImage,int level,int col,int row,int frame,ChangeSet &changes)
{
#if defined(__ANDROID__)
    __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Offline loadedImage() %d: (%d,%d) %d", level,col,row,frame);
#endif
    
    numFetches--;
    Quadtree::Identifier tileIdent(col,row,level);
    OfflineTile *tile = getTile(tileIdent);
    if (!tile)
        return;
    
    if (tile->numLoading > 0)
    {
        numFetches--;
        tile->numLoading--;
    }

    // Assemble the images
    std::vector<LoadedImage *> loadImages;
    loadImages.push_back(loadImage);

    if ((frame == -1 && numImages != loadImages.size()) || (frame != -1 && loadImages.size() != 1))
    {
        control->tileDidNotLoad(tileIdent, frame);
        return;
    }
    if (frame == -1)
    {
        tile->images = loadImages;
    } else {
        tile->images[frame] = loadImages[0];
    }
    
    //    NSLog(@"Loaded tile %d: (%d,%d), frame = %d",level,col,row,frame);
    control->tileDidLoad(tileIdent, frame);

    // We'll need to update this frame
    {
        std::lock_guard<std::mutex> lock(mut);
        updatedFrames.insert(frame);
        somethingChanged = true;
    }
}

}
