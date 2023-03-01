/*  LoadedTileNew.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/18.
 *  Copyright 2011-2021 Saildrone Inc.
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
 */

#import "LoadedTileNew.h"
#import "BasicDrawableBuilder.h"
#import "WhirlyKitLog.h"

using namespace Eigen;

namespace WhirlyKit
{

LoadedTileNew::LoadedTileNew(const QuadTreeNew::ImportantNode &ident,const MbrD &mbr) :
    ident(ident),
    mbr(mbr),
    tileNumber(ident.NodeNumber())
{
}

bool LoadedTileNew::isValidSpatial(TileGeomManager *geomManage) const
{
    const MbrD theMbr = geomManage->quadTree->generateMbrForNode(ident);

    // Make sure this overlaps the area we care about
    return geomManage->mbr.overlaps(theMbr);
}

static inline Point3d CoordSystemConvert(bool isSameCS, const CoordSystem *inSystem,
                                         const CoordSystem *outSystem,const Point3d &inCoord)
{
    // We'll go through geocentric which isn't horrible, but obviously we're assuming the same datum
    // Easy if the coordinate systems are the same
    return isSameCS ? inCoord : outSystem->geocentricToLocal(inSystem->localToGeocentric(inCoord));
}
static inline Point3d CoordSystemConvert(bool isSameCS, const CoordSystem *inSystem,
                                         const CoordSystem *outSystem,const Point2d &inCoord)
{
    return CoordSystemConvert(isSameCS, inSystem, outSystem, Pad(inCoord, 0.0));
}

// Clip the span of a tile to the bounds of the world in one dimension, calculating the scale
// and offset necessary to place the right part of the texture on the resulting geometry.
static std::tuple<double,double,double,double> clipDim(Point2d tile, Point2d world)
{
    // Limit the tile edges to the world span
    const auto lo = std::max(tile[0], world[0]);
    const auto hi = std::min(tile[1], world[1]);

    // The texture scale is the ratio of the visible portion of the tile to the whole tile
    const auto tileSpan = tile[1] - tile[0];
    const auto scale = (hi - lo) / tileSpan;

    // If we're adjusting the low side, we also need to offset the texture by that fraction
    const auto offset = (tile[0] - lo) / tileSpan;

    return { lo, hi, scale, offset };
}

void LoadedTileNew::makeDrawables(SceneRenderer *sceneRender,TileGeomManager *geomManage,
                                  const TileGeomSettings &geomSettings,ChangeSet &changes)
{
    enabled = true;

    // Don't bother to actually build the geometry in this case
    if (!geomSettings.buildGeom)
        return;

    MbrD theMbr = geomManage->quadTree->generateMbrForNode(ident);
    //const MbrD origMbr = theMbr;

    Point2d texScale(1, 1);
    Point2d texOffset(0, 0);
    std::tie(theMbr.ll().x(), theMbr.ur().x(), texScale.x(), texOffset.x()) = clipDim(theMbr.x(), geomManage->mbr.x());
    std::tie(theMbr.ll().y(), theMbr.ur().y(), texScale.y(), texOffset.y()) = clipDim(theMbr.y(), geomManage->mbr.y());

    //if (theMbr != origMbr)
    //{
    //    wkLog("Tile %d:%d,%d clip: %.8f,%.8f / %.8f,%.8f => %.8f,%.8f / %.8f,%.8f scale=%.4f,%.4f offset=%.4f,%.4f",
    //          ident.level, ident.x, ident.y,
    //          origMbr.ll().x(), origMbr.ll().y(), origMbr.ur().x(), origMbr.ur().y(),
    //          theMbr.ll().x(), theMbr.ll().y(), theMbr.ur().x(), theMbr.ur().y(),
    //          texScale.x(), texScale.y(), texOffset.x(), texOffset.y());
    //}

    // Calculate a center for the tile
    const CoordSystemDisplayAdapter *sceneAdapter = geomManage->coordAdapter;
    const CoordSystem *sceneCoordSys = sceneAdapter->getCoordSystem();
    const CoordSystem *geomCoordSys = geomManage->coordSys.get();
    const bool isSameCS = (geomCoordSys == sceneCoordSys) || geomCoordSys->isSameAs(sceneCoordSys);

    // Convert through geocentric, if necessary
    const Point3d ll = sceneAdapter->localToDisplay(CoordSystemConvert(isSameCS, geomCoordSys, sceneCoordSys, theMbr.ll()));
    const Point3d ur = sceneAdapter->localToDisplay(CoordSystemConvert(isSameCS, geomCoordSys, sceneCoordSys, theMbr.ur()));

    // This clips the center to something 32 bit floating point can represent.
    const Point3d dispCenter = ((ll + ur) / 2.0).cast<float>().cast<double>();

    // Translation for the middle.  The drawable stores floats which isn't high res enough zoomed way in
    const Point3d chunkMidDisp = (geomSettings.useTileCenters ? dispCenter : Point3d(0,0,0));
//        wkLogLevel(Debug,"id = %d: (%d,%d),mid = (%f,%f,%f)",ident.level,ident.x,ident.y,chunkMidDisp.x(),chunkMidDisp.y(),chunkMidDisp.z());
    const Eigen::Affine3d trans(Eigen::Translation3d(chunkMidDisp.x(),chunkMidDisp.y(),chunkMidDisp.z()));
    const Matrix4d &transMat = trans.matrix();

    // Size of each chunk
    const Point2d chunkSize = theMbr.ur() - theMbr.ll();
    
    int sphereTessX = geomSettings.sampleX,sphereTessY = geomSettings.sampleY;
    if (ident.level == 0)
    {
        sphereTessX = geomSettings.topSampleX;
        sphereTessY = geomSettings.topSampleY;
    }
    
    // For single level mode it's not worth getting fancy
    // Note: The level check is kind of a hack.  We're avoiding a resolution problem at high levels
    //    if (singleLevel || drawInfo->ident.level > 17)
    if (ident.level > 17)
    {
        sphereTessX = 1;
        sphereTessY = 1;
    }
    
    // Unit size of each tessellation in spherical mercator
    const Point2d sphereTess = Point2d(1.0 / sphereTessX, 1.0 / sphereTessY);
    const Point2d incr = chunkSize.cwiseProduct(sphereTess);

    // Texture increment for each tessellation
    const Point2d texIncr = sphereTess.cwiseProduct(texScale);

    // We need the corners in geographic for the cullable
    const Point2d chunkLL = theMbr.ll();
    const Point2d chunkUR = theMbr.ur();
    const GeoCoord geoLL(geomCoordSys->localToGeographic(Pad(chunkLL)));
    const GeoCoord geoUR(geomCoordSys->localToGeographic(Pad(chunkUR)));

    BasicDrawableBuilderRef chunk = sceneRender->makeBasicDrawableBuilder("LoadedTileNew chunk");
    chunk->reserve((sphereTessX+1)*(sphereTessY+1),2*sphereTessX*sphereTessY);
    // Note: Make this flexible
    chunk->setupTexCoordEntry(0, 0);

    // TODO: Revisit this with stencils
//    const auto drawOrder = tileNumber;
    const auto drawOrder = BaseInfo::DrawOrderTiles;
    chunk->setDrawOrder(drawOrder);

    std::vector<BasicDrawableBuilderRef> drawables;
    drawables.push_back(chunk);
    drawInfo.emplace_back(DrawableGeom,chunk->getDrawableID(),chunk->getDrawablePriority(),drawOrder);
    if (geomSettings.useTileCenters)
        chunk->setMatrix(&transMat);

    drawPriority = geomSettings.baseDrawPriority + ident.level * geomSettings.drawPriorityPerLevel;
    chunk->setDrawPriority(drawPriority);
    chunk->setVisibleRange(geomSettings.minVis, geomSettings.maxVis);
//    chunk->setColor(geomSettings.color);
    chunk->setLocalMbr(Mbr(Point2f(geoLL.x(),geoLL.y()),Point2f(geoUR.x(),geoUR.y())));
    chunk->setProgram(geomSettings.programID);
    chunk->setOnOff(false);

    // Might need another drawable for poles
    bool separatePoleChunk = false;
    BasicDrawableBuilderRef poleChunk;
    if (geomManage->coverPoles && (geomManage->useNorthPoleColor || geomManage->useSouthPoleColor))
    {
        poleChunk = sceneRender->makeBasicDrawableBuilder("LoadedTileNew poleChunk");
        poleChunk->setupTexCoordEntry(0, 0);
        drawables.push_back(poleChunk);
        if (geomSettings.useTileCenters)
            poleChunk->setMatrix(&transMat);
        poleChunk->setType(Triangles);
        poleChunk->setDrawOrder(drawOrder);
        poleChunk->setDrawPriority(drawPriority);
        poleChunk->setVisibleRange(geomSettings.minVis, geomSettings.maxVis);
//        poleChunk->setColor(geomSettings.color);
        poleChunk->setLocalMbr(Mbr(Point2f(geoLL.x(),geoLL.y()),Point2f(geoUR.x(),geoUR.y())));
        poleChunk->setProgram(geomSettings.programID);
        poleChunk->setOnOff(false);
        drawInfo.emplace_back(DrawablePole,poleChunk->getDrawableID(),poleChunk->getDrawablePriority(),drawOrder);
        separatePoleChunk = true;
    } else
        poleChunk = chunk;
    
    // We're in line mode or the texture didn't load
    if (geomSettings.lineMode)
    {
        chunk->setType(Lines);
        
        // Two lines per cell
        for (unsigned int iy=0;iy<sphereTessY;iy++)
            for (unsigned int ix=0;ix<sphereTessX;ix++)
            {
                const auto org3D  = sceneAdapter->localToDisplay(CoordSystemConvert(isSameCS, geomCoordSys, sceneCoordSys, Point2d(chunkLL.x()+ ix   *incr.x(),chunkLL.y()+ iy   *incr.y())));
                const auto ptA_3D = sceneAdapter->localToDisplay(CoordSystemConvert(isSameCS, geomCoordSys, sceneCoordSys, Point2d(chunkLL.x()+(ix+1)*incr.x(),chunkLL.y()+ iy   *incr.y())));
                const auto ptB_3D = sceneAdapter->localToDisplay(CoordSystemConvert(isSameCS, geomCoordSys, sceneCoordSys, Point2d(chunkLL.x()+ ix   *incr.x(),chunkLL.y()+(iy+1)*incr.y())));
                
                const TexCoord texCoord(ix*texIncr.x() - texOffset.x(),
                                        1.0f-(iy*texIncr.y()) + texOffset.y());
                
                chunk->addPoint(Point3d(org3D-chunkMidDisp));
                chunk->addNormal(org3D);
                chunk->addTexCoord(-1,texCoord);
                chunk->addPoint(Point3d(ptA_3D-chunkMidDisp));
                chunk->addNormal(ptA_3D);
                chunk->addTexCoord(-1,texCoord);
                
                chunk->addPoint(Point3d(org3D-chunkMidDisp));
                chunk->addNormal(org3D);
                chunk->addTexCoord(-1,texCoord);
                chunk->addPoint(Point3d(ptB_3D-chunkMidDisp));
                chunk->addNormal(ptB_3D);
                chunk->addTexCoord(-1,texCoord);
            }
    } else {
        chunk->setType(Triangles);
        // Generate point, texture coords, and normals
        Point3dVector locs((sphereTessX+1)*(sphereTessY+1));
        std::vector<float> elevs(geomSettings.includeElev ? (sphereTessX+1)*(sphereTessY+1) : 0);
        
        const Point3d wrapAt = sceneCoordSys->getWrapCoords();
        const bool enableWrap = !isSameCS && geomCoordSys->canBeWrapped() && (wrapAt.squaredNorm() > 0);
        const auto dmax = std::numeric_limits<double>::max();
        Point3d minLoc(dmax,dmax,dmax), maxLoc(-dmax,-dmax,-dmax), sgnLoc(0,0,0);
        for (unsigned int iy=0;iy<sphereTessY+1;iy++)
        {
            for (unsigned int ix=0;ix<sphereTessX+1;ix++)
            {
                constexpr auto locZ = 0.0;
                auto &loc = locs[iy*(sphereTessX+1)+ix];

                loc = Point3d(chunkLL.x() + ix * incr.x(),
                              chunkLL.y() + iy * incr.y(),
                              locZ);
                if (!isSameCS)
                {
                    loc = CoordSystemConvert(false, geomCoordSys, sceneCoordSys, loc);
                    if (enableWrap)
                    {
                        minLoc = minLoc.cwiseMin(loc);
                        maxLoc = maxLoc.cwiseMax(loc);
                        sgnLoc += Point3d(std::copysign(1.0, loc.x()),
                                          std::copysign(1.0, loc.y()),
                                          std::copysign(1.0, loc.z()));
                    }
                }
            }
        }

        // Check for antimeridian wrapping in coordinate conversion
        if (enableWrap)
        {
            // Do spans of the converted coordinates differ by more than that amount?
            const Point3d dwrap = maxLoc - minLoc;
            for (int axis = 0; axis < 3; ++axis)
            {
                if (wrapAt[axis] && dwrap[axis] > wrapAt[axis])
                {
                    // We assume it always wraps symmetrically centered on zero.
                    constexpr auto wrapAround = 0.0;
                    const auto wrapRight = (sgnLoc[axis] > 0);
                    const auto wrapVal = (wrapRight ? 2 : -2) * wrapAt[axis];
                    for (unsigned int iy=0;iy<sphereTessY+1;iy++)
                    {
                        for (unsigned int ix=0;ix<sphereTessX+1;ix++)
                        {
                            auto &loc = locs[iy*(sphereTessX+1)+ix];
                            if ((wrapRight && loc[axis] < wrapAround) ||
                                (!wrapRight && loc[axis] > wrapAround))
                            {
                                loc[axis] += wrapVal;
                            }
                        }
                    }
                }
            }
        }

        std::vector<TexCoord, Eigen::aligned_allocator<TexCoord>> texCoords((sphereTessX+1)*(sphereTessY+1));
        for (unsigned int iy=0;iy<sphereTessY+1;iy++)
        {
            for (unsigned int ix=0;ix<sphereTessX+1;ix++)
            {
                constexpr auto locZ = 0.0;

                auto &loc = locs[iy*(sphereTessX+1)+ix];
                loc = geomManage->coordAdapter->localToDisplay(loc);
                if (geomManage->coordAdapter->isFlat())
                {
                    loc.z() = locZ;
                }
                
                // Use Z priority to sort the levels
                //                    if (singleLevel != -1)
                //                        loc3D.z() = (drawPriority + nodeInfo->ident.level * 0.01)/10000;

                // Do the texture coordinate separately
                const TexCoord texCoord(ix*texIncr.x() - texOffset.x(),
                                        1.0f-(iy*texIncr.y()) + texOffset.y());
                texCoords[iy*(sphereTessX+1)+ix] = texCoord;
            }
        }
        
        // Without elevation data we can share the vertices
        for (unsigned int iy=0;iy<sphereTessY+1;iy++)
        {
            for (unsigned int ix=0;ix<sphereTessX+1;ix++)
            {
                const Point3d &loc3D = locs[iy*(sphereTessX+1)+ix];
                
                // And the normal
                Point3d norm3D;
                if (geomManage->coordAdapter->isFlat())
                    norm3D = geomManage->coordAdapter->normalForLocal(loc3D);
                else
                    norm3D = loc3D;
                
                const TexCoord &texCoord = texCoords[iy*(sphereTessX+1)+ix];
                
                chunk->addPoint(Point3d(loc3D-chunkMidDisp));
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
        
        if (geomManage->buildSkirts && !geomManage->coordAdapter->isFlat())
        {
            // We'll set up and fill in the drawable
            const auto skirtChunk = sceneRender->makeBasicDrawableBuilder("LoadedTileNew SkirtChunk");
            drawables.push_back(skirtChunk);
            if (geomSettings.useTileCenters)
                skirtChunk->setMatrix(&transMat);
            // We hard-wire this to appear after the atmosphere.  A bit hacky.
            skirtChunk->setupTexCoordEntry(0, 0);
            skirtChunk->setDrawOrder(drawOrder);
            skirtChunk->setDrawPriority(11);
            skirtChunk->setVisibleRange(geomSettings.minVis, geomSettings.maxVis);
//            skirtChunk->setColor(geomSettings.color);
            skirtChunk->setLocalMbr(Mbr(Point2f(geoLL.x(),geoLL.y()),Point2f(geoUR.x(),geoUR.y())));
            skirtChunk->setType(Triangles);
            // We need the skirts rendered with the z buffer on, even if we're doing (mostly) pure sorting
            skirtChunk->setRequestZBuffer(true);
            skirtChunk->setProgram(geomSettings.programID);
            skirtChunk->setOnOff(false);
            drawInfo.emplace_back(DrawableSkirt,skirtChunk->getDrawableID(),skirtChunk->getDrawablePriority(),drawOrder);

            // We'll vary the skirt size a bit.  Otherwise the fill gets ridiculous when we're looking
            //  at the very highest levels.  On the other hand, this doesn't fix a really big large/small
            //  disparity
            const auto skirtFactor = 1.0 - 0.2 / (1<<ident.level);
            
            // Bottom skirt
            Point3dVector skirtLocs;
            std::vector<TexCoord> skirtTexCoords;
            skirtLocs.reserve(sphereTessX);
            skirtTexCoords.reserve(sphereTessX);
            for (unsigned int ix=0;ix<=sphereTessX;ix++)
            {
                skirtLocs.push_back(locs[ix]);
                skirtTexCoords.push_back(texCoords[ix]);
            }
            buildSkirt(skirtChunk,skirtLocs,skirtTexCoords,skirtFactor,false,chunkMidDisp);
            // Top skirt
            skirtLocs.clear();
            skirtTexCoords.clear();
            for (int ix=sphereTessX;ix>=0;ix--)
            {
                skirtLocs.push_back(locs[(sphereTessY)*(sphereTessX+1)+ix]);
                skirtTexCoords.push_back(texCoords[(sphereTessY)*(sphereTessX+1)+ix]);
            }
            buildSkirt(skirtChunk,skirtLocs,skirtTexCoords,skirtFactor,false,chunkMidDisp);
            // Left skirt
            skirtLocs.clear();
            skirtTexCoords.clear();
            for (int iy=sphereTessY;iy>=0;iy--)
            {
                skirtLocs.push_back(locs[(sphereTessX+1)*iy+0]);
                skirtTexCoords.push_back(texCoords[(sphereTessX+1)*iy+0]);
            }
            buildSkirt(skirtChunk,skirtLocs,skirtTexCoords,skirtFactor,false,chunkMidDisp);
            // right skirt
            skirtLocs.clear();
            skirtTexCoords.clear();
            for (int iy=0;iy<=sphereTessY;iy++)
            {
                skirtLocs.push_back(locs[(sphereTessX+1)*iy+(sphereTessX)]);
                skirtTexCoords.push_back(texCoords[(sphereTessX+1)*iy+(sphereTessX)]);
            }
            buildSkirt(skirtChunk,skirtLocs,skirtTexCoords,skirtFactor,false,chunkMidDisp);
        }
        
        if (geomManage->coverPoles && !geomManage->coordAdapter->isFlat())
        {
            // If we're at the top, toss in a few more triangles to represent that
            const int maxY = 1 << ident.level;
            if (ident.y == maxY-1)
            {
                const TexCoord singleTexCoord(0.5,0.0);
                // One point for the north pole
                const Point3d northPt(0,0,1.0);
                poleChunk->addPoint(Point3d(northPt-chunkMidDisp));
                if (separatePoleChunk)
                    poleChunk->addColor(geomManage->northPoleColor);
                else
                    poleChunk->addTexCoord(-1,singleTexCoord);
                poleChunk->addNormal(Point3d(0,0,1.0));
                const unsigned northVert = poleChunk->getNumPoints()-1;
                
                // A line of points for the outer ring, but we can copy them
                const unsigned startOfLine = poleChunk->getNumPoints();
                const int iy = sphereTessY;
                for (unsigned int ix=0;ix<sphereTessX+1;ix++)
                {
                    const Point3d pt = locs[(iy*(sphereTessX+1)+ix)];
                    poleChunk->addPoint(Point3d(pt-chunkMidDisp));
                    if (geomManage->coordAdapter->isFlat())
                        poleChunk->addNormal(Point3d(0,0,1.0));
                    else
                        poleChunk->addNormal(pt);
                    if (separatePoleChunk)
                        poleChunk->addColor(geomManage->northPoleColor);
                    else
                        poleChunk->addTexCoord(-1,singleTexCoord);
                }
                
                // And define the triangles
                for (unsigned int ix=0;ix<sphereTessX;ix++)
                {
                    BasicDrawable::Triangle tri;
                    tri.verts[0] = startOfLine+ix;
                    tri.verts[1] = startOfLine+ix+1;
                    tri.verts[2] = northVert;
                    poleChunk->addTriangle(tri);
                }
            }
            
            if (ident.y == 0)
            {
                const TexCoord singleTexCoord(0.5,1.0);
                // One point for the south pole
                const Point3d southPt(0,0,-1.0);
                poleChunk->addPoint(Point3d(southPt-chunkMidDisp));
                if (separatePoleChunk)
                    poleChunk->addColor(geomManage->southPoleColor);
                else
                    poleChunk->addTexCoord(-1,singleTexCoord);
                poleChunk->addNormal(Point3d(0,0,-1.0));
                const unsigned southVert = poleChunk->getNumPoints()-1;
                
                // A line of points for the outside ring, which we can copy
                const unsigned startOfLine = poleChunk->getNumPoints();
                const int iy = 0;
                for (unsigned int ix=0;ix<sphereTessX+1;ix++)
                {
                    const Point3d pt = locs[(iy*(sphereTessX+1)+ix)];
                    poleChunk->addPoint(Point3d(pt-chunkMidDisp));
                    if (geomManage->coordAdapter->isFlat())
                        poleChunk->addNormal(Point3d(0,0,1.0));
                    else
                        poleChunk->addNormal(pt);
                    if (separatePoleChunk)
                        poleChunk->addColor(geomManage->southPoleColor);
                    else
                        poleChunk->addTexCoord(-1,singleTexCoord);
                }
                
                // And define the triangles
                for (unsigned int ix=0;ix<sphereTessX;ix++)
                {
                    BasicDrawable::Triangle tri;
                    tri.verts[0] = southVert;
                    tri.verts[1] = startOfLine+ix+1;
                    tri.verts[2] = startOfLine+ix;
                    poleChunk->addTriangle(tri);
                }
            }
        }
    }

    changes.reserve(changes.size() + drawables.size());
    for (const auto &draw : drawables) {
        changes.push_back(new AddDrawableReq(draw->getDrawable()));
    }
}
    
void LoadedTileNew::buildSkirt(const BasicDrawableBuilderRef &draw,const Point3dVector &pts,
                               const std::vector<TexCoord> &texCoords,double skirtFactor,
                               bool haveElev,const Point3d &theCenter)
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
        const unsigned base = draw->getNumPoints();
        for (unsigned int jj=0;jj<4;jj++)
        {
            draw->addPoint(Point3d(corners[jj]-theCenter));
            const Point3d norm = (pts[ii]+pts[ii+1])/2.f;
            draw->addNormal(norm);
            const TexCoord texCoord = cornerTex[jj];
            draw->addTexCoord(-1,texCoord);
        }
        
        // Add two triangles
        draw->addTriangle(BasicDrawable::Triangle(base+3,base+2,base+0));
        draw->addTriangle(BasicDrawable::Triangle(base+0,base+2,base+1));
    }
}
    
void LoadedTileNew::enable(const TileGeomSettings &geomSettings,ChangeSet &changes)
{
    if (geomSettings.enableGeom && !enabled) {
        changes.reserve(changes.size() + drawInfo.size());

        for (const auto &di : drawInfo) {
            changes.push_back(new OnOffChangeRequest(di.drawID,true));
        }
    }
    enabled = true;
}

void LoadedTileNew::disable(const TileGeomSettings &geomSettings,ChangeSet &changes)
{
    if (geomSettings.enableGeom && enabled) {
        changes.reserve(changes.size() + drawInfo.size());

        for (const auto &di : drawInfo) {
            changes.push_back(new OnOffChangeRequest(di.drawID,false));
        }
    }
    enabled = false;
}
    
void LoadedTileNew::removeDrawables(ChangeSet &changes)
{
    changes.reserve(changes.size() + drawInfo.size());

    for (const auto &di : drawInfo) {
        changes.push_back(new RemDrawableReq(di.drawID));
    }
}

void TileGeomManager::setup(SceneRenderer *inSceneRender,TileGeomSettings &geomSettings,
                            QuadTreeNew *inQuadTree,CoordSystemDisplayAdapter *inCoordAdapter,
                            CoordSystemRef inCoordSys,const MbrD &inMbr)
{
    sceneRender = inSceneRender;
    settings = geomSettings;
    quadTree = inQuadTree;
    coordAdapter = inCoordAdapter;
    coordSys = std::move(inCoordSys);
    mbr = inMbr;
}
    
TileGeomManager::NodeChanges TileGeomManager::addRemoveTiles(
        const QuadTreeNew::ImportantNodeSet &addTiles,
        const QuadTreeNew::NodeSet &removeTiles,ChangeSet &changes)
{
    NodeChanges nodeChanges;

    for (const auto &ident: removeTiles) {
        const auto it = tileMap.find(ident);
        if (it != tileMap.end()) {
            const auto &tile = it->second;
            tile->removeDrawables(changes);
            tileMap.erase(it);
        }
    }

    for (const auto &ident: addTiles) {
        // Look for an existing tile
        const auto it = tileMap.find(ident);
        if (it == tileMap.end()) {
            // Add a new one
            const MbrD nodeMbr = quadTree->generateMbrForNode(ident);
            const auto tile = std::make_shared<LoadedTileNew>(ident, nodeMbr);
            if (tile->isValidSpatial(this))
            {
                tile->makeDrawables(sceneRender,this,settings,changes);
                tileMap[ident] = tile;
                nodeChanges.addedTiles.push_back(tile);
            }
        }
    }
    
    updateParents(changes,nodeChanges.enabledTiles,nodeChanges.disabledTiles);
    
    return nodeChanges;
}

void TileGeomManager::cleanup(ChangeSet &changes)
{
    for (const auto &tileInst: tileMap) {
        const auto &tile = tileInst.second;
        tile->removeDrawables(changes);
    }
    
    tileMap.clear();
}
    
std::vector<LoadedTileNewRef> TileGeomManager::getTiles(const QuadTreeNew::NodeSet &tiles)
{
    std::vector<LoadedTileNewRef> retTiles;
    retTiles.reserve(tiles.size());

    for (const auto &ident: tiles) {
        const auto it = tileMap.find(ident);
        if (it != tileMap.end()) {
            const auto &tile = it->second;
            retTiles.push_back(tile);
        }
    }
    
    return retTiles;
}
    
LoadedTileVec TileGeomManager::getAllTiles()
{
    LoadedTileVec retTiles;
    retTiles.reserve(tileMap.size());

    for (const auto &tile: tileMap) {
        retTiles.push_back(tile.second);
    }
    
    return retTiles;
}
    
LoadedTileNewRef TileGeomManager::getTile(const QuadTreeNew::Node &ident)
{
    const auto it = tileMap.find(ident);
    return (it != tileMap.end()) ? it->second : LoadedTileNewRef();
}
    
void TileGeomManager::updateParents(ChangeSet &changes,LoadedTileVec &enabledNodes,LoadedTileVec &disabledNodes)
{
    // No parent logic with single level.  Everything is on.
    if (settings.singleLevel)
        return;
    
    for (const auto &entry : tileMap) {
        const auto ident = entry.first;
        const auto tile = entry.second;
        
        if (ident.level < quadTree->maxLevel-1) {
            bool childPresent = false;
            for (int iy=0;iy<2;iy++)
                for (int ix=0;ix<2;ix++) {
                    QuadTreeNew::Node child(ident.x*2+ix,ident.y*2+iy,ident.level+1);
                    if (tileMap.find(child) != tileMap.end()) {
                        childPresent = true;
                        break;
                    }
                }
            if (childPresent)
            {
                if (tile->enabled)
                {
                    disabledNodes.push_back(tile);
                    tile->disable(settings,changes);
                }
            } else {
                if (!tile->enabled) {
                    enabledNodes.push_back(tile);
                    tile->enable(settings,changes);
                }
            }
        }
    }
}

}
