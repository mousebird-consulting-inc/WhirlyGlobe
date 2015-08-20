/*
*  LoadedTile.mm
*  WhirlyGlobeLib
*
*  Created by Steve Gifford on 9/19/13.
*  Copyright 2011-2015 mousebird consulting
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
// Note: This works around an Android boost problem
#define _LITTLE_ENDIAN
#import <boost/math/special_functions/fpclassify.hpp>
#import "DynamicTextureAtlas.h"
#import "DynamicDrawableAtlas.h"

using namespace Eigen;

namespace WhirlyKit
{
    
LoadedImage::LoadedImage()
{
    
}
    
LoadedImage::~LoadedImage()
{
}

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
    fade(1.0),
    texAtlas(NULL),
    newDrawables(false),
    singleLevel(false),
    texAtlasPixelFudge(0.0),
    useTileCenters(true)
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
    
void TileBuilder::initAtlases(TileImageType imageType,int numImages,int textureAtlasSize,int sampleSizeX,int sampleSizeY)
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
        texAtlas->setPixelFudgeFactor(texAtlasPixelFudge);
        drawAtlas = new DynamicDrawableAtlas("Tile Quad Loader",SingleElementSize,DrawBufferSize,ElementBufferSize,scene->getMemManager(),NULL,programId);
        drawAtlas->setFade(fade);
        newDrawables = true;
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
void TileBuilder::buildSkirt(BasicDrawable *draw,Point3dVector &pts,std::vector<TexCoord> &texCoords,float skirtFactor,bool haveElev,const Point3d &theCenter)
{
    for (unsigned int ii=0;ii<pts.size()-1;ii++)
    {
        Point3d corners[4];
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
                            draw->addPoint(Point3d(corners[jj]-theCenter));
                            Point3d norm = (pts[ii]+pts[ii+1])/2.f;
                            draw->addNormal(norm);
                            TexCoord texCoord = cornerTex[jj];
                            draw->addTexCoord(-1,texCoord);
                        }
        
        // Add two triangles
        draw->addTriangle(BasicDrawable::Triangle(base+3,base+2,base+0));
        draw->addTriangle(BasicDrawable::Triangle(base+0,base+2,base+1));
    }
}

void TileBuilder::generateDrawables(ElevationDrawInfo *drawInfo,BasicDrawable **draw,BasicDrawable **skirtDraw)
{
    // Size of each chunk
    Point2f chunkSize = drawInfo->theMbr.ur() - drawInfo->theMbr.ll();
    
    int sphereTessX = defaultSphereTessX,sphereTessY = defaultSphereTessY;
    
    // For single level mode it's not worth getting fancy
    // Note: The level check is kind of a hack.  We're avoiding a resolution problem at high levels
    if (singleLevel || drawInfo->ident.level > 17)
    {
        sphereTessX = 1;
        sphereTessY = 1;
    }
    
    // Unit size of each tesselation in spherical mercator
    Point2d incr(chunkSize.x()/sphereTessX,chunkSize.y()/sphereTessY);
    
    // Texture increment for each tesselation
    TexCoord texIncr(1.0/(float)sphereTessX,1.0/(float)sphereTessY);
    
    // We're viewing this as a parameterization from ([0->1.0],[0->1.0]) so we'll
    //  break up these coordinates accordingly
    Point2f paramSize(1.0/(drawInfo->xDim*sphereTessX),1.0/(drawInfo->yDim*sphereTessY));
    
    // We need the corners in geographic for the cullable
    Point2d chunkLL(drawInfo->theMbr.ll().x(),drawInfo->theMbr.ll().y());
    Point2d chunkUR(drawInfo->theMbr.ur().x(),drawInfo->theMbr.ur().y());
    //    Point2d chunkMid = (chunkLL+chunkUR)/2.0;
    CoordSystem *sceneCoordSys = drawInfo->coordAdapter->getCoordSystem();
    GeoCoord geoLL(coordSys->localToGeographic(Point3d(chunkLL.x(),chunkLL.y(),0.0)));
    GeoCoord geoUR(coordSys->localToGeographic(Point3d(chunkUR.x(),chunkUR.y(),0.0)));
    
    BasicDrawable *chunk = new BasicDrawable("Tile Quad Loader",(sphereTessX+1)*(sphereTessY+1),2*sphereTessX*sphereTessY);
    if (useTileCenters)
        chunk->setMatrix(&drawInfo->transMat);
    
    if (activeTextures > 0)
        chunk->setTexId(activeTextures-1, EmptyIdentity);
    chunk->setDrawOffset(drawOffset);
    chunk->setDrawPriority(drawInfo->drawPriority);
    chunk->setVisibleRange(minVis, maxVis);
    chunk->setAlpha(hasAlpha);
    chunk->setColor(color);
    chunk->setLocalMbr(Mbr(Point2f(geoLL.x(),geoLL.y()),Point2f(geoUR.x(),geoUR.y())));
    chunk->setProgram(programId);
    
    // We're in line mode or the texture didn't load
    if (lineMode || (drawInfo->texs && !(drawInfo->texs)->empty() && !((*(drawInfo->texs))[0])))
    {
        chunk->setType(GL_LINES);
        
        // Two lines per cell
        for (unsigned int iy=0;iy<sphereTessY;iy++)
            for (unsigned int ix=0;ix<sphereTessX;ix++)
            {
                Point3d org3D = drawInfo->coordAdapter->localToDisplay(CoordSystemConvert3d(coordSys,sceneCoordSys,Point3d(chunkLL.x()+ix*incr.x(),chunkLL.y()+iy*incr.y(),0.0)));
                Point3d ptA_3D = drawInfo->coordAdapter->localToDisplay(CoordSystemConvert3d(coordSys,sceneCoordSys,Point3d(chunkLL.x()+(ix+1)*incr.x(),chunkLL.y()+iy*incr.y(),0.0)));
                Point3d ptB_3D = drawInfo->coordAdapter->localToDisplay(CoordSystemConvert3d(coordSys,sceneCoordSys,Point3d(chunkLL.x()+ix*incr.x(),chunkLL.y()+(iy+1)*incr.y(),0.0)));
                
                TexCoord texCoord(ix*texIncr.x()*drawInfo->texScale.x()+drawInfo->texOffset.x(),1.0-(iy*texIncr.y()*drawInfo->texScale.y()+drawInfo->texOffset.y()));
                
                chunk->addPoint(Point3d(org3D-drawInfo->chunkMidDisp));
                chunk->addNormal(org3D);
                chunk->addTexCoord(-1,texCoord);
                chunk->addPoint(Point3d(ptA_3D-drawInfo->chunkMidDisp));
                chunk->addNormal(ptA_3D);
                chunk->addTexCoord(-1,texCoord);
                
                chunk->addPoint(Point3d(org3D-drawInfo->chunkMidDisp));
                chunk->addNormal(org3D);
                chunk->addTexCoord(-1,texCoord);
                chunk->addPoint(Point3d(ptB_3D-drawInfo->chunkMidDisp));
                chunk->addNormal(ptB_3D);
                chunk->addTexCoord(-1,texCoord);
            }
    } else {
        chunk->setType(GL_TRIANGLES);
        // Generate point, texture coords, and normals
        std::vector<Point3d> locs((sphereTessX+1)*(sphereTessY+1));
        std::vector<float> elevs;
        if (includeElev || useElevAsZ)
            elevs.resize((sphereTessX+1)*(sphereTessY+1));
        std::vector<TexCoord> texCoords((sphereTessX+1)*(sphereTessY+1));
        for (unsigned int iy=0;iy<sphereTessY+1;iy++)
        {
            for (unsigned int ix=0;ix<sphereTessX+1;ix++)
            {
                float locZ = 0.0;
                Point3d loc3D = drawInfo->coordAdapter->localToDisplay(CoordSystemConvert3d(coordSys,sceneCoordSys,Point3d(chunkLL.x()+ix*incr.x(),chunkLL.y()+iy*incr.y(),locZ)));
                if (drawInfo->coordAdapter->isFlat())
                    loc3D.z() = locZ;
                
                // Use Z priority to sort the levels
                //                    if (singleLevel != -1)
                //                        loc3D.z() = (drawPriority + nodeInfo->ident.level * 0.01)/10000;
                
                locs[iy*(sphereTessX+1)+ix] = loc3D;
                
                // Do the texture coordinate seperately
                TexCoord texCoord(ix*texIncr.x()*drawInfo->texScale.x()+drawInfo->texOffset.x(),1.0-(iy*texIncr.y()*drawInfo->texScale.y()+drawInfo->texOffset.y()));
                texCoords[iy*(sphereTessX+1)+ix] = texCoord;
            }
        }
        
        // Without elevation data we can share the vertices
        for (unsigned int iy=0;iy<sphereTessY+1;iy++)
        {
            for (unsigned int ix=0;ix<sphereTessX+1;ix++)
            {
                Point3d &loc3D = locs[iy*(sphereTessX+1)+ix];
                
                // And the normal
                Point3d norm3D;
                if (drawInfo->coordAdapter->isFlat())
                    norm3D = drawInfo->coordAdapter->normalForLocal(loc3D);
                else
                    norm3D = loc3D;
                
                TexCoord &texCoord = texCoords[iy*(sphereTessX+1)+ix];
                
                chunk->addPoint(Point3d(loc3D-drawInfo->chunkMidDisp));
                chunk->addNormal(norm3D);
                chunk->addTexCoord(-1,texCoord);
            }
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
        
        if (!drawInfo->ignoreEdgeMatching && !drawInfo->coordAdapter->isFlat() && skirtDraw)
        {
            // We'll set up and fill in the drawable
            BasicDrawable *skirtChunk = new BasicDrawable("Tile Quad Loader Skirt");
            if (useTileCenters)
                skirtChunk->setMatrix(&drawInfo->transMat);
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
            float skirtFactor = 1.0 - 0.2 / (1<<drawInfo->ident.level);
            
            // Bottom skirt
            Point3dVector skirtLocs;
            std::vector<TexCoord> skirtTexCoords;
            for (unsigned int ix=0;ix<=sphereTessX;ix++)
            {
                skirtLocs.push_back(locs[ix]);
                skirtTexCoords.push_back(texCoords[ix]);
            }
            buildSkirt(skirtChunk,skirtLocs,skirtTexCoords,skirtFactor,false,drawInfo->chunkMidDisp);
            // Top skirt
            skirtLocs.clear();
            skirtTexCoords.clear();
            for (int ix=sphereTessX;ix>=0;ix--)
            {
                skirtLocs.push_back(locs[(sphereTessY)*(sphereTessX+1)+ix]);
                skirtTexCoords.push_back(texCoords[(sphereTessY)*(sphereTessX+1)+ix]);
            }
            buildSkirt(skirtChunk,skirtLocs,skirtTexCoords,skirtFactor,false,drawInfo->chunkMidDisp);
            // Left skirt
            skirtLocs.clear();
            skirtTexCoords.clear();
            for (int iy=sphereTessY;iy>=0;iy--)
            {
                skirtLocs.push_back(locs[(sphereTessX+1)*iy+0]);
                skirtTexCoords.push_back(texCoords[(sphereTessX+1)*iy+0]);
            }
            buildSkirt(skirtChunk,skirtLocs,skirtTexCoords,skirtFactor,false,drawInfo->chunkMidDisp);
            // right skirt
            skirtLocs.clear();
            skirtTexCoords.clear();
            for (int iy=0;iy<=sphereTessY;iy++)
            {
                skirtLocs.push_back(locs[(sphereTessX+1)*iy+(sphereTessX)]);
                skirtTexCoords.push_back(texCoords[(sphereTessX+1)*iy+(sphereTessX)]);
            }
            buildSkirt(skirtChunk,skirtLocs,skirtTexCoords,skirtFactor,false,drawInfo->chunkMidDisp);
            
            if (drawInfo->texs && !(drawInfo->texs)->empty() && !((*(drawInfo->texs))[0]))
                skirtChunk->setTexId(0,(*(drawInfo->texs))[0]->getId());
            *skirtDraw = skirtChunk;
        }
        
        if (coverPoles && !drawInfo->coordAdapter->isFlat())
        {
            // If we're at the top, toss in a few more triangles to represent that
            int maxY = 1 << drawInfo->ident.level;
            if (drawInfo->ident.y == maxY-1)
            {
                TexCoord singleTexCoord(0.5,0.0);
                // One point for the north pole
                Point3d northPt(0,0,1.0);
                chunk->addPoint(Point3d(northPt-drawInfo->chunkMidDisp));
                chunk->addTexCoord(-1,singleTexCoord);
                chunk->addNormal(Point3d(0,0,1.0));
                int northVert = chunk->getNumPoints()-1;
                
                // A line of points for the outer ring, but we can copy them
                int startOfLine = chunk->getNumPoints();
                int iy = sphereTessY;
                for (unsigned int ix=0;ix<sphereTessX+1;ix++)
                {
                    Point3d pt = locs[(iy*(sphereTessX+1)+ix)];
                    chunk->addPoint(Point3d(pt-drawInfo->chunkMidDisp));
                    chunk->addNormal(Point3d(0,0,1.0));
                    chunk->addTexCoord(-1,singleTexCoord);
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
            
            if (drawInfo->ident.y == 0)
            {
                TexCoord singleTexCoord(0.5,1.0);
                // One point for the south pole
                Point3d southPt(0,0,-1.0);
                chunk->addPoint(Point3d(southPt-drawInfo->chunkMidDisp));
                chunk->addTexCoord(-1,singleTexCoord);
                chunk->addNormal(Point3d(0,0,-1.0));
                int southVert = chunk->getNumPoints()-1;
                
                // A line of points for the outside ring, which we can copy
                int startOfLine = chunk->getNumPoints();
                int iy = 0;
                for (unsigned int ix=0;ix<sphereTessX+1;ix++)
                {
                    Point3d pt = locs[(iy*(sphereTessX+1)+ix)];
                    chunk->addPoint(Point3d(pt-drawInfo->chunkMidDisp));
                    chunk->addNormal(Point3d(0,0,-1.0));
                    chunk->addTexCoord(-1,singleTexCoord);
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
        
        if (drawInfo->texs && !(drawInfo->texs)->empty() && (*(drawInfo->texs))[0])
            chunk->setTexId(0,(*(drawInfo->texs))[0]->getId());
    }
    
    *draw = chunk;
}


bool TileBuilder::buildTile(Quadtree::NodeInfo *nodeInfo,BasicDrawable **draw,BasicDrawable **skirtDraw,std::vector<Texture *> *texs,
                            Point2f texScale,Point2f texOffset,std::vector<LoadedImage *> *loadImages,const Point3d &dispCenter,Quadtree::NodeInfo *parentNodeInfo)
{
    Mbr theMbr = nodeInfo->mbr;
    
    // Make sure this overlaps the area we care about
    if (!theMbr.overlaps(mbr))
    {
//        NSLog(@"Building bogus tile: (%d,%d,%d)",nodeInfo->ident.x,nodeInfo->ident.y,nodeInfo->ident.level);
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
    
    // Translation for the middle.  The drawable stores floats which isn't high res enough zoomed way in
    Point3d chunkMidDisp = (useTileCenters ? dispCenter : Point3d(0,0,0));
    //    NSLog(@"mid = (%f,%f,%f)",chunkMidDisp.x(),chunkMidDisp.y(),chunkMidDisp.z());
    Eigen::Affine3d trans(Eigen::Translation3d(chunkMidDisp.x(),chunkMidDisp.y(),chunkMidDisp.z()));
    Matrix4d transMat = trans.matrix();
    
    // Get textures (locally)
    if (texs)
    {
        bool texturesClean = true;
        if (loadImages && (*loadImages)[0]->getType() != WKLoadedImagePlaceholder)
        {
            // They'll all be the same width
            LoadedImage *loadImage = (*loadImages)[0];
            int destWidth,destHeight;
            textureSize(loadImage->getWidth(),loadImage->getHeight(),&destWidth,&destHeight);
            
            // Create a texture for each
            for (unsigned int ii=0;ii<loadImages->size();ii++)
            {
                Texture *newTex = (*loadImages)[ii]->buildTexture(borderTexel,destWidth,destHeight);
                
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
        ElevationDrawInfo drawInfo;
        drawInfo.theMbr = theMbr;
        if (parentNodeInfo)
            drawInfo.parentMbr = parentNodeInfo->mbr;
        else
            drawInfo.parentMbr = theMbr;
        drawInfo.xDim = xDim;        drawInfo.yDim = yDim;
        drawInfo.coverPoles = coverPoles;
        drawInfo.useTileCenters = useTileCenters;
        drawInfo.texScale = texScale;
        drawInfo.texOffset = texOffset;
        drawInfo.dispCenter = dispCenter;
        drawInfo.transMat = transMat;
        drawInfo.drawPriority = drawPriority;
        if (singleLevel)
            drawInfo.drawPriority += nodeInfo->ident.level;
        drawInfo.texs = texs;
        drawInfo.coordAdapter = scene->getCoordAdapter();
        drawInfo.coordSys = coordSys;
        drawInfo.chunkMidDisp = chunkMidDisp;
        drawInfo.ignoreEdgeMatching = ignoreEdgeMatching;
        drawInfo.ident = nodeInfo->ident;
        drawInfo.activeTextures = activeTextures;
        drawInfo.drawOffset = drawOffset;
        drawInfo.minVis = minVis;
        drawInfo.maxVis = maxVis;
        drawInfo.hasAlpha = hasAlpha;
        drawInfo.color = color;
        drawInfo.programId = programId;
        drawInfo.includeElev = includeElev;
        drawInfo.useElevAsZ = useElevAsZ;
        drawInfo.lineMode = lineMode;

        // Note: Porting
        // Have the elevation provider generate the drawables
//        if (elevData)
//            [elevData generateDrawables:&drawInfo chunk:draw skirts:skirtDraw];
//        else {
            // No elevation provider, so we'll do it ourselves
            generateDrawables(&drawInfo,draw,skirtDraw);
//        }
    }
    
    return true;
}
    
Texture *TileBuilder::buildTexture(LoadedImage *loadImage)
{
    // They'll all be the same width
    int destWidth,destHeight;
    textureSize(loadImage->getWidth(),loadImage->getHeight(),&destWidth,&destHeight);
    
    Texture *newTex = loadImage->buildTexture(borderTexel,destWidth,destHeight);
    
    if (newTex)
    {
        newTex->setFormat(glFormat);
        newTex->setSingleByteSource(singleByteSource);
    }
    
    return newTex;
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
    
//void TileBuilder::log(NSString *name)
//{
//    if (!drawAtlas && !texAtlas)
//        return;
//    
//    NSLog(@"++ Quad Tile Loader %@ ++",(name ? name : @"Unknown"));
//    if (drawAtlas)
//        drawAtlas->log();
//    if (texAtlas)
//        texAtlas->log();
//    NSLog(@"++ ++ ++");
//}

InternalLoadedTile::InternalLoadedTile()
{
    isInitialized = false;
    isLoading = false;
    placeholder = false;
    drawId = EmptyIdentity;
    skirtDrawId = EmptyIdentity;
    dispCenter = Point3d(0,0,0);
    tileSize = 0.0;
    for (unsigned int ii=0;ii<4;ii++)
    {
        childDrawIds[ii] = EmptyIdentity;
        childSkirtDrawIds[ii] = EmptyIdentity;
    }
}

InternalLoadedTile::InternalLoadedTile(const WhirlyKit::Quadtree::Identifier &ident)
{
    nodeInfo.ident = ident;
    isInitialized = false;
    isLoading = false;
    placeholder = false;
    drawId = EmptyIdentity;
    skirtDrawId = EmptyIdentity;
    // Note: Porting
//    elevData = nil;
    dispCenter = Point3d(0,0,0);
    tileSize = 0.0;
    for (unsigned int ii=0;ii<4;ii++)
    {
        childDrawIds[ii] = EmptyIdentity;
        childSkirtDrawIds[ii] = EmptyIdentity;
    }
}

void InternalLoadedTile::calculateSize(Quadtree *quadTree,CoordSystemDisplayAdapter *coordAdapt,CoordSystem *coordSys)
{
    Mbr mbr = quadTree->generateMbrForNode(nodeInfo.ident);
    CoordSystem *sceneCoordSys = coordAdapt->getCoordSystem();
    Point3d ll = coordAdapt->localToDisplay(sceneCoordSys->geocentricToLocal(coordSys->localToGeocentric(Point3d(mbr.ll().x(),mbr.ll().y(),0.0))));
    Point3d ur = coordAdapt->localToDisplay(sceneCoordSys->geocentricToLocal(coordSys->localToGeocentric(Point3d(mbr.ur().x(),mbr.ur().y(),0.0))));
    dispCenter = (ll+ur)/2.0;
    tileSize = (ll-ur).norm();
}

// Note: This only works with texture atlases
bool InternalLoadedTile::updateTexture(TileBuilder *tileBuilder,LoadedImage *loadImage,int frame,std::vector<WhirlyKit::ChangeRequest *> &changeRequests)
{
    if (!loadImage || loadImage->isPlaceholder())
    {
        placeholder = true;
        return true;
    }
    
    Texture *newTex = tileBuilder->buildTexture(loadImage);
    
    if (tileBuilder->texAtlas)
    {
        tileBuilder->texAtlas->updateTexture(newTex, frame, texRegion, changeRequests);
        changeRequests.push_back(NULL);
    }
    
    delete newTex;
    
    return true;
}

// Add the geometry and texture to the scene for a given tile
bool InternalLoadedTile::addToScene(TileBuilder *tileBuilder,std::vector<LoadedImage *>loadImages,int frame,int currentImage0,int currentImage1,std::vector<WhirlyKit::ChangeRequest *> &changeRequests)
{
    isInitialized = true;

    // If it's a placeholder, we don't create geometry
    if (!loadImages.empty() && loadImages[0]->isPlaceholder())
    {
        placeholder = true;
        return true;
    }
    
    BasicDrawable *draw = NULL;
    BasicDrawable *skirtDraw = NULL;
    std::vector<Texture *> texs(loadImages.size(),NULL);
    if (tileBuilder->texAtlas)
        subTexs.resize(loadImages.size());
    if (!tileBuilder->buildTile(&nodeInfo, &draw, &skirtDraw, (!loadImages.empty() ? &texs : NULL), Point2f(1.0,1.0), Point2f(0.0,0.0), &loadImages,dispCenter,NULL))
        return false;
    drawId = draw->getId();
    skirtDrawId = (skirtDraw ? skirtDraw->getId() : EmptyIdentity);

    if (tileBuilder->texAtlas)
    {
        tileBuilder->texAtlas->addTexture(texs, frame, NULL, NULL, subTexs[0], tileBuilder->scene->getMemManager(), changeRequests, tileBuilder->borderTexel);
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

    // Tex IDs for new drawables
    std::vector<SimpleIdentity> startTexIDs;
    if (subTexs.size() > 0)
    {
        if (currentImage0 != -1 && currentImage1 != -1)
        {
            SimpleIdentity baseTexID = subTexs[0].texId;
            startTexIDs.push_back(tileBuilder->texAtlas->getTextureIDForFrame(baseTexID, currentImage0));
            startTexIDs.push_back(tileBuilder->texAtlas->getTextureIDForFrame(baseTexID, currentImage1));
        }
    }

    // Now for the changes to the scene
    if (tileBuilder->drawAtlas)
    {
        bool addedBigDraw = false;
        tileBuilder->drawAtlas->addDrawable(draw,changeRequests,true,&startTexIDs,&addedBigDraw,&dispCenter,tileSize);
        tileBuilder->newDrawables |= addedBigDraw;
        delete draw;
        if (skirtDraw)
        {
            tileBuilder->drawAtlas->addDrawable(skirtDraw,changeRequests,true,&startTexIDs,&addedBigDraw,&dispCenter,tileSize);
            tileBuilder->newDrawables |= addedBigDraw;
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
void InternalLoadedTile::clearContents(TileBuilder *tileBuilder,ChangeSet &changeRequests)
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
void InternalLoadedTile::updateContents(TileBuilder *tileBuilder,InternalLoadedTile *childTiles[],int currentImage0,int currentImage1,ChangeSet &changeRequests)
{
    bool childrenExist = false;
    
    if (placeholder)
        return;
    
    // Tex IDs for new drawables
    std::vector<SimpleIdentity> startTexIDs;
    if (subTexs.size() > 0)
    {
        if (currentImage0 != -1 && currentImage1 != -1)
        {
            SimpleIdentity baseTexID = subTexs[0].texId;
            startTexIDs.push_back(tileBuilder->texAtlas->getTextureIDForFrame(baseTexID, currentImage0));
            startTexIDs.push_back(tileBuilder->texAtlas->getTextureIDForFrame(baseTexID, currentImage1));
        }
    }
    
    // Work through the possible children
    int whichChild = 0;
    for (unsigned int iy=0;iy<2;iy++)
        for (unsigned int ix=0;ix<2;ix++)
        {
            // Is it here?
            bool isPresent = false;
            Quadtree::Identifier childIdent(2*nodeInfo.ident.x+ix,2*nodeInfo.ident.y+iy,nodeInfo.ident.level+1);
//            LoadedTile *childTile = [loader getTile:childIdent];
            InternalLoadedTile *childTile = childTiles[iy*2+ix];
            isPresent = childTile && childTile->isInitialized;
            
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
                        tileBuilder->buildTile(&childInfo,&childDraw,&childSkirtDraw,NULL,Point2f(0.5,0.5),Point2f(0.5*ix,0.5*iy),NULL,dispCenter,&this->nodeInfo);
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
                            if (childDraw && !subTexs.empty())
                                childDraw->applySubTexture(-1,subTexs[0]);
                            if (childSkirtDraw && !subTexs.empty())
                                childSkirtDraw->applySubTexture(-1,subTexs[0]);
                        }
                        if (tileBuilder->drawAtlas)
                        {
                            bool addedBigDrawable = false;
                            tileBuilder->drawAtlas->addDrawable(childDraw, changeRequests,true,&startTexIDs,&addedBigDrawable,&dispCenter,tileSize);
                            tileBuilder->newDrawables |= addedBigDrawable;
                            delete childDraw;
                            if (childSkirtDraw)
                            {
                                tileBuilder->drawAtlas->addDrawable(childSkirtDraw, changeRequests,true,&startTexIDs,&addedBigDrawable,&dispCenter,tileSize);
                                tileBuilder->newDrawables |= addedBigDrawable;
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
            tileBuilder->buildTile(&nodeInfo, &draw, &skirtDraw, NULL, Point2f(1.0,1.0), Point2f(0.0,0.0), NULL, dispCenter, NULL);
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
                bool addedBigDrawable = false;
                tileBuilder->drawAtlas->addDrawable(draw, changeRequests,true,&startTexIDs,&addedBigDrawable,&dispCenter,tileSize);
                tileBuilder->newDrawables |= addedBigDrawable;
                delete draw;
                if (skirtDraw)
                {
                    tileBuilder->drawAtlas->addDrawable(skirtDraw, changeRequests,true,&startTexIDs,&addedBigDrawable,&dispCenter,tileSize);
                    tileBuilder->newDrawables |= addedBigDrawable;
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
    
void InternalLoadedTile::setCurrentImages(TileBuilder *tileBuilder,int whichImage0,int whichImage1,ChangeSet &changeRequests)
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
    
void InternalLoadedTile::setEnable(TileBuilder *tileBuilder, bool enable, ChangeSet &theChanges)
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
    
// Note: This does nothing
void InternalLoadedTile::setFade(TileBuilder *tileBuilder, float fade, ChangeSet &theChanges)
{
    //    if (drawId != EmptyIdentity)
    //        theChanges.push_back(new FadeChangeRequest(drawId,fade));
    //    if (skirtDrawId != EmptyIdentity)
    //        theChanges.push_back(new FadeChangeRequest(skirtDrawId,fade));
    //
    //    for (unsigned int ii=0;ii<4;ii++)
    //    {
    //        if (childDrawIds[ii] != EmptyIdentity)
    //            theChanges.push_back(new FadeChangeRequest(childDrawIds[ii],fade));
    //        if (childSkirtDrawIds[ii] != EmptyIdentity)
    //            theChanges.push_back(new FadeChangeRequest(childSkirtDrawIds[ii],fade));
    //    }
}

void InternalLoadedTile::Print(TileBuilder *tileBuilder)
{
    // Note: Porting
//    NSLog(@"Node (%d,%d,%d), drawId = %d",nodeInfo.ident.x,nodeInfo.ident.y,nodeInfo.ident.level,(int)drawId);
//    for (unsigned int ii=0;ii<4;ii++)
//    {
//        NSLog(@" Child %d drawId = %d",ii,(int)childDrawIds[ii]);
//    }
//    std::vector<Quadtree::Identifier> childIdents;
//    tileBuilder->tree->childrenForNode(nodeInfo.ident,childIdents);
//    for (unsigned int ii=0;ii<childIdents.size();ii++)
//        NSLog(@" Query child (%d,%d,%d)",childIdents[ii].x,childIdents[ii].y,childIdents[ii].level);
}

}
