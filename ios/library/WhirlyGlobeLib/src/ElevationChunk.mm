/*
 *  ElevationChunk.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/24/13.
 *  Copyright 2011-2015 mousebird consulting. All rights reserved.
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

#import "ElevationChunk.h"

using namespace Eigen;
using namespace WhirlyKit;


typedef enum {WhirlyKitElevationFloats,WhirlyKitElevationShorts} WhirlyKitElevationFormat;


@implementation WhirlyKitElevationGridChunk
{
    WhirlyKitElevationFormat dataType;
    NSData *data;
}

@synthesize sizeX = _sizeX, sizeY = _sizeY;


+ (WhirlyKitElevationGridChunk *)ElevationChunkWithRandomData
{
    int numX = 20;
    int numY = 20;
    float floatArray[numX*numY];
    for (unsigned int ii=0;ii<numX*numY;ii++)
        floatArray[ii] = drand48()*30000;
    NSMutableData *data = [[NSMutableData alloc] initWithBytes:floatArray length:sizeof(float)*numX*numY];

    return [[WhirlyKitElevationGridChunk alloc] initWithFloatData:data sizeX:numX sizeY:numY];
}

- (id)initWithFloatData:(NSData *)inData sizeX:(int)sizeX sizeY:(int)sizeY
{
    self = [super init];
    if (!self)
        return nil;
    
    _sizeX = sizeX;
    _sizeY = sizeY;
    dataType = WhirlyKitElevationFloats;
    data = inData;
    _noDataValue = -10000000;
    
    return self;
}

- (id)initWithShortData:(NSData *)inData sizeX:(int)sizeX sizeY:(int)sizeY
{
    self = [super init];
    if (!self)
        return nil;
    
    _sizeX = sizeX;
    _sizeY = sizeY;
    dataType = WhirlyKitElevationShorts;
    data = inData;
    _noDataValue = -10000000;
    
    return self;    
}

- (float)elevationAtX:(int)x y:(int)y
{
    if (!data)
        return 0.0;
    if ([data length] == 0)
        return 0.0;

    if (x < 0)  x = 0;
    if (y < 0)  y = 0;
    if (x >= _sizeX)  x = _sizeX-1;
    if (y >= _sizeY)  y = _sizeY-1;
    
    float ret = 0.0;
    switch (dataType)
    {
        case WhirlyKitElevationShorts:
            ret = ((short *)[data bytes])[y*_sizeX+x];
            break;
        case WhirlyKitElevationFloats:
            ret = ((float *)[data bytes])[y*_sizeX+x];
            break;
    }
    
    if (ret == _noDataValue)
        ret = 0.0;
    
    return ret;
}

- (float)interpolateElevationAtX:(float)x y:(float)y
{
    if (!data)
        return 0.0;
    
    float elevs[4];
    int minX = (int)x;
    int minY = (int)y;
    elevs[0] = [self elevationAtX:minX y:minY];
    elevs[1] = [self elevationAtX:minX+1 y:minY];
    elevs[2] = [self elevationAtX:minX+1 y:minY+1];
    elevs[3] = [self elevationAtX:minX y:minY+1];
    
    // Interpolate a new value
    float ta = (x-minX);
    float tb = (y-minY);
    float elev0 = (elevs[1]-elevs[0])*ta + elevs[0];
    float elev1 = (elevs[2]-elevs[3])*ta + elevs[3];
    float ret = (elev1-elev0)*tb + elev0;
    
    return ret;
}

- (void)buildSkirt:(BasicDrawable *)draw pts:(std::vector<Point3d> &)pts texCoord:(std::vector<TexCoord> &)texCoords skirtFactor:(float)skirtFactor haveElev:(bool)haveElev theCenter:(const Point3d &)theCenter
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

- (void)generateDrawables:(WhirlyKit::ElevationDrawInfo *)drawInfo chunk:(BasicDrawable **)draw skirts:(BasicDrawable **)skirtDraw;
{
    // Size of each chunk
    Point2f chunkSize = drawInfo->theMbr.ur() - drawInfo->theMbr.ll();

    int sphereTessX = _sizeX;
    int sphereTessY = _sizeY;

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
    GeoCoord geoLL(drawInfo->coordSys->localToGeographic(Point3d(chunkLL.x(),chunkLL.y(),0.0)));
    GeoCoord geoUR(drawInfo->coordSys->localToGeographic(Point3d(chunkUR.x(),chunkUR.y(),0.0)));
    
    // We'll set up and fill in the drawable
    BasicDrawable *chunk = new BasicDrawable("Tile Quad Loader",(sphereTessX+1)*(sphereTessY+1),2*sphereTessX*sphereTessY);
    if (drawInfo->useTileCenters)
        chunk->setMatrix(&drawInfo->transMat);
    
    if (drawInfo->activeTextures > 0)
        chunk->setTexId(drawInfo->activeTextures-1, EmptyIdentity);
    chunk->setDrawOffset(drawInfo->drawOffset);
    chunk->setDrawPriority(drawInfo->drawPriority);
    chunk->setVisibleRange(drawInfo->minVis, drawInfo->maxVis);
    chunk->setAlpha(drawInfo->hasAlpha);
    chunk->setColor(drawInfo->color);
    chunk->setLocalMbr(Mbr(Point2f(geoLL.x(),geoLL.y()),Point2f(geoUR.x(),geoUR.y())));
    chunk->setProgram(drawInfo->programId);
    int elevEntry = 0;
    if (drawInfo->includeElev)
        elevEntry = chunk->addAttribute(BDFloatType, "a_elev");
    // Single level mode uses Z to sort out priority
    //        if (singleLevel != -1)
    //        {
    //            chunk->setRequestZBuffer(true);
    //            chunk->setWriteZBuffer(true);
    //        }
    
    chunk->setType(GL_TRIANGLES);
    // Generate point, texture coords, and normals
    std::vector<Point3d> locs((sphereTessX+1)*(sphereTessY+1));
    std::vector<float> elevs;
    if (drawInfo->includeElev || drawInfo->useElevAsZ)
        elevs.resize((sphereTessX+1)*(sphereTessY+1));
    std::vector<TexCoord> texCoords((sphereTessX+1)*(sphereTessY+1));
    for (unsigned int iy=0;iy<sphereTessY+1;iy++)
        for (unsigned int ix=0;ix<sphereTessX+1;ix++)
        {
            float locZ = 0.0;
            if (!elevs.empty())
            {
                float whereX = ix*drawInfo->texScale.x() + (_sizeX-1)*drawInfo->texOffset.x();
                float whereY = iy*drawInfo->texScale.y() + (_sizeY-1)*drawInfo->texOffset.y();
                
                locZ = [self interpolateElevationAtX:whereX y:whereY];
                elevs[iy*(sphereTessX+1)+ix] = locZ;
            }
            // We don't want real elevations in the mesh, just off in another attribute
            if (!drawInfo->useElevAsZ)
                locZ = 0.0;
            Point3d loc3D = drawInfo->coordAdapter->localToDisplay(CoordSystemConvert3d(drawInfo->coordSys,sceneCoordSys,Point3d(chunkLL.x()+ix*incr.x(),chunkLL.y()+iy*incr.y(),locZ)));
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
                Point3d ptA_0 = locs[idx0];
                int idx1 = iy*(sphereTessX+1)+ix;
                Point3d ptA_1 = locs[idx1];
                int idx2 = (iy+1)*(sphereTessX+1)+(ix+1);
                Point3d ptA_2 = locs[idx2];
                Point3d normA = (ptA_2-ptA_1).cross(ptA_0-ptA_1);
                normA.normalize();
                chunk->addPoint(Point3d(ptA_0-drawInfo->chunkMidDisp));
                chunk->addTexCoord(-1,texCoords[idx0]);
                chunk->addNormal(normA);
                if (elevEntry != 0)
                    chunk->addAttributeValue(elevEntry, elevs[idx0]);
                
                chunk->addPoint(Point3d(ptA_1-drawInfo->chunkMidDisp));
                chunk->addTexCoord(-1,texCoords[idx1]);
                chunk->addNormal(normA);
                if (elevEntry != 0)
                    chunk->addAttributeValue(elevEntry, elevs[idx1]);
                
                chunk->addPoint(Point3d(ptA_2-drawInfo->chunkMidDisp));
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
                Point3d ptB_0 = ptA_2;
                idx1 = idx1;
                Point3d ptB_1 = ptA_1;
                idx2 = iy*(sphereTessX+1)+(ix+1);
                Point3d ptB_2 = locs[idx2];
                Point3d normB = (ptB_0-ptB_2).cross(ptB_1-ptB_2);
                normB.normalize();
                chunk->addPoint(Point3d(ptB_0-drawInfo->chunkMidDisp));
                chunk->addTexCoord(-1,texCoords[idx0]);
                chunk->addNormal(normB);
                if (elevEntry != 0)
                    chunk->addAttributeValue(elevEntry, elevs[idx0]);
                
                chunk->addPoint(Point3d(ptB_1-drawInfo->chunkMidDisp));
                chunk->addTexCoord(-1,texCoords[idx1]);
                chunk->addNormal(normB);
                if (elevEntry != 0)
                    chunk->addAttributeValue(elevEntry, elevs[idx1]);
                
                chunk->addPoint(Point3d(ptB_2-drawInfo->chunkMidDisp));
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
    }
    
    if (!drawInfo->ignoreEdgeMatching && !drawInfo->coordAdapter->isFlat() && skirtDraw)
    {
        // We'll set up and fill in the drawable
        BasicDrawable *skirtChunk = new BasicDrawable("Tile Quad Loader Skirt");
        if (drawInfo->useTileCenters)
            skirtChunk->setMatrix(&drawInfo->transMat);
        if (drawInfo->activeTextures > 0)
            skirtChunk->setTexId(drawInfo->activeTextures-1, EmptyIdentity);
        skirtChunk->setDrawOffset(drawInfo->drawOffset);
        skirtChunk->setDrawPriority(0);
        skirtChunk->setVisibleRange(drawInfo->minVis, drawInfo->maxVis);
        skirtChunk->setAlpha(drawInfo->hasAlpha);
        skirtChunk->setColor(drawInfo->color);
        skirtChunk->setLocalMbr(Mbr(Point2f(geoLL.x(),geoLL.y()),Point2f(geoUR.x(),geoUR.y())));
        skirtChunk->setType(GL_TRIANGLES);
        // We need the skirts rendered with the z buffer on, even if we're doing (mostly) pure sorting
        skirtChunk->setRequestZBuffer(true);
        skirtChunk->setProgram(drawInfo->programId);
        
        // We'll vary the skirt size a bit.  Otherwise the fill gets ridiculous when we're looking
        //  at the very highest levels.  On the other hand, this doesn't fix a really big large/small
        //  disparity
        float skirtFactor = 0.95;
        bool haveElev = drawInfo->useElevAsZ;
        // Leave the big skirts in place if we're doing real elevation
        if (!drawInfo->useElevAsZ)
            skirtFactor = 1.0 - 0.2 / (1<<drawInfo->ident.level);
        
        // Bottom skirt
        std::vector<Point3d> skirtLocs;
        std::vector<TexCoord> skirtTexCoords;
        for (unsigned int ix=0;ix<=sphereTessX;ix++)
        {
            skirtLocs.push_back(locs[ix]);
            skirtTexCoords.push_back(texCoords[ix]);
        }
        [self buildSkirt:skirtChunk pts:skirtLocs texCoord:skirtTexCoords skirtFactor:skirtFactor haveElev:haveElev theCenter:drawInfo->chunkMidDisp];
        // Top skirt
        skirtLocs.clear();
        skirtTexCoords.clear();
        for (int ix=sphereTessX;ix>=0;ix--)
        {
            skirtLocs.push_back(locs[(sphereTessY)*(sphereTessX+1)+ix]);
            skirtTexCoords.push_back(texCoords[(sphereTessY)*(sphereTessX+1)+ix]);
        }
        [self buildSkirt:skirtChunk pts:skirtLocs texCoord:skirtTexCoords skirtFactor:skirtFactor haveElev:haveElev theCenter:drawInfo->chunkMidDisp];
        // Left skirt
        skirtLocs.clear();
        skirtTexCoords.clear();
        for (int iy=sphereTessY;iy>=0;iy--)
        {
            skirtLocs.push_back(locs[(sphereTessX+1)*iy+0]);
            skirtTexCoords.push_back(texCoords[(sphereTessX+1)*iy+0]);
        }
        [self buildSkirt:skirtChunk pts:skirtLocs texCoord:skirtTexCoords skirtFactor:skirtFactor haveElev:haveElev theCenter:drawInfo->chunkMidDisp];
        // right skirt
        skirtLocs.clear();
        skirtTexCoords.clear();
        for (int iy=0;iy<=sphereTessY;iy++)
        {
            skirtLocs.push_back(locs[(sphereTessX+1)*iy+(sphereTessX)]);
            skirtTexCoords.push_back(texCoords[(sphereTessX+1)*iy+(sphereTessX)]);
        }
        [self buildSkirt:skirtChunk pts:skirtLocs texCoord:skirtTexCoords skirtFactor:skirtFactor haveElev:haveElev theCenter:drawInfo->chunkMidDisp];
        
        if (drawInfo->texs && !(drawInfo->texs)->empty() && !((*(drawInfo->texs))[0]))
            skirtChunk->setTexId(0,(*(drawInfo->texs))[0]->getId());
        *skirtDraw = skirtChunk;
    }
    
    if (drawInfo->coverPoles && !drawInfo->coordAdapter->isFlat())
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
            if (elevEntry != 0)
                chunk->addAttributeValue(elevEntry, 0.0);
            int northVert = chunk->getNumPoints()-1;
            
            // A line of points for the outer ring, but we can copy them
            int startOfLine = chunk->getNumPoints();
            int iy = sphereTessY;
            for (unsigned int ix=0;ix<sphereTessX+1;ix++)
            {
                Point3d pt = locs[(iy*(sphereTessX+1)+ix)];
                float elev = 0.0;
                if (!elevs.empty())
                    elev = elevs[(iy*(sphereTessX+1)+ix)];
                chunk->addPoint(Point3d(pt-drawInfo->chunkMidDisp));
                chunk->addNormal(Point3d(0,0,1.0));
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
        
        if (drawInfo->ident.y == 0)
        {
            TexCoord singleTexCoord(0.5,1.0);
            // One point for the south pole
            Point3d southPt(0,0,-1.0);
            chunk->addPoint(Point3d(southPt-drawInfo->chunkMidDisp));
            chunk->addTexCoord(-1,singleTexCoord);
            chunk->addNormal(Point3d(0,0,-1.0));
            if (elevEntry != 0)
                chunk->addAttributeValue(elevEntry, 0.0);
            int southVert = chunk->getNumPoints()-1;
            
            // A line of points for the outside ring, which we can copy
            int startOfLine = chunk->getNumPoints();
            int iy = 0;
            for (unsigned int ix=0;ix<sphereTessX+1;ix++)
            {
                Point3d pt = locs[(iy*(sphereTessX+1)+ix)];
                float elev = 0.0;
                if (!elevs.empty())
                    elev = elevs[(iy*(sphereTessX+1)+ix)];
                chunk->addPoint(Point3d(pt-drawInfo->chunkMidDisp));
                chunk->addNormal(Point3d(0,0,-1.0));
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
    
    if (drawInfo->texs && !(drawInfo->texs)->empty() && (*(drawInfo->texs))[0])
        chunk->setTexId(0,(*(drawInfo->texs))[0]->getId());
    
    *draw = chunk;
}


@end
