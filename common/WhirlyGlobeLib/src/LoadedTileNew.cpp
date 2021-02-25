/*
 *  LoadedTileNew.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/18.
 *  Copyright 2011-2019 Saildrone Inc.
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

#import "LoadedTileNew.h"
#import "BasicDrawableBuilder.h"
#import "WhirlyKitLog.h"

using namespace Eigen;

namespace WhirlyKit
{
    
TileGeomSettings::TileGeomSettings()
    : buildGeom(true), useTileCenters(true), color(RGBAColor(255,255,255,255)),
      programID(0), sampleX(10), sampleY(10), topSampleX(10), topSampleY(10),
      minVis(DrawVisibleInvalid), maxVis(DrawVisibleInvalid),
      baseDrawPriority(0), drawPriorityPerLevel(1), lineMode(false),
      includeElev(false), enableGeom(true), singleLevel(false)
{
}
    
LoadedTileNew::LoadedTileNew(const QuadTreeNew::ImportantNode &ident,const MbrD &mbr)
    : ident(ident), mbr(mbr), enabled(false),
      tileNumber(ident.NodeNumber())
{
}
    
bool LoadedTileNew::isValidSpatial(TileGeomManager *geomManage)
{
    MbrD theMbr = geomManage->quadTree->generateMbrForNode(ident);

    // Make sure this overlaps the area we care about
    return geomManage->mbr.inside(theMbr.mid());
}

void LoadedTileNew::makeDrawables(SceneRenderer *sceneRender,TileGeomManager *geomManage,TileGeomSettings &geomSettings,ChangeSet &changes)
{
    enabled = true;
    MbrD theMbr = geomManage->quadTree->generateMbrForNode(ident);
    
    // Don't bother to actually build the geometry in this case
    if (!geomSettings.buildGeom)
        return;
    
    // Scale texture coordinates if we're clipping this tile
    Point2d texScale(1.0,1.0);
    const Point2d texOffset(0.0,0.0);   // Note: Not using this
    
    // Snap to the designated area
    if (theMbr.ll().x() < geomManage->mbr.ll().x()) {
        theMbr.ll().x() = geomManage->mbr.ll().x();
    }
    if (theMbr.ur().x() > geomManage->mbr.ur().x()) {
        texScale.x() = (geomManage->mbr.ur().x()-theMbr.ll().x())/(theMbr.ur().x()-theMbr.ll().x());
        theMbr.ur().x() = geomManage->mbr.ur().x();
    }
    if (theMbr.ll().y() < geomManage->mbr.ll().y()) {
        theMbr.ll().y() = geomManage->mbr.ll().y();
    }
    if (theMbr.ur().y() > geomManage->mbr.ur().y()) {
        texScale.y() = (geomManage->mbr.ur().y()-theMbr.ll().y())/(theMbr.ur().y()-theMbr.ll().y());
        theMbr.ur().y() = geomManage->mbr.ur().y();
    }
    
    // Calculate a center for the tile
    CoordSystem *sceneCoordSys = geomManage->coordAdapter->getCoordSystem();
    const Point3d ll = geomManage->coordAdapter->localToDisplay(sceneCoordSys->geocentricToLocal(geomManage->coordSys->localToGeocentric(Point3d(theMbr.ll().x(),theMbr.ll().y(),0.0))));
    const Point3d ur = geomManage->coordAdapter->localToDisplay(sceneCoordSys->geocentricToLocal(geomManage->coordSys->localToGeocentric(Point3d(theMbr.ur().x(),theMbr.ur().y(),0.0))));
    Point3d dispCenter = (ll+ur)/2.0;
    // This clips the center to something 32 bit floating point can represent.
    const Point3f dispCenter3f(dispCenter.x(),dispCenter.y(),dispCenter.z());
    dispCenter = Point3d(dispCenter3f.x(),dispCenter3f.y(),dispCenter3f.z());

    // Translation for the middle.  The drawable stores floats which isn't high res enough zoomed way in
    const Point3d chunkMidDisp = (geomSettings.useTileCenters ? dispCenter : Point3d(0,0,0));
//        wkLogLevel(Debug,"id = %d: (%d,%d),mid = (%f,%f,%f)",ident.level,ident.x,ident.y,chunkMidDisp.x(),chunkMidDisp.y(),chunkMidDisp.z());
    const Eigen::Affine3d trans(Eigen::Translation3d(chunkMidDisp.x(),chunkMidDisp.y(),chunkMidDisp.z()));
    const Matrix4d transMat = trans.matrix();

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
    
    // Unit size of each tesselation in spherical mercator
    const Point2d incr(chunkSize.x()/sphereTessX,chunkSize.y()/sphereTessY);
    
    // Texture increment for each tesselation
    const TexCoord texIncr(1.0/(float)sphereTessX * texScale.x(),1.0/(float)sphereTessY * texScale.y());
    
    // We need the corners in geographic for the cullable
    const Point2d chunkLL(theMbr.ll().x(),theMbr.ll().y());
    const Point2d chunkUR(theMbr.ur().x(),theMbr.ur().y());
    //    Point2d chunkMid = (chunkLL+chunkUR)/2.0;
    const GeoCoord geoLL(geomManage->coordSys->localToGeographic(Point3d(chunkLL.x(),chunkLL.y(),0.0)));
    const GeoCoord geoUR(geomManage->coordSys->localToGeographic(Point3d(chunkUR.x(),chunkUR.y(),0.0)));
    
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
    drawInfo.push_back(DrawableInfo(DrawableGeom,chunk->getDrawableID(),chunk->getDrawablePriority(),drawOrder));
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
        drawInfo.push_back(DrawableInfo(DrawablePole,poleChunk->getDrawableID(),poleChunk->getDrawablePriority(),drawOrder));
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
                const auto cs = geomManage->coordSys.get();
                const auto org3D = geomManage->coordAdapter->localToDisplay(CoordSystemConvert3d(cs,sceneCoordSys,Point3d(chunkLL.x()+ix*incr.x(),chunkLL.y()+iy*incr.y(),0.0)));
                const auto ptA_3D = geomManage->coordAdapter->localToDisplay(CoordSystemConvert3d(cs,sceneCoordSys,Point3d(chunkLL.x()+(ix+1)*incr.x(),chunkLL.y()+iy*incr.y(),0.0)));
                const auto ptB_3D = geomManage->coordAdapter->localToDisplay(CoordSystemConvert3d(cs,sceneCoordSys,Point3d(chunkLL.x()+ix*incr.x(),chunkLL.y()+(iy+1)*incr.y(),0.0)));
                
                const TexCoord texCoord(ix*texIncr.x(),1.0-(iy*texIncr.y()));
                
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
        std::vector<float> elevs;
        if (geomSettings.includeElev)
            elevs.resize((sphereTessX+1)*(sphereTessY+1));
        std::vector<TexCoord> texCoords((sphereTessX+1)*(sphereTessY+1));
        for (unsigned int iy=0;iy<sphereTessY+1;iy++)
        {
            for (unsigned int ix=0;ix<sphereTessX+1;ix++)
            {
                float locZ = 0.0;
                const auto cs = geomManage->coordSys.get();
                auto loc3D = geomManage->coordAdapter->localToDisplay(CoordSystemConvert3d(cs,sceneCoordSys,Point3d(chunkLL.x()+ix*incr.x(),chunkLL.y()+iy*incr.y(),locZ)));
                if (geomManage->coordAdapter->isFlat())
                    loc3D.z() = locZ;
                
                // Use Z priority to sort the levels
                //                    if (singleLevel != -1)
                //                        loc3D.z() = (drawPriority + nodeInfo->ident.level * 0.01)/10000;
                
                locs[iy*(sphereTessX+1)+ix] = loc3D;
                
                // Do the texture coordinate seperately
                const TexCoord texCoord(ix*texIncr.x(),1.0-(iy*texIncr.y()));
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
            auto skirtChunk = sceneRender->makeBasicDrawableBuilder("LoadedTileNew SkirtChunk");
            drawables.push_back(skirtChunk);
            if (geomSettings.useTileCenters)
                skirtChunk->setMatrix(&transMat);
            // We hardwire this to appear after the atmosphere.  A bit hacky.
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
            drawInfo.push_back(DrawableInfo(DrawableSkirt,skirtChunk->getDrawableID(),skirtChunk->getDrawablePriority(),drawOrder));

            // We'll vary the skirt size a bit.  Otherwise the fill gets ridiculous when we're looking
            //  at the very highest levels.  On the other hand, this doesn't fix a really big large/small
            //  disparity
            const float skirtFactor = 1.0 - 0.2 / (1<<ident.level);
            
            // Bottom skirt
            Point3dVector skirtLocs;
            std::vector<TexCoord> skirtTexCoords;
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
                const int northVert = poleChunk->getNumPoints()-1;
                
                // A line of points for the outer ring, but we can copy them
                const int startOfLine = poleChunk->getNumPoints();
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
                int southVert = poleChunk->getNumPoints()-1;
                
                // A line of points for the outside ring, which we can copy
                const int startOfLine = poleChunk->getNumPoints();
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
    
    for (auto draw : drawables) {
        changes.push_back(new AddDrawableReq(draw->getDrawable()));
    }
}
    
void LoadedTileNew::buildSkirt(BasicDrawableBuilderRef &draw,Point3dVector &pts,std::vector<TexCoord> &texCoords,double skirtFactor,bool haveElev,const Point3d &theCenter)
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
    
void LoadedTileNew::enable(TileGeomSettings &geomSettings,ChangeSet &changes)
{
    if (geomSettings.enableGeom && !enabled)
        for (auto di : drawInfo) {
            changes.push_back(new OnOffChangeRequest(di.drawID,true));
        }
    enabled = true;
}

void LoadedTileNew::disable(TileGeomSettings &geomSettings,ChangeSet &changes)
{
    if (geomSettings.enableGeom && enabled)
        for (auto di : drawInfo) {
            changes.push_back(new OnOffChangeRequest(di.drawID,false));
        }
    enabled = false;
}
    
void LoadedTileNew::removeDrawables(ChangeSet &changes)
{
    for (auto di : drawInfo) {
        changes.push_back(new RemDrawableReq(di.drawID));
    }
}

TileGeomManager::TileGeomManager()
: sceneRender(NULL), quadTree(NULL), coordAdapter(NULL), coverPoles(false),
    useNorthPoleColor(false), northPoleColor(255,255,255,255),
    useSouthPoleColor(false), southPoleColor(255,255,255,255),
    buildSkirts(false)
{
}
    
void TileGeomManager::setup(SceneRenderer *inSceneRender,TileGeomSettings &geomSettings,QuadTreeNew *inQuadTree,CoordSystemDisplayAdapter *inCoordAdapter,CoordSystemRef inCoordSys,MbrD inMbr)
{
    sceneRender = inSceneRender;
    settings = geomSettings;
    quadTree = inQuadTree;
    coordAdapter = inCoordAdapter;
    coordSys = inCoordSys;
    mbr = inMbr;
}
    
TileGeomManager::NodeChanges TileGeomManager::addRemoveTiles(const QuadTreeNew::ImportantNodeSet &addTiles,const QuadTreeNew::NodeSet &removeTiles,ChangeSet &changes)
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
            MbrD mbr = quadTree->generateMbrForNode(ident);
            LoadedTileNewRef tile = std::make_shared<LoadedTileNew>(ident,mbr);
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
    for (auto tileInst: tileMap) {
        auto tile = tileInst.second;
        tile->removeDrawables(changes);
    }
    
    tileMap.clear();
}
    
std::vector<LoadedTileNewRef> TileGeomManager::getTiles(const QuadTreeNew::NodeSet &tiles)
{
    std::vector<LoadedTileNewRef> retTiles;
    
    for (auto ident: tiles) {
        auto it = tileMap.find(ident);
        if (it != tileMap.end()) {
            auto tile = it->second;
            retTiles.push_back(tile);
        }
    }
    
    return retTiles;
}
    
LoadedTileVec TileGeomManager::getAllTiles()
{
    LoadedTileVec retTiles;
    
    for (auto tile: tileMap) {
        retTiles.push_back(tile.second);
    }
    
    return retTiles;
}
    
LoadedTileNewRef TileGeomManager::getTile(const QuadTreeNew::Node &ident)
{
    auto it = tileMap.find(ident);
    if (it != tileMap.end())
        return it->second;
    
    return LoadedTileNewRef();
}
    
void TileGeomManager::updateParents(ChangeSet &changes,LoadedTileVec &enabledNodes,LoadedTileVec &disabledNodes)
{
    // No parent logic with single level.  Everything is on.
    if (settings.singleLevel)
        return;
    
    for (auto entry : tileMap) {
        auto ident = entry.first;
        auto tile = entry.second;
        
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
