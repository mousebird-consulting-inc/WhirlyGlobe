/*
*  LoadedTile.mm
*  WhirlyGlobeLib
*
*  Created by Steve Gifford on 9/19/13.
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

#import "LoadedTile.h"
#import "GlobeMath.h"
#import "GlobeLayerViewWatcher.h"
#import "UIImage+Stuff.h"
#import <boost/math/special_functions/fpclassify.hpp>
#import "TileQuadLoader.h"
#import "DynamicTextureAtlas.h"
#import "DynamicDrawableAtlas.h"

using namespace WhirlyKit;

@implementation WhirlyKitLoadedTile

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    _images = [NSMutableArray array];
    
    return self;
}

@end

@implementation WhirlyKitLoadedImage

+ (WhirlyKitLoadedImage *)LoadedImageWithUIImage:(UIImage *)image
{
    WhirlyKitLoadedImage *loadImage = [[WhirlyKitLoadedImage alloc] init];
    loadImage.type = WKLoadedImageUIImage;
    loadImage.borderSize = 0;
    loadImage.imageData = image;
    CGImageRef cgImage = image.CGImage;
    loadImage.width = CGImageGetWidth(cgImage);
    loadImage.height = CGImageGetHeight(cgImage);
    
    return loadImage;
}

+ (WhirlyKitLoadedImage *)LoadedImageWithPVRTC:(NSData *)imageData size:(int)squareSize
{
    WhirlyKitLoadedImage *loadImage = [[WhirlyKitLoadedImage alloc] init];
    loadImage.type = WKLoadedImagePVRTC4;
    loadImage.borderSize = 0;
    loadImage.imageData = imageData;
    loadImage.width = loadImage.height = squareSize;
    
    return loadImage;
}

+ (WhirlyKitLoadedImage *)LoadedImageWithNSDataAsPNGorJPG:(NSData *)imageData
{
    WhirlyKitLoadedImage *loadImage = [[WhirlyKitLoadedImage alloc] init];
    loadImage.type = WKLoadedImageNSDataAsImage;
    loadImage.borderSize = 0;
    loadImage.imageData = imageData;
    loadImage.width = loadImage.height = 0;
    UIImage *texImage = [UIImage imageWithData:(NSData *)imageData];
    if (texImage)
    {
        loadImage.imageData = texImage;
        loadImage.width = CGImageGetWidth(texImage.CGImage);
        loadImage.height = CGImageGetHeight(texImage.CGImage);
        loadImage.type = WKLoadedImageUIImage;
    } else
        return nil;
    
    return loadImage;
}

+ (WhirlyKitLoadedImage *)PlaceholderImage
{
    WhirlyKitLoadedImage *loadImage = [[WhirlyKitLoadedImage alloc] init];
    loadImage.type = WKLoadedImagePlaceholder;
    
    return loadImage;
}

- (WhirlyKit::Texture *)textureFromRawData:(NSData *)theData width:(int)theWidth height:(int)theHeight
{
    Texture *newTex = new Texture("Tile Quad Loader",theData,false);
    newTex->setWidth(theWidth);
    newTex->setHeight(theHeight);
    
    return newTex;
}

- (WhirlyKit::Texture *)buildTexture:(int)reqBorderTexel destWidth:(int)destWidth destHeight:(int)destHeight
{
    Texture *newTex = NULL;
    
    switch (_type)
    {
        case WKLoadedImageUIImage:
        {
            destWidth = (destWidth <= 0 ? _width : destWidth);
            destHeight = (destHeight <= 0 ? _height : destHeight);
            NSData *rawData = [(UIImage *)_imageData rawDataScaleWidth:destWidth height:destHeight border:reqBorderTexel];
            newTex = [self textureFromRawData:rawData width:destWidth height:destHeight];
        }
            break;
        case WKLoadedImageNSDataAsImage:
            // These are converted to UIImages on initialization.  So it must have failed.
            break;
        case WKLoadedImageNSDataRawData:
            if ([_imageData isKindOfClass:[NSData class]])
            {
                // Note: This isn't complete
                return [self textureFromRawData:(NSData *)_imageData width:_width height:_height];
            }
            break;
        case WKLoadedImagePVRTC4:
            if ([_imageData isKindOfClass:[NSData class]])
            {
                newTex = new Texture("Tile Quad Loader", (NSData *)_imageData,true);
                newTex->setWidth(_width);
                newTex->setHeight(_height);
            }
            break;
        case WKLoadedImagePlaceholder:
        default:
            break;
    }
    
    return newTex;
}

- (bool)convertToRawData:(int)borderTexel
{
    switch (_type)
    {
        case WKLoadedImageUIImage:
        {
            int destWidth = _width;
            int destHeight = _height;
            // We need this to be square.  Because duh.
            if (destWidth != destHeight)
            {
                int size = std::max(destWidth,destHeight);
                destWidth = destHeight = size;
            }
            NSData *rawData = [(UIImage *)_imageData rawDataScaleWidth:destWidth height:destHeight border:borderTexel];
            if (rawData)
            {
                _imageData = rawData;
                _type = WKLoadedImageNSDataRawData;
                _width = destWidth;
                _height = destHeight;
            }
        }
            break;
        default:
            return false;
            break;
    }
    
    return true;
}

@end

namespace WhirlyKit
{
    
// Figure out the target size for an image based on our settings
void TileBuilder::textureSize(int width, int height,int *destWidth,int *destHeight)
{
    switch (tileScale)
    {
        case WKTileScaleNone:
            *destWidth = width;
            *destHeight = height;
            break;
        case WKTileScaleDown:
        {
            int upWidth = NextPowOf2(width);
            int upHeight = NextPowOf2(height);
            
            if (upWidth > width && upWidth > 4)
                upWidth /= 2;
                if (upHeight > height && upHeight > 4)
                    upHeight /= 2;
                    
                    // Note: Shouldn't be necessary
                    int square = std::max(upWidth,upHeight);
                    *destWidth = square;
                    *destHeight = square;
                    }
            break;
        case WKTileScaleUp:
        {
            int upWidth = NextPowOf2(width);
            int upHeight = NextPowOf2(height);
            
            *destWidth = upWidth;
            *destHeight = upHeight;
        }
            break;
        case WKTileScaleFixed:
            *destWidth = *destHeight = fixedTileSize;
            break;
    }
}

TileBuilder::TileBuilder(CoordSystem *coordSys,Mbr mbr,WhirlyKit::Quadtree *quadTree)
    : coordSys(coordSys), mbr(mbr), tree(quadTree),
    tileScale(WKTileScaleNone), fixedTileSize(128),
    drawOffset(0),drawPriority(0), minVis(DrawVisibleInvalid), maxVis(DrawVisibleInvalid), hasAlpha(false),
    color(255,255,255,255), programId(EmptyIdentity),
    includeElev(false), useElevAsZ(true),
    ignoreEdgeMatching(false),
    coverPoles(true),
    glFormat(WKTileIntRGBA), singleByteSource(WKSingleRGB),
    defaultSphereTessX(10), defaultSphereTessY(10),
    texelBinSize(64),
    drawAtlas(NULL),
    borderTexel(0),
    scene(NULL),
    lineMode(false),
    activeTextures(-1),
    enabled(true),
    texAtlas(NULL)
{
    pthread_mutex_init(&texAtlasMappingLock, NULL);
}

TileBuilder::~TileBuilder()
{
    pthread_mutex_lock(&texAtlasMappingLock);
    texAtlasMappings.clear();
    pthread_mutex_unlock(&texAtlasMappingLock);
    pthread_mutex_destroy(&texAtlasMappingLock);
    
    if (texAtlas)
    {
        delete texAtlas;
        texAtlas = NULL;
    }
    if (drawAtlas)
    {
        delete drawAtlas;
        drawAtlas = NULL;
    }
}
    
// The vertex size is just used for buffer size estimates
static const int SingleVertexSize = 3*sizeof(float) + 2*sizeof(float) +  4*sizeof(unsigned char) + 3*sizeof(float);
static const int SingleElementSize = sizeof(GLushort);
    
void TileBuilder::initAtlases(WhirlyKitTileImageType imageType,int numImages,int textureAtlasSize,int sampleSizeX,int sampleSizeY)
{
    // Note: Trouble with PVRTC sub texture loading
    if (imageType != WKTilePVRTC4)
    {
        // How many tiles can we stuff into a texture atlas, if we assume tiles 256 pixels in size
        int NumTiles = textureAtlasSize / 256; NumTiles = NumTiles*NumTiles;
        int DrawBufferVertices = (sampleSizeX + 1) * (sampleSizeY + 1) * NumTiles;
        // Have to be able to address them
        // Note: Can't go up to 65536 for some reason
        DrawBufferVertices = std::min(DrawBufferVertices,32768);
        int DrawBufferSize = DrawBufferVertices * SingleVertexSize;
        
        // Two triangles per grid cell in a tile
        int ElementBufferSize = 6 * DrawBufferVertices * SingleElementSize;
        int texSortSize = (tileScale == WKTileScaleFixed ? fixedTileSize : texelBinSize);
        
        imageDepth = numImages;
        texAtlas = new DynamicTextureAtlas(textureAtlasSize,texSortSize,glFormat,numImages);
        drawAtlas = new DynamicDrawableAtlas("Tile Quad Loader",SingleElementSize,DrawBufferSize,ElementBufferSize,scene->getMemManager(),NULL,programId);
    }
}
    
void TileBuilder::clearAtlases(ChangeSet &theChangeRequests)
{
    pthread_mutex_lock(&texAtlasMappingLock);
    texAtlasMappings.clear();
    pthread_mutex_unlock(&texAtlasMappingLock);
    
    if (texAtlas)
    {
        texAtlas->shutdown(theChangeRequests);
        delete texAtlas;
        texAtlas = NULL;
    }
    
    if (drawAtlas)
    {
        drawAtlas->shutdown(theChangeRequests);
        delete drawAtlas;
        drawAtlas = NULL;
    }
}

// Helper routine for constructing the skirt around a tile
void TileBuilder::buildSkirt(BasicDrawable *draw,std::vector<Point3f> &pts,std::vector<TexCoord> &texCoords,float skirtFactor,bool haveElev)
{
    for (unsigned int ii=0;ii<pts.size()-1;ii++)
    {
        Point3f corners[4];
        TexCoord cornerTex[4];
        corners[0] = pts[ii];
        cornerTex[0] = texCoords[ii];
        corners[1] = pts[ii+1];
        cornerTex[1] = texCoords[ii+1];
        if (haveElev)
            corners[2] = pts[ii+1].normalized();
            else
                corners[2] = pts[ii+1] * skirtFactor;
                cornerTex[2] = texCoords[ii+1];
                if (haveElev)
                    corners[3] = pts[ii].normalized();
                    else
                        corners[3] = pts[ii] * skirtFactor;
                        cornerTex[3] = texCoords[ii];
                        
                        // Toss in the points, but point the normal up
                        int base = draw->getNumPoints();
                        for (unsigned int jj=0;jj<4;jj++)
                        {
                            draw->addPoint(corners[jj]);
                            draw->addNormal((pts[ii]+pts[ii+1])/2.0);
                            TexCoord texCoord = cornerTex[jj];
                            draw->addTexCoord(-1,texCoord);
                        }
        
        // Add two triangles
        draw->addTriangle(BasicDrawable::Triangle(base+3,base+2,base+0));
        draw->addTriangle(BasicDrawable::Triangle(base+0,base+2,base+1));
    }
}

    
bool TileBuilder::buildTile(Quadtree::NodeInfo *nodeInfo,BasicDrawable **draw,BasicDrawable **skirtDraw,std::vector<Texture *> *texs,
                            Point2f texScale,Point2f texOffset,std::vector<WhirlyKitLoadedImage *> *loadImages,WhirlyKitElevationChunk *elevData)
{
    Mbr theMbr = nodeInfo->mbr;
    
    // Make sure this overlaps the area we care about
    if (!theMbr.overlaps(mbr))
    {
        NSLog(@"Building bogus tile: (%d,%d,%d)",nodeInfo->ident.x,nodeInfo->ident.y,nodeInfo->ident.level);
    }
    
    // Snap to the designated area
    if (theMbr.ll().x() < mbr.ll().x())
        theMbr.ll().x() = mbr.ll().x();
    if (theMbr.ur().x() > mbr.ur().x())
        theMbr.ur().x() = mbr.ur().x();
    if (theMbr.ll().y() < mbr.ll().y())
        theMbr.ll().y() = mbr.ll().y();
    if (theMbr.ur().y() > mbr.ur().y())
        theMbr.ur().y() = mbr.ur().y();
    
    // Number of pieces at this level
    int xDim = 1<<nodeInfo->ident.level;
    int yDim = 1<<nodeInfo->ident.level;
    
    //    NSLog(@"Chunk ll = (%.4f,%.4f)  ur = (%.4f,%.4f)",mbr.ll().x(),mbr.ll().y(),mbr.ur().x(),mbr.ur().y());
    
    // Size of each chunk
    Point2f chunkSize = theMbr.ur() - theMbr.ll();
    
    int sphereTessX = defaultSphereTessX,sphereTessY = defaultSphereTessY;
    if (elevData)
    {
        sphereTessX = elevData.numX-1;
        sphereTessY = elevData.numY-1;
    }
    
    // Unit size of each tesselation in spherical mercator
    Point2f incr(chunkSize.x()/sphereTessX,chunkSize.y()/sphereTessY);
    
    // Texture increment for each tesselation
    TexCoord texIncr(1.0/(float)sphereTessX,1.0/(float)sphereTessY);
    
    // We're viewing this as a parameterization from ([0->1.0],[0->1.0]) so we'll
    //  break up these coordinates accordingly
    Point2f paramSize(1.0/(xDim*sphereTessX),1.0/(yDim*sphereTessY));
    
    // We need the corners in geographic for the cullable
    Point2f chunkLL = theMbr.ll();
    Point2f chunkUR = theMbr.ur();
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    CoordSystem *sceneCoordSys = coordAdapter->getCoordSystem();
    GeoCoord geoLL(coordSys->localToGeographic(Point3f(chunkLL.x(),chunkLL.y(),0.0)));
    GeoCoord geoUR(coordSys->localToGeographic(Point3f(chunkUR.x(),chunkUR.y(),0.0)));
    
    // Get textures (locally)
    if (texs)
    {
        bool texturesClean = true;
        if (loadImages && (*loadImages)[0].type != WKLoadedImagePlaceholder)
        {
            // They'll all be the same width
            WhirlyKitLoadedImage *loadImage = (*loadImages)[0];
            int destWidth,destHeight;
            textureSize(loadImage.width,loadImage.height,&destWidth,&destHeight);
            
            // Create a texture for each
            for (unsigned int ii=0;ii<loadImages->size();ii++)
            {
                Texture *newTex = [(*loadImages)[ii] buildTexture:borderTexel destWidth:destWidth destHeight:destHeight];
                
                if (newTex)
                {
                    newTex->setFormat(glFormat);
                    newTex->setSingleByteSource(singleByteSource);
                    (*texs)[ii] = newTex;
                } else {
                    texturesClean = false;
                    (*texs)[ii] = NULL;
                }
            }
        } else {
            for (unsigned int ii=0;ii<texs->size();ii++)
                (*texs)[ii] = NULL;
        }
        
        // If the textures didn't build cleanly, we'll delete them and fail
        if (!texturesClean)
        {
            if (texs)
                for (unsigned int ii=0;ii<texs->size();ii++)
                    if ((*texs)[ii])
                    {
                        delete (*texs)[ii];
                        (*texs)[ii] = NULL;
                    }
            return false;
        }
    }
    
    if (draw)
    {
        // We'll set up and fill in the drawable
        BasicDrawable *chunk = new BasicDrawable("Tile Quad Loader",(sphereTessX+1)*(sphereTessY+1),2*sphereTessX*sphereTessY);
        if (activeTextures > 0)
            chunk->setTexId(activeTextures-1, EmptyIdentity);
        chunk->setDrawOffset(drawOffset);
        chunk->setDrawPriority(drawPriority);
        chunk->setVisibleRange(minVis, maxVis);
        chunk->setAlpha(hasAlpha);
        chunk->setColor(color);
        chunk->setLocalMbr(Mbr(Point2f(geoLL.x(),geoLL.y()),Point2f(geoUR.x(),geoUR.y())));
        chunk->setProgram(programId);
        int elevEntry = 0;
        if (includeElev)
            elevEntry = chunk->addAttribute(BDFloatType, "a_elev");
        
        // We're in line mode or the texture didn't load
        if (lineMode || (texs && !texs->empty() && !((*texs)[0])))
        {
            chunk->setType(GL_LINES);
            
            // Two lines per cell
            for (unsigned int iy=0;iy<sphereTessY;iy++)
                for (unsigned int ix=0;ix<sphereTessX;ix++)
                {
                    Point3f org3D = coordAdapter->localToDisplay(CoordSystemConvert(coordSys,sceneCoordSys,Point3f(chunkLL.x()+ix*incr.x(),chunkLL.y()+iy*incr.y(),0.0)));
                    Point3f ptA_3D = coordAdapter->localToDisplay(CoordSystemConvert(coordSys,sceneCoordSys,Point3f(chunkLL.x()+(ix+1)*incr.x(),chunkLL.y()+iy*incr.y(),0.0)));
                    Point3f ptB_3D = coordAdapter->localToDisplay(CoordSystemConvert(coordSys,sceneCoordSys,Point3f(chunkLL.x()+ix*incr.x(),chunkLL.y()+(iy+1)*incr.y(),0.0)));
                    
                    TexCoord texCoord(ix*texIncr.x()*texScale.x()+texOffset.x(),1.0-(iy*texIncr.y()*texScale.y()+texOffset.y()));
                    
                    chunk->addPoint(org3D);
                    chunk->addNormal(org3D);
                    chunk->addTexCoord(-1,texCoord);
                    chunk->addPoint(ptA_3D);
                    chunk->addNormal(ptA_3D);
                    chunk->addTexCoord(-1,texCoord);
                    
                    chunk->addPoint(org3D);
                    chunk->addNormal(org3D);
                    chunk->addTexCoord(-1,texCoord);
                    chunk->addPoint(ptB_3D);
                    chunk->addNormal(ptB_3D);
                    chunk->addTexCoord(-1,texCoord);
                }
        } else {
            chunk->setType(GL_TRIANGLES);
            // Generate point, texture coords, and normals
            std::vector<Point3f> locs((sphereTessX+1)*(sphereTessY+1));
            std::vector<float> elevs;
            if (includeElev || useElevAsZ)
                elevs.resize((sphereTessX+1)*(sphereTessY+1));
            std::vector<TexCoord> texCoords((sphereTessX+1)*(sphereTessY+1));
            for (unsigned int iy=0;iy<sphereTessY+1;iy++)
                for (unsigned int ix=0;ix<sphereTessX+1;ix++)
                {
                    float locZ = 0.0;
                    if (!elevs.empty())
                    {
                        if (elevData)
                        {
                            float whereX = ix*texScale.x() + (elevData.numX-1)*texOffset.x();
                            float whereY = iy*texScale.y() + (elevData.numY-1)*texOffset.y();
                            locZ = [elevData interpolateElevationAtX:whereX y:whereY];
                        }
                        elevs[iy*(sphereTessX+1)+ix] = locZ;
                    }
                    // We don't want real elevations in the mesh, just off in another attribute
                    if (!useElevAsZ)
                        locZ = 0.0;
                    Point3f loc3D = coordAdapter->localToDisplay(CoordSystemConvert(coordSys,sceneCoordSys,Point3f(chunkLL.x()+ix*incr.x(),chunkLL.y()+iy*incr.y(),locZ)));
                    if (coordAdapter->isFlat())
                        loc3D.z() = locZ;
                    locs[iy*(sphereTessX+1)+ix] = loc3D;
                    
                    // Do the texture coordinate seperately
                    TexCoord texCoord(ix*texIncr.x()*texScale.x()+texOffset.x(),1.0-(iy*texIncr.y()*texScale.y()+texOffset.y()));
                    texCoords[iy*(sphereTessX+1)+ix] = texCoord;
                }
            
            // If there's elevation data, we need per triangle normals, which means more vertices
            if (!elevs.empty())
            {
                // Two triangles per cell
                for (unsigned int iy=0;iy<sphereTessY;iy++)
                {
                    for (unsigned int ix=0;ix<sphereTessX;ix++)
                    {
                        int startPt = chunk->getNumPoints();
                        int idx0 = (iy+1)*(sphereTessX+1)+ix;
                        Point3f ptA_0 = locs[idx0];
                        int idx1 = iy*(sphereTessX+1)+ix;
                        Point3f ptA_1 = locs[idx1];
                        int idx2 = (iy+1)*(sphereTessX+1)+(ix+1);
                        Point3f ptA_2 = locs[idx2];
                        Point3f normA = (ptA_2-ptA_1).cross(ptA_0-ptA_1);
                        normA.normalize();
                        chunk->addPoint(ptA_0);
                        chunk->addTexCoord(-1,texCoords[idx0]);
                        chunk->addNormal(normA);
                        if (elevEntry != 0)
                            chunk->addAttributeValue(elevEntry, elevs[idx0]);
                        
                        chunk->addPoint(ptA_1);
                        chunk->addTexCoord(-1,texCoords[idx1]);
                        chunk->addNormal(normA);
                        if (elevEntry != 0)
                            chunk->addAttributeValue(elevEntry, elevs[idx1]);
                        
                        chunk->addPoint(ptA_2);
                        chunk->addTexCoord(-1,texCoords[idx2]);
                        chunk->addNormal(normA);
                        if (elevEntry != 0)
                            chunk->addAttributeValue(elevEntry, elevs[idx2]);
                        
                        BasicDrawable::Triangle triA,triB;
                        triA.verts[0] = startPt;
                        triA.verts[1] = startPt+1;
                        triA.verts[2] = startPt+2;
                        chunk->addTriangle(triA);
                        
                        startPt = chunk->getNumPoints();
                        idx0 = idx2;
                        Point3f ptB_0 = ptA_2;
                        idx1 = idx1;
                        Point3f ptB_1 = ptA_1;
                        idx2 = iy*(sphereTessX+1)+(ix+1);
                        Point3f ptB_2 = locs[idx2];
                        Point3f normB = (ptB_0-ptB_2).cross(ptB_1-ptB_2);
                        normB.normalize();
                        chunk->addPoint(ptB_0);
                        chunk->addTexCoord(-1,texCoords[idx0]);
                        chunk->addNormal(normB);
                        if (elevEntry != 0)
                            chunk->addAttributeValue(elevEntry, elevs[idx0]);
                        
                        chunk->addPoint(ptB_1);
                        chunk->addTexCoord(-1,texCoords[idx1]);
                        chunk->addNormal(normB);
                        if (elevEntry != 0)
                            chunk->addAttributeValue(elevEntry, elevs[idx1]);
                        
                        chunk->addPoint(ptB_2);
                        chunk->addTexCoord(-1,texCoords[idx2]);
                        chunk->addNormal(normB);
                        if (elevEntry != 0)
                            chunk->addAttributeValue(elevEntry, elevs[idx2]);
                        
                        triB.verts[0] = startPt;
                        triB.verts[1] = startPt+1;
                        triB.verts[2] = startPt+2;
                        chunk->addTriangle(triB);
                    }
                }
                
                
            } else {
                // Without elevation data we can share the vertices
                for (unsigned int iy=0;iy<sphereTessY+1;iy++)
                    for (unsigned int ix=0;ix<sphereTessX+1;ix++)
                    {
                        Point3f &loc3D = locs[iy*(sphereTessX+1)+ix];
                        
                        // And the normal
                        Point3f norm3D;
                        if (coordAdapter->isFlat())
                            norm3D = coordAdapter->normalForLocal(loc3D);
                        else
                            norm3D = loc3D;
                        
                        TexCoord &texCoord = texCoords[iy*(sphereTessX+1)+ix];
                        
                        chunk->addPoint(loc3D);
                        chunk->addNormal(norm3D);
                        chunk->addTexCoord(-1,texCoord);
                    }
                
                // Two triangles per cell
                for (unsigned int iy=0;iy<sphereTessY;iy++)
                {
                    for (unsigned int ix=0;ix<sphereTessX;ix++)
                    {
                        BasicDrawable::Triangle triA,triB;
                        triA.verts[0] = (iy+1)*(sphereTessX+1)+ix;
                        triA.verts[1] = iy*(sphereTessX+1)+ix;
                        triA.verts[2] = (iy+1)*(sphereTessX+1)+(ix+1);
                        triB.verts[0] = triA.verts[2];
                        triB.verts[1] = triA.verts[1];
                        triB.verts[2] = iy*(sphereTessX+1)+(ix+1);
                        chunk->addTriangle(triA);
                        chunk->addTriangle(triB);
                    }
                }
            }
            
            if (!ignoreEdgeMatching && !coordAdapter->isFlat() && skirtDraw)
            {
                // We'll set up and fill in the drawable
                BasicDrawable *skirtChunk = new BasicDrawable("Tile Quad Loader Skirt");
                if (activeTextures > 0)
                    skirtChunk->setTexId(activeTextures-1, EmptyIdentity);
                skirtChunk->setDrawOffset(drawOffset);
                skirtChunk->setDrawPriority(0);
                skirtChunk->setVisibleRange(minVis, maxVis);
                skirtChunk->setAlpha(hasAlpha);
                skirtChunk->setColor(color);
                skirtChunk->setLocalMbr(Mbr(Point2f(geoLL.x(),geoLL.y()),Point2f(geoUR.x(),geoUR.y())));
                skirtChunk->setType(GL_TRIANGLES);
                // We need the skirts rendered with the z buffer on, even if we're doing (mostly) pure sorting
                skirtChunk->setRequestZBuffer(true);
                skirtChunk->setProgram(programId);
                
                // We'll vary the skirt size a bit.  Otherwise the fill gets ridiculous when we're looking
                //  at the very highest levels.  On the other hand, this doesn't fix a really big large/small
                //  disparity
                float skirtFactor = 0.95;
                bool haveElev = elevData && useElevAsZ;
                // Leave the big skirts in place if we're doing real elevation
                if (!elevData || !useElevAsZ)
                    skirtFactor = 1.0 - 0.2 / (1<<nodeInfo->ident.level);
                
                // Bottom skirt
                std::vector<Point3f> skirtLocs;
                std::vector<TexCoord> skirtTexCoords;
                for (unsigned int ix=0;ix<=sphereTessX;ix++)
                {
                    skirtLocs.push_back(locs[ix]);
                    skirtTexCoords.push_back(texCoords[ix]);
                }
                buildSkirt(skirtChunk,skirtLocs,skirtTexCoords,skirtFactor,haveElev);
                // Top skirt
                skirtLocs.clear();
                skirtTexCoords.clear();
                for (int ix=sphereTessX;ix>=0;ix--)
                {
                    skirtLocs.push_back(locs[(sphereTessY)*(sphereTessX+1)+ix]);
                    skirtTexCoords.push_back(texCoords[(sphereTessY)*(sphereTessX+1)+ix]);
                }
                buildSkirt(skirtChunk,skirtLocs,skirtTexCoords,skirtFactor,haveElev);
                // Left skirt
                skirtLocs.clear();
                skirtTexCoords.clear();
                for (int iy=sphereTessY;iy>=0;iy--)
                {
                    skirtLocs.push_back(locs[(sphereTessX+1)*iy+0]);
                    skirtTexCoords.push_back(texCoords[(sphereTessX+1)*iy+0]);
                }
                buildSkirt(skirtChunk,skirtLocs,skirtTexCoords,skirtFactor,haveElev);
                // right skirt
                skirtLocs.clear();
                skirtTexCoords.clear();
                for (int iy=0;iy<=sphereTessY;iy++)
                {
                    skirtLocs.push_back(locs[(sphereTessX+1)*iy+(sphereTessX)]);
                    skirtTexCoords.push_back(texCoords[(sphereTessX+1)*iy+(sphereTessX)]);
                }
                buildSkirt(skirtChunk,skirtLocs,skirtTexCoords,skirtFactor,haveElev);
                
                if (texs && !texs->empty() && !((*texs)[0]))
                    skirtChunk->setTexId(0,(*texs)[0]->getId());
                *skirtDraw = skirtChunk;
            }
            
            if (coverPoles && !coordAdapter->isFlat())
            {
                // If we're at the top, toss in a few more triangles to represent that
                int maxY = 1 << nodeInfo->ident.level;
                if (nodeInfo->ident.y == maxY-1)
                {
                    TexCoord singleTexCoord(0.5,0.0);
                    // One point for the north pole
                    Point3f northPt(0,0,1.0);
                    chunk->addPoint(northPt);
                    chunk->addTexCoord(-1,singleTexCoord);
                    chunk->addNormal(Point3f(0,0,1.0));
                    if (elevEntry != 0)
                        chunk->addAttributeValue(elevEntry, 0.0);
                    int northVert = chunk->getNumPoints()-1;
                    
                    // A line of points for the outer ring, but we can copy them
                    int startOfLine = chunk->getNumPoints();
                    int iy = sphereTessY;
                    for (unsigned int ix=0;ix<sphereTessX+1;ix++)
                    {
                        Point3f pt = locs[(iy*(sphereTessX+1)+ix)];
                        float elev = 0.0;
                        if (!elevs.empty())
                            elev = elevs[(iy*(sphereTessX+1)+ix)];
                        chunk->addPoint(pt);
                        chunk->addNormal(Point3f(0,0,1.0));
                        chunk->addTexCoord(-1,singleTexCoord);
                        if (elevEntry != 0)
                            chunk->addAttributeValue(elevEntry, elev);
                    }
                    
                    // And define the triangles
                    for (unsigned int ix=0;ix<sphereTessX;ix++)
                    {
                        BasicDrawable::Triangle tri;
                        tri.verts[0] = startOfLine+ix;
                        tri.verts[1] = startOfLine+ix+1;
                        tri.verts[2] = northVert;
                        chunk->addTriangle(tri);
                    }
                }
                
                if (nodeInfo->ident.y == 0)
                {
                    TexCoord singleTexCoord(0.5,1.0);
                    // One point for the south pole
                    Point3f southPt(0,0,-1.0);
                    chunk->addPoint(southPt);
                    chunk->addTexCoord(-1,singleTexCoord);
                    chunk->addNormal(Point3f(0,0,-1.0));
                    if (elevEntry != 0)
                        chunk->addAttributeValue(elevEntry, 0.0);
                    int southVert = chunk->getNumPoints()-1;
                    
                    // A line of points for the outside ring, which we can copy
                    int startOfLine = chunk->getNumPoints();
                    int iy = 0;
                    for (unsigned int ix=0;ix<sphereTessX+1;ix++)
                    {
                        Point3f pt = locs[(iy*(sphereTessX+1)+ix)];
                        float elev = 0.0;
                        if (!elevs.empty())
                            elev = elevs[(iy*(sphereTessX+1)+ix)];
                        chunk->addPoint(pt);
                        chunk->addNormal(Point3f(0,0,-1.0));
                        chunk->addTexCoord(-1,singleTexCoord);
                        if (elevEntry != 0)
                            chunk->addAttributeValue(elevEntry, elev);
                    }
                    
                    // And define the triangles
                    for (unsigned int ix=0;ix<sphereTessX;ix++)
                    {
                        BasicDrawable::Triangle tri;
                        tri.verts[0] = southVert;
                        tri.verts[1] = startOfLine+ix+1;
                        tri.verts[2] = startOfLine+ix;
                        chunk->addTriangle(tri);
                    }
                }
            }
            
            if (texs && !texs->empty() && (*texs)[0])
                chunk->setTexId(0,(*texs)[0]->getId());
        }
        
        *draw = chunk;
    }
    
    return true;
}

// Note: Off for now
bool TileBuilder::flushUpdates(ChangeSet &changes)
{
    
    return false;
}
    
void TileBuilder::updateAtlasMappings()
{
    std::vector<std::vector<SimpleIdentity> > newTexAtlasMappings;
    std::vector<DynamicDrawableAtlas::DrawTexInfo> newDrawTexInfo;
    
    if (texAtlas)
    {
        for (unsigned int ii=0;ii<imageDepth;ii++)
        {
            std::vector<SimpleIdentity> texIDs;
            texAtlas->getTextureIDs(texIDs,ii);
            newTexAtlasMappings.push_back(texIDs);
        }
    }

    SimpleIDSet newDrawIDs;
    if (drawAtlas)
        drawAtlas->getDrawableTextures(newDrawTexInfo);
    
    // Move the new data over at once (to avoid stalling the main thread)
    pthread_mutex_lock(&texAtlasMappingLock);
    texAtlasMappings = newTexAtlasMappings;
    drawTexInfo = newDrawTexInfo;
    pthread_mutex_unlock(&texAtlasMappingLock);    
}
    
bool TileBuilder::isReady()
{
    return !drawAtlas || !drawAtlas->waitingOnSwap();
}
    
void TileBuilder::log(NSString *name)
{
    if (!drawAtlas && !texAtlas)
        return;
    
    NSLog(@"++ Quad Tile Loader %@ ++",(name ? name : @"Unknown"));
    if (drawAtlas)
        drawAtlas->log();
    if (texAtlas)
        texAtlas->log();
    NSLog(@"++ ++ ++");
}

LoadedTile::LoadedTile()
{
    isLoading = false;
    placeholder = false;
    drawId = EmptyIdentity;
    skirtDrawId = EmptyIdentity;
    for (unsigned int ii=0;ii<4;ii++)
    {
        childDrawIds[ii] = EmptyIdentity;
        childSkirtDrawIds[ii] = EmptyIdentity;
    }
}

LoadedTile::LoadedTile(const WhirlyKit::Quadtree::Identifier &ident)
{
    nodeInfo.ident = ident;
    isLoading = false;
    placeholder = false;
    drawId = EmptyIdentity;
    skirtDrawId = EmptyIdentity;
    elevData = nil;
    for (unsigned int ii=0;ii<4;ii++)
    {
        childDrawIds[ii] = EmptyIdentity;
        childSkirtDrawIds[ii] = EmptyIdentity;
    }
}

// Add the geometry and texture to the scene for a given tile
bool LoadedTile::addToScene(TileBuilder *tileBuilder,std::vector<WhirlyKitLoadedImage *>loadImages,int currentImage0,int currentImage1,WhirlyKitElevationChunk *loadElev,std::vector<WhirlyKit::ChangeRequest *> &changeRequests)
{
    // If it's a placeholder, we don't create geometry
    if (!loadImages.empty() && loadImages[0].type == WKLoadedImagePlaceholder)
    {
        placeholder = true;
        return true;
    }
    
    BasicDrawable *draw = NULL;
    BasicDrawable *skirtDraw = NULL;
    std::vector<Texture *> texs(loadImages.size(),NULL);
    if (tileBuilder->texAtlas)
        subTexs.resize(loadImages.size());
    if (!tileBuilder->buildTile(&nodeInfo, &draw, &skirtDraw, (!loadImages.empty() ? &texs : NULL), Point2f(1.0,1.0), Point2f(0.0,0.0), &loadImages, loadElev))
        return false;
//    if (![loader buildTile:&nodeInfo draw:&draw skirtDraw:&skirtDraw tex:(!loadImages.empty() ? &texs : NULL) activeTextures:loader.activeTextures texScale:Point2f(1.0,1.0) texOffset:Point2f(0.0,0.0) lines:layer.lineMode layer:layer imageData:&loadImages elevData:loadElev])
//        return false;
    drawId = draw->getId();
    skirtDrawId = (skirtDraw ? skirtDraw->getId() : EmptyIdentity);

    if (tileBuilder->texAtlas)
    {
        tileBuilder->texAtlas->addTexture(texs, NULL, NULL, subTexs[0], tileBuilder->scene->getMemManager(), changeRequests, tileBuilder->borderTexel);
        changeRequests.push_back(NULL);
    }
    for (unsigned int ii=0;ii<texs.size();ii++)
    {
        Texture *tex = texs[ii];
        if (tex)
        {
            if (tileBuilder->texAtlas)
            {
                if (ii == 0)
                {
                    if (draw)
                        draw->applySubTexture(-1,subTexs[0]);
                    if (skirtDraw)
                        skirtDraw->applySubTexture(-1,subTexs[0]);
                }                
                
                delete tex;
            } else {
                texIds.push_back(tex->getId());
                changeRequests.push_back(new AddTextureReq(tex));
            }
        } else
            texIds.push_back(EmptyIdentity);
    }
    
    // Now for the changes to the scene
    if (tileBuilder->drawAtlas)
    {
        tileBuilder->drawAtlas->addDrawable(draw,changeRequests);
        delete draw;
        if (skirtDraw)
        {
            tileBuilder->drawAtlas->addDrawable(skirtDraw,changeRequests);
            delete skirtDraw;
        }
    } else {
        changeRequests.push_back(new AddDrawableReq(draw));
        if (skirtDraw)
            changeRequests.push_back(new AddDrawableReq(skirtDraw));
    }
    
    // Just in case, we don't have any child drawables here
    for (unsigned int ii=0;ii<4;ii++)
    {
        childDrawIds[ii] = EmptyIdentity;
        childSkirtDrawIds[ii] = EmptyIdentity;
    }
    
    return true;
}

// Clean out the geometry and texture associated with the given tile
void LoadedTile::clearContents(TileBuilder *tileBuilder,ChangeSet &changeRequests)
{
    if (drawId != EmptyIdentity)
    {
        if (tileBuilder->drawAtlas)
            tileBuilder->drawAtlas->removeDrawable(drawId, changeRequests);
        else
            changeRequests.push_back(new RemDrawableReq(drawId));
        drawId = EmptyIdentity;
    }
    if (skirtDrawId != EmptyIdentity)
    {
        if (tileBuilder->drawAtlas)
            tileBuilder->drawAtlas->removeDrawable(skirtDrawId, changeRequests);
        else
            changeRequests.push_back(new RemDrawableReq(skirtDrawId));
        skirtDrawId = EmptyIdentity;
    }
    if (tileBuilder)
    {
        if (!subTexs.empty() && subTexs[0].texId != EmptyIdentity && tileBuilder->texAtlas)
            tileBuilder->texAtlas->removeTexture(subTexs[0], changeRequests);
        subTexs.clear();
    }
    for (unsigned int ii=0;ii<texIds.size();ii++)
        if (texIds[ii] != EmptyIdentity)
        {
            changeRequests.push_back(new RemTextureReq(texIds[ii]));
        }
    texIds.clear();
    for (unsigned int ii=0;ii<4;ii++)
    {
        if (childDrawIds[ii] != EmptyIdentity)
        {
            if (tileBuilder->drawAtlas)
                tileBuilder->drawAtlas->removeDrawable(childDrawIds[ii], changeRequests);
            else
                changeRequests.push_back(new RemDrawableReq(childDrawIds[ii]));
        }
        if (childSkirtDrawIds[ii] != EmptyIdentity)
        {
            if (tileBuilder->drawAtlas)
                tileBuilder->drawAtlas->removeDrawable(childSkirtDrawIds[ii], changeRequests);
            else
                changeRequests.push_back(new RemDrawableReq(childSkirtDrawIds[ii]));
        }
    }
}

// Make sure a given tile overlaps the real world
bool TileBuilder::isValidTile(const Mbr &theMbr)
{
    return (theMbr.overlaps(mbr));
}

// Update based on what children are doing
void LoadedTile::updateContents(TileBuilder *tileBuilder,LoadedTile *childTiles[],ChangeSet &changeRequests)
{
    bool childrenExist = false;
    
    if (placeholder)
        return;
    
    // Work through the possible children
    int whichChild = 0;
    for (unsigned int iy=0;iy<2;iy++)
        for (unsigned int ix=0;ix<2;ix++)
        {
            // Is it here?
            bool isPresent = false;
            Quadtree::Identifier childIdent(2*nodeInfo.ident.x+ix,2*nodeInfo.ident.y+iy,nodeInfo.ident.level+1);
//            LoadedTile *childTile = [loader getTile:childIdent];
            LoadedTile *childTile = childTiles[iy*2+ix];
            isPresent = childTile && !childTile->isLoading;
            
            // If it exists, make sure we're not representing it here
            if (isPresent)
            {
                // Turn the child back off
                if (childDrawIds[whichChild] != EmptyIdentity)
                {
                    if (tileBuilder->drawAtlas)
                    {
                        tileBuilder->drawAtlas->removeDrawable(childDrawIds[whichChild], changeRequests);
                        if (childSkirtDrawIds[whichChild])
                            tileBuilder->drawAtlas->removeDrawable(childSkirtDrawIds[whichChild], changeRequests);
                    } else {
                        changeRequests.push_back(new RemDrawableReq(childDrawIds[whichChild]));
                        if (childSkirtDrawIds[whichChild])
                            changeRequests.push_back(new RemDrawableReq(childSkirtDrawIds[whichChild]));
                    }
                    childDrawIds[whichChild] = EmptyIdentity;
                    childSkirtDrawIds[whichChild] = EmptyIdentity;
                }
                
                childrenExist = true;
            } else {
                // It's not there, so make sure we're faking it with our texture
                // May need to build the geometry
                if (childDrawIds[whichChild] == EmptyIdentity)
                {
                    Quadtree::NodeInfo childInfo = tileBuilder->tree->generateNode(childIdent);
                    if (tileBuilder->isValidTile(childInfo.mbr) && !placeholder)
                    {
                        BasicDrawable *childDraw = NULL;
                        BasicDrawable *childSkirtDraw = NULL;
                        tileBuilder->buildTile(&childInfo,&childDraw,&childSkirtDraw,NULL,Point2f(0.5,0.5),Point2f(0.5*ix,0.5*iy),nil,elevData);
//                        [loader buildTile:&childInfo draw:&childDraw skirtDraw:&childSkirtDraw tex:NULL activeTextures:loader.activeTextures texScale:Point2f(0.5,0.5) texOffset:Point2f(0.5*ix,0.5*iy) lines:layer.lineMode layer:layer imageData:nil elevData:elevData];
                        // Set this to change the color of child drawables.  Helpfull for debugging
                        //                        childDraw->setColor(RGBAColor(64,64,64,255));
                        childDrawIds[whichChild] = childDraw->getId();
                        if (childSkirtDraw)
                            childSkirtDrawIds[whichChild] = childSkirtDraw->getId();
                        if (!tileBuilder->lineMode && !texIds.empty())
                        {
                            childDraw->setTexId(0,texIds[0]);
                            if (childSkirtDraw)
                                childSkirtDraw->setTexId(0,texIds[0]);
                        }
                        if (tileBuilder->texAtlas)
                        {
                            if (childDraw)
                                childDraw->applySubTexture(-1,subTexs[0]);
                            if (childSkirtDraw)
                                childSkirtDraw->applySubTexture(-1,subTexs[0]);
                        }
                        if (tileBuilder->drawAtlas)
                        {
                            tileBuilder->drawAtlas->addDrawable(childDraw, changeRequests);
                            delete childDraw;
                            if (childSkirtDraw)
                            {
                                tileBuilder->drawAtlas->addDrawable(childSkirtDraw, changeRequests);
                                delete childSkirtDraw;
                            }
                        } else {
                            changeRequests.push_back(new AddDrawableReq(childDraw));
                            if (childSkirtDraw)
                                changeRequests.push_back(new AddDrawableReq(childSkirtDraw));
                        }
                    }
                }
            }
            
            whichChild++;
        }
    
    // No children, so turn the geometry for this tile back on
    if (!childrenExist)
    {
        if (drawId == EmptyIdentity && !placeholder)
        {
            BasicDrawable *draw = NULL;
            BasicDrawable *skirtDraw = NULL;
            tileBuilder->buildTile(&nodeInfo, &draw, &skirtDraw, NULL, Point2f(1.0,1.0), Point2f(0.0,0.0), nil, elevData);
//            [loader buildTile:&nodeInfo draw:&draw skirtDraw:&skirtDraw tex:NULL activeTextures:loader.activeTextures texScale:Point2f(1.0,1.0) texOffset:Point2f(0.0,0.0) lines:layer.lineMode layer:layer imageData:nil elevData:elevData];
            drawId = draw->getId();
            if (!texIds.empty())
                draw->setTexId(0,texIds[0]);
            if (skirtDraw)
            {
                skirtDrawId = skirtDraw->getId();
                if (!texIds.empty())
                    skirtDraw->setTexId(0,texIds[0]);
            }
            if (tileBuilder->texAtlas)
            {
                draw->applySubTexture(-1,subTexs[0]);
                if (skirtDraw)
                    skirtDraw->applySubTexture(-1,subTexs[0]);
            }
            if (tileBuilder->drawAtlas)
            {
                tileBuilder->drawAtlas->addDrawable(draw, changeRequests);
                delete draw;
                if (skirtDraw)
                {
                    tileBuilder->drawAtlas->addDrawable(skirtDraw, changeRequests);
                    delete skirtDraw;
                }
            } else {
                changeRequests.push_back(new AddDrawableReq(draw));
                if (skirtDraw)
                    changeRequests.push_back(new AddDrawableReq(skirtDraw));
            }
        }
        
        // Also turn off any children that may have been on
        for (unsigned int ii=0;ii<4;ii++)
        {
            if (childDrawIds[ii] != EmptyIdentity)
            {
                if (tileBuilder->drawAtlas)
                {
                    tileBuilder->drawAtlas->removeDrawable(childDrawIds[ii], changeRequests);
                    if (childSkirtDrawIds[ii] != EmptyIdentity)
                        tileBuilder->drawAtlas->removeDrawable(childSkirtDrawIds[ii], changeRequests);
                } else {
                    changeRequests.push_back(new RemDrawableReq(childDrawIds[ii]));
                    if (childSkirtDrawIds[ii] != EmptyIdentity)
                        changeRequests.push_back(new RemDrawableReq(childSkirtDrawIds[ii]));
                }
                childDrawIds[ii] = EmptyIdentity;
                childSkirtDrawIds[ii] = EmptyIdentity;
            }
        }
    } else {
        // Make sure our representation is off
        if (drawId != EmptyIdentity)
        {
            if (tileBuilder->drawAtlas)
            {
                tileBuilder->drawAtlas->removeDrawable(drawId, changeRequests);
                if (skirtDrawId != EmptyIdentity)
                    tileBuilder->drawAtlas->removeDrawable(skirtDrawId, changeRequests);
            } else {
                changeRequests.push_back(new RemDrawableReq(drawId));
                if (skirtDrawId != EmptyIdentity)
                    changeRequests.push_back(new RemDrawableReq(skirtDrawId));
            }
            drawId = EmptyIdentity;
            skirtDrawId = EmptyIdentity;
        }
    }
    
    //    tree->Print();
}
    
void LoadedTile::setCurrentImages(TileBuilder *tileBuilder,int whichImage0,int whichImage1,ChangeSet &changeRequests)
{
    std::vector<unsigned int> whichImages;
    if (whichImage0 != EmptyIdentity)
        whichImages.push_back(whichImage0);
    if (whichImage1 != EmptyIdentity)
        whichImages.push_back(whichImage1);
    if (tileBuilder->texAtlas)
    {
        for (unsigned int ii=0;ii<whichImages.size();ii++)
        {
            unsigned int whichImage = whichImages[ii];
            // Individual textures
            if (whichImage < texIds.size())
            {
                SimpleIdentity newTexId = texIds[whichImage];
                if (drawId != EmptyIdentity)
                    changeRequests.push_back(new DrawTexChangeRequest(drawId,0,newTexId));
                if (skirtDrawId != EmptyIdentity)
                    changeRequests.push_back(new DrawTexChangeRequest(skirtDrawId,0,newTexId));
                
                for (unsigned int ii=0;ii<4;ii++)
                {
                    if (childDrawIds[ii] != EmptyIdentity)
                        changeRequests.push_back(new DrawTexChangeRequest(childDrawIds[ii],0,newTexId));
                    if (childSkirtDrawIds[ii] != EmptyIdentity)
                        changeRequests.push_back(new DrawTexChangeRequest(childSkirtDrawIds[ii],0,newTexId));
                }
            }
        }
    }
}
    
void LoadedTile::setEnable(TileBuilder *tileBuilder, bool enable, ChangeSet &theChanges)
{
    if (drawId != EmptyIdentity)
        theChanges.push_back(new OnOffChangeRequest(drawId,enable));
    if (skirtDrawId != EmptyIdentity)
        theChanges.push_back(new OnOffChangeRequest(skirtDrawId,enable));

    for (unsigned int ii=0;ii<4;ii++)
    {
        if (childDrawIds[ii] != EmptyIdentity)
            theChanges.push_back(new OnOffChangeRequest(childDrawIds[ii],enable));
        if (childSkirtDrawIds[ii] != EmptyIdentity)
            theChanges.push_back(new OnOffChangeRequest(childSkirtDrawIds[ii],enable));
    }
}

void LoadedTile::Print(TileBuilder *tileBuilder)
{
    NSLog(@"Node (%d,%d,%d), drawId = %d",nodeInfo.ident.x,nodeInfo.ident.y,nodeInfo.ident.level,(int)drawId);
    for (unsigned int ii=0;ii<4;ii++)
    {
        NSLog(@" Child %d drawId = %d",ii,(int)childDrawIds[ii]);
    }
    std::vector<Quadtree::Identifier> childIdents;
    tileBuilder->tree->childrenForNode(nodeInfo.ident,childIdents);
    for (unsigned int ii=0;ii<childIdents.size();ii++)
        NSLog(@" Query child (%d,%d,%d)",childIdents[ii].x,childIdents[ii].y,childIdents[ii].level);
}

}
