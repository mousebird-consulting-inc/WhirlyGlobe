/*
 *  SphericalEarthChunkManager.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/23/13.
 *  Copyright 2011-2019 mousebird consulting
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

#import <queue>
#import <set>
#import "DynamicTextureAtlas.h"
#import "SphericalEarthChunkManager.h"
#import "WhirlyKitLog.h"
#import "SharedAttributes.h"

using namespace Eigen;

namespace WhirlyKit
{
    
SphericalChunkInfo::SphericalChunkInfo()
    : color(RGBAColor(255,255,255,255)), doEdgeMatching(false)
{
}
    
SphericalChunkInfo::SphericalChunkInfo(const Dictionary &dict)
    : BaseInfo(dict), doEdgeMatching(false)
{
    color = dict.getColor(MaplyColor, RGBAColor(255,255,255,255));
    // Note: Should expose edge matching
}

SphericalChunk::SphericalChunk()
    : loadImage(NULL),programID(EmptyIdentity),
    sampleX(12), sampleY(12), minSampleX(1), minSampleY(1), eps(0.0005),
    rotation(0.0), coordSys(NULL)
{
}

static const float SkirtFactor = 0.95;

// Helper routine for constructing the skirt around a tile
void SphericalChunk::buildSkirt(SceneRenderer *sceneRender,BasicDrawableBuilderRef draw,Point3fVector &pts,std::vector<TexCoord> &texCoords,const SphericalChunkInfo &chunkInfo)
{
    for (unsigned int ii=0;ii<pts.size()-1;ii++)
    {
        Point3f corners[4];
        TexCoord cornerTex[4];
        corners[0] = pts[ii];
        cornerTex[0] = texCoords[ii];
        corners[1] = pts[ii+1];
        cornerTex[1] = texCoords[ii+1];
        corners[2] = pts[ii+1] * SkirtFactor;
        cornerTex[2] = texCoords[ii+1];
        corners[3] = pts[ii] * SkirtFactor;
        cornerTex[3] = texCoords[ii];
        
        // Toss in the points, but point the normal up
        int base = draw->getNumPoints();
        for (unsigned int jj=0;jj<4;jj++)
        {
            draw->addPoint(corners[jj]);
            draw->addNormal((Point3f)((pts[ii]+pts[ii+1])/2.f));
            TexCoord texCoord = cornerTex[jj];
            draw->addTexCoord(0,texCoord);
        }
        
        // Add two triangles
        draw->addTriangle(BasicDrawable::Triangle(base+3,base+2,base+0));
        draw->addTriangle(BasicDrawable::Triangle(base+0,base+2,base+1));
    }
}

// Calculate a reasonable sample size based on the four corners passed in
void SphericalChunk::calcSampleX(int &thisSampleX,int &thisSampleY,Point3f *dispPts)
{
    // Default to the sample passed in
    thisSampleX = sampleX;
    thisSampleY = sampleY;
    
    // If there's an epsilon, look at that
    if (eps > 0.0)
    {
        float angBot = acos(dispPts[0].dot(dispPts[1]));
        float angTop = acos(dispPts[3].dot(dispPts[2]));
        float angLeft = acos(dispPts[0].dot(dispPts[3]));
        float angRight = acos(dispPts[1].dot(dispPts[2]));
        float angX = std::max(angBot,angTop);
        float angY = std::max(angLeft,angRight);
        
        float minAng = acosf(1.0-eps) * 2.0;
        if (minAng < angX)
            thisSampleX = angX/minAng;
        if (minAng < angY)
            thisSampleY = angY/minAng;
        thisSampleX = std::max(minSampleX,thisSampleX);
        thisSampleY = std::max(minSampleY,thisSampleY);
        if (sampleX > 0)
            thisSampleX = std::min(thisSampleX,sampleX);
        if (sampleY > 0)
            thisSampleY = std::min(thisSampleY,sampleY);
    }
}

void SphericalChunk::buildDrawable(SceneRenderer *sceneRender,BasicDrawableBuilderRef &draw,bool shouldBuildSkirt,BasicDrawableBuilderRef &skirtDraw,bool enable,CoordSystemDisplayAdapter *coordAdapter,const SphericalChunkInfo &chunkInfo)
{
    CoordSystem *localSys = coordAdapter->getCoordSystem();

    CoordSystemRef srcSystem;
    CoordSystemRef geoSystem(new GeoCoordSystem());
    if (coordSys)
    {
        srcSystem = coordSys;
    } else {
        srcSystem = geoSystem;
    }
    
    BasicDrawableBuilderRef drawable = sceneRender->makeBasicDrawableBuilder("Spherical Earth Chunk");
    chunkInfo.setupBasicDrawable(drawable);
    drawable->setType(Triangles);
//    drawable->setLocalMbr(_mbr);
    drawable->setColor(chunkInfo.color);
    drawable->setTexIDs(texIDs);
    
    int thisSampleX = sampleX, thisSampleY = sampleY;
    
    Mbr localMbr;
    Point3f dispPts[4];
    Point2fVector pts;
    mbr.asPoints(pts);
    std::vector<GeoCoord> geoCoords(4);
    for (unsigned int ii=0;ii<4;ii++)
    {
        geoCoords[ii] = srcSystem->localToGeographic(Point3f(pts[ii].x(),pts[ii].y(),0.0));
        dispPts[ii] = coordAdapter->localToDisplay(localSys->geographicToLocal(geoCoords[ii]));
    }
    
    Point3fVector locs;
    std::vector<TexCoord> texCoords;
    
    Point2f texIncr;
    // Without rotation, we'll just follow the boundaries
    if (rotation == 0.0)
    {
        Point3f srcLL,srcUR;
        srcLL = Point3f(pts[0].x(),pts[0].y(),0.0);
        srcUR = Point3f(pts[2].x(),pts[2].y(),0.0);
        
        // Calculate a reasonable sample size
        calcSampleX(thisSampleX, thisSampleY, dispPts);
        locs.resize((thisSampleX+1)*(thisSampleY+1));
        texCoords.resize((thisSampleX+1)*(thisSampleY+1));
        texIncr = Point2f(1.0/thisSampleX,1.0/thisSampleY);
        
        Point2f localIncr((srcUR.x()-srcLL.x())/thisSampleX,(srcUR.y()-srcLL.y())/thisSampleY);
        
        // Vertices
        for (unsigned int iy=0;iy<thisSampleY+1;iy++)
            for (unsigned int ix=0;ix<thisSampleX+1;ix++)
            {
                Point3d srcLoc(srcLL.x() + ix * localIncr.x(), srcLL.y() + iy * localIncr.y(), 0.0);
                localMbr.addPoint(Point2f(srcLoc.x(),srcLoc.y()));
                Point3d dispLoc = coordAdapter->localToDisplay(CoordSystemConvert3d(srcSystem.get(), localSys, srcLoc));
                Point3f dispLoc3f = Point3f(dispLoc.x(),dispLoc.y(),dispLoc.z());
                
                locs[iy*(thisSampleX+1)+ix] = dispLoc3f;
                TexCoord texCoord(ix * texIncr.x(), 1.0-iy * texIncr.y());
                texCoords[iy*(thisSampleX+1)+ix] = texCoord;
                
                drawable->addPoint(dispLoc3f);
                drawable->addTexCoord(0,texCoord);
                drawable->addNormal(dispLoc3f);
            }
    } else {
        // With rotation, we need to handle this differently
        // Convert the four corners into place
        // Rotate around the center
        Point3f center = (dispPts[0] + dispPts[1] + dispPts[2] + dispPts[3])/4.0;

        Eigen::Matrix4f mat;
        if (coordAdapter->isFlat())
        {
            Eigen::Affine3f trans1(Translation3f(-center.x(),-center.y(),0.0));
            Eigen::Affine3f rot(AngleAxisf(rotation,Point3f(0.0,0.0,1.0)));
            Eigen::Affine3f trans2(Translation3f(center.x(),center.y(),0.0));
            mat = trans2.matrix() * rot.matrix() * trans1.matrix();
        } else {
            Eigen::Affine3f rot(AngleAxisf(rotation,center));
            mat = rot.matrix();
        }
        
        // Rotate the corners
        for (unsigned int ii=0;ii<4;ii++)
        {
            Vector3f &dispPt = dispPts[ii];
            Vector4f dispPost = mat * Vector4f(dispPt.x(),dispPt.y(),dispPt.z(),1.0);
            dispPt = Point3f(dispPost.x(),dispPost.y(),dispPost.z());
        }
        
        // Calculate a reasonable sample size in both directions
        calcSampleX(thisSampleX, thisSampleY, dispPts);
        locs.resize((thisSampleX+1)*(thisSampleY+1));
        texCoords.resize((thisSampleX+1)*(thisSampleY+1));
        texIncr = Point2f(1.0/thisSampleX,1.0/thisSampleY);
        
        Point3f vecA = dispPts[1] - dispPts[0];
        Point3f vecB = dispPts[2] - dispPts[3];
        
        // Vertices
        for (unsigned int iy=0;iy<thisSampleY+1;iy++)
            for (unsigned int ix=0;ix<thisSampleX+1;ix++)
            {
                Point3f ptA = dispPts[0] + ix * vecA / thisSampleX;
                Point3f ptB = dispPts[3] + ix * vecB / thisSampleX;
                Point3f dispLoc = ptA + iy * (ptB-ptA) / thisSampleY;
                Point3f loc = coordAdapter->displayToLocal(dispLoc);
                localMbr.addPoint(Point2f(loc.x(),loc.y()));
                if (!coordAdapter->isFlat())
                    dispLoc.normalize();
                
                locs[iy*(thisSampleX+1)+ix] = dispLoc;
                TexCoord texCoord(ix * texIncr.x(), 1.0-iy * texIncr.y());
                texCoords[iy*(thisSampleX+1)+ix] = texCoord;
                
                drawable->addPoint(dispLoc);
                drawable->addTexCoord(0,texCoord);
                drawable->addNormal(dispLoc);
            }
    }
    drawable->setLocalMbr(localMbr);
    
    // Two triangles per cell
    for (unsigned int iy=0;iy<thisSampleY;iy++)
    {
        for (unsigned int ix=0;ix<thisSampleX;ix++)
        {
            BasicDrawable::Triangle triA,triB;
            triA.verts[0] = (iy+1)*(thisSampleX+1)+ix;
            triA.verts[1] = iy*(thisSampleX+1)+ix;
            triA.verts[2] = (iy+1)*(thisSampleX+1)+(ix+1);
            triB.verts[0] = triA.verts[2];
            triB.verts[1] = triA.verts[1];
            triB.verts[2] = iy*(thisSampleX+1)+(ix+1);
            drawable->addTriangle(triA);
            drawable->addTriangle(triB);
        }
    }
    draw = drawable;
    
    // Build the skirts
    if (shouldBuildSkirt && !coordAdapter->isFlat())
    {
        BasicDrawableBuilderRef skirtDrawable = sceneRender->makeBasicDrawableBuilder("Spherical Earth Chunk Skirts");
        chunkInfo.setupBasicDrawable(skirtDrawable);
        skirtDrawable->setType(Triangles);
        skirtDrawable->setLocalMbr(mbr);
        skirtDrawable->setTexIDs(texIDs);
        
        // Bottom skirt
        Point3fVector skirtLocs;
        std::vector<TexCoord> skirtTexCoords;
        for (unsigned int ix=0;ix<=thisSampleX;ix++)
        {
            skirtLocs.push_back(locs[ix]);
            skirtTexCoords.push_back(texCoords[ix]);
        }
        buildSkirt(sceneRender, skirtDrawable, skirtLocs, skirtTexCoords, chunkInfo);
        // Top skirt
        skirtLocs.clear();
        skirtTexCoords.clear();
        for (int ix=thisSampleX;ix>=0;ix--)
        {
            skirtLocs.push_back(locs[(thisSampleY)*(thisSampleX+1)+ix]);
            skirtTexCoords.push_back(texCoords[(thisSampleY)*(thisSampleX+1)+ix]);
        }
        buildSkirt(sceneRender, skirtDrawable,skirtLocs,skirtTexCoords, chunkInfo);
        // Left skirt
        skirtLocs.clear();
        skirtTexCoords.clear();
        for (int iy=thisSampleY;iy>=0;iy--)
        {
            skirtLocs.push_back(locs[(thisSampleX+1)*iy+0]);
            skirtTexCoords.push_back(texCoords[(thisSampleX+1)*iy+0]);
        }
        buildSkirt(sceneRender, skirtDrawable,skirtLocs,skirtTexCoords, chunkInfo);
        // right skirt
        skirtLocs.clear();
        skirtTexCoords.clear();
        for (int iy=0;iy<=thisSampleY;iy++)
        {
            skirtLocs.push_back(locs[(thisSampleX+1)*iy+(thisSampleX)]);
            skirtTexCoords.push_back(texCoords[(thisSampleX+1)*iy+(thisSampleX)]);
        }
        buildSkirt(sceneRender, skirtDrawable,skirtLocs,skirtTexCoords, chunkInfo);
        
        skirtDraw = skirtDrawable;
    }
}
    
SphericalChunkManager::SphericalChunkManager()
    : borderTexel(0)
{
}
    
SphericalChunkManager::~SphericalChunkManager()
{
}
    
/// Add the given chunk (enabled or disabled)
SimpleIdentity SphericalChunkManager::addChunk(SphericalChunk *chunk,const SphericalChunkInfo &chunkInfo,ChangeSet &changes)
{
    ChunkRequest request(ChunkAdd,chunkInfo,chunk);
    request.doEdgeMatching = chunkInfo.doEdgeMatching;
    // If it needs the altases, just queue it up
    if (chunk->loadImage)
    {
        std::lock_guard<std::mutex> guardLock(requestLock);
        requests.push(request);
    } else {
        // If it doesn't, just run it right now
        processChunkRequest(request,changes);
    }
    
    // Get rid of an unnecessary circularity
//    chunkInfo->chunk = nil;
    
    // And possibly run the outstanding request queue
//    processRequests(changes);
    
    return chunk->getId();
}
    
bool SphericalChunkManager::modifyChunkTextures(SimpleIdentity chunkID,const std::vector<SimpleIdentity> &texIDs,ChangeSet &changes)
{
    SimpleIDSet drawIDs;
    SimpleIDSet oldTexIDs;
    
    {
        std::lock_guard<std::mutex> guardLock(repLock);
        ChunkSceneRepRef dummyRef(new ChunkSceneRep(chunkID));
        ChunkRepSet::iterator it = chunkReps.find(dummyRef);
        if (it != chunkReps.end())
        {
            drawIDs = (*it)->drawIDs;
            oldTexIDs = (*it)->texIDs;
        }
    }

    // Make sure we have the same number of textures
    //    if (oldTexIDs.size() != texIDs.size())
    //        return false;
    
    for (SimpleIDSet::iterator it = drawIDs.begin(); it != drawIDs.end(); ++it)
        changes.push_back(new DrawTexturesChangeRequest(*it,texIDs));
    
    return true;
}
    
bool SphericalChunkManager::modifyDrawPriority(SimpleIdentity chunkID,int drawPriority,ChangeSet &changes)
{
    SimpleIDSet drawIDs;

    {
        std::lock_guard<std::mutex> guardLock(repLock);
        ChunkSceneRepRef dummyRef(new ChunkSceneRep(chunkID));
        ChunkRepSet::iterator it = chunkReps.find(dummyRef);
        if (it != chunkReps.end())
        {
            drawIDs = (*it)->drawIDs;
        }
    }
    
    for (SimpleIDSet::iterator it = drawIDs.begin(); it != drawIDs.end(); ++it)
        changes.push_back(new DrawPriorityChangeRequest(*it,drawPriority));
    
    return true;
}

/// Enable or disable the given chunk
void SphericalChunkManager::enableChunk(SimpleIdentity chunkID,bool enable,ChangeSet &changes)
{
    SphericalChunkInfo chunkInfo;
    
    {
        std::lock_guard<std::mutex> guardLock(requestLock);
        if (enable)
            requests.push(ChunkRequest(ChunkEnable,chunkInfo,chunkID));
        else
            requests.push(ChunkRequest(ChunkDisable,chunkInfo,chunkID));
    }
    
    processRequests(changes);
}

/// Remove the given chunks
void SphericalChunkManager::removeChunks(SimpleIDSet &chunkIDs,ChangeSet &changes)
{
    SphericalChunkInfo chunkInfo;

    {
        std::lock_guard<std::mutex> guardLock(requestLock);
        for (SimpleIDSet::iterator it = chunkIDs.begin(); it != chunkIDs.end(); ++it)
            requests.push(ChunkRequest(ChunkRemove,chunkInfo,*it));
    }
    
    processRequests(changes);
}

int SphericalChunkManager::getNumChunks()
{
    std::lock_guard<std::mutex> guardLock(repLock);

    int numChunks = 0;
    numChunks = chunkReps.size();
    
    return chunkReps.size();
}
    
/// Process outstanding requests
void SphericalChunkManager::processRequests(ChangeSet &changes)
{
    // Process the changes and then flush them out
    while (!requests.empty())
    {
        std::lock_guard<std::mutex> guardLock(requestLock);
        ChunkRequest request = requests.front();
        requests.pop();
    }
}
    
// Process a single chunk request (add, enable, disable)
void SphericalChunkManager::processChunkRequest(ChunkRequest &request,ChangeSet &changes)
{
    switch (request.type)
    {
        case ChunkAdd:
        {
            CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
            ChunkSceneRepRef chunkRep(new ChunkSceneRep(request.chunkId));
            SphericalChunk *chunk = request.chunk;
            
            // May need to set up the texture
            SimpleIdentity texId = EmptyIdentity;
            Texture *newTex = NULL;
            if (chunk->loadImage)
            {
                // Let's just deal with square images
                newTex = chunk->loadImage->buildTexture();
            }
            if (newTex)
            {
                chunk->texIDs.push_back(newTex->getId());
                chunkRep->texIDs.insert(newTex->getId());
                changes.push_back(new AddTextureReq(newTex));
            }
            texId = EmptyIdentity;
            if (!chunk->texIDs.empty())
                texId = chunk->texIDs.at(0);
            
            // Build the main drawable and possibly skirt
            BasicDrawableBuilderRef drawable,skirtDraw;
            chunk->buildDrawable(renderer,drawable,request.doEdgeMatching,skirtDraw,request.chunkInfo.enable,coordAdapter,request.chunkInfo);
            
            if (skirtDraw)
            {
                chunkRep->drawIDs.insert(skirtDraw->getDrawableID());
                skirtDraw->setTexId(0,texId);
                changes.push_back(new AddDrawableReq(skirtDraw->getDrawable()));
            }
            
            chunkRep->drawIDs.insert(drawable->getDrawableID());
            drawable->setTexId(0,texId);
            changes.push_back(new AddDrawableReq(drawable->getDrawable()));
            
            {
                std::lock_guard<std::mutex> guardLock(repLock);
                chunkReps.insert(chunkRep);
            }
        }
            break;
        case ChunkEnable:
        {
            std::lock_guard<std::mutex> guardLock(repLock);
            ChunkSceneRepRef dummyRef(new ChunkSceneRep(request.chunkId));
            ChunkRepSet::iterator it = chunkReps.find(dummyRef);
            if (it != chunkReps.end())
                (*it)->enable(changes);
        }
            break;
        case ChunkDisable:
        {
            std::lock_guard<std::mutex> guardLock(repLock);
            ChunkSceneRepRef dummyRef(new ChunkSceneRep(request.chunkId));
            ChunkRepSet::iterator it = chunkReps.find(dummyRef);
            if (it != chunkReps.end())
                (*it)->disable(changes);
        }
            break;
        case ChunkRemove:
        {
            std::lock_guard<std::mutex> guardLock(repLock);
            ChunkSceneRepRef dummyRef(new ChunkSceneRep(request.chunkId));
            ChunkRepSet::iterator it = chunkReps.find(dummyRef);
            if (it != chunkReps.end())
	      {
                (*it)->clear(scene,changes);
              chunkReps.erase(it);
	      }
        }
            break;
    }
}


}
