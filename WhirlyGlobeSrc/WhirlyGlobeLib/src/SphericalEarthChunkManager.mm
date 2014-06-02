/*
 *  SphericalEarthChunkManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/23/13.
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

#import <queue>
#import <set>
#import "DynamicTextureAtlas.h"
#import "DynamicDrawableAtlas.h"
#import "SphericalEarthChunkManager.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation WhirlyKitSphericalChunk

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    // Adaptive sample is the default
    _sampleX = _sampleY = 12;
    _minSampleX = _minSampleY = 1;
    _eps = 0.0005;
    _minVis = DrawVisibleInvalid;
    _maxVis = DrawVisibleInvalid;
    _readZBuffer = false;
    _writeZBuffer = true;
    
    return self;
}

static const float SkirtFactor = 0.95;

// Helper routine for constructing the skirt around a tile
- (void)buildSkirt:(BasicDrawable *)draw pts:(std::vector<Point3f> &)pts tex:(std::vector<TexCoord> &)texCoords
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
            draw->addNormal((pts[ii]+pts[ii+1])/2.0);
            TexCoord texCoord = cornerTex[jj];
            draw->addTexCoord(0,texCoord);
        }
        
        // Add two triangles
        draw->addTriangle(BasicDrawable::Triangle(base+3,base+2,base+0));
        draw->addTriangle(BasicDrawable::Triangle(base+0,base+2,base+1));
    }
}

// Calculate a reasonable sample size based on the four corners passed in
- (void)calcSampleX:(int &)thisSampleX sampleY:(int &)thisSampleY fromPoints:(Point3f *)dispPts
{
    // Default to the sample passed in
    thisSampleX = _sampleX;
    thisSampleY = _sampleY;
    
    // If there's an epsilon, look at that
    if (_eps > 0.0)
    {
        float angBot = acos(dispPts[0].dot(dispPts[1]));
        float angTop = acos(dispPts[3].dot(dispPts[2]));
        float angLeft = acos(dispPts[0].dot(dispPts[3]));
        float angRight = acos(dispPts[1].dot(dispPts[2]));
        float angX = std::max(angBot,angTop);
        float angY = std::max(angLeft,angRight);
        
        float minAng = acosf(1.0-_eps) * 2.0;
        if (minAng < angX)
            thisSampleX = angX/minAng;
        if (minAng < angY)
            thisSampleY = angY/minAng;
        thisSampleX = std::max(_minSampleX,thisSampleX);
        thisSampleY = std::max(_minSampleY,thisSampleY);
        if (_sampleX > 0)
            thisSampleX = std::min(thisSampleX,_sampleX);
        if (_sampleY > 0)
            thisSampleY = std::min(thisSampleY,_sampleY);
    }
    
    // Note: Debugging
    //    NSLog(@"Sampling: (%d,%d)",thisSampleX,thisSampleY);
}

- (void)buildDrawable:(BasicDrawable **)draw skirtDraw:(BasicDrawable **)skirtDraw enabled:(bool)enable adapter:(CoordSystemDisplayAdapter *)coordAdapter
{
    CoordSystem *localSys = coordAdapter->getCoordSystem();

    CoordSystem *srcSystem = nil;
    GeoCoordSystem geoSystem;
    if (_coordSys)
    {
        srcSystem = _coordSys;
    } else {
        srcSystem = &geoSystem;
    }
    
    BasicDrawable *drawable = new BasicDrawable("Spherical Earth Chunk");
    drawable->setType(GL_TRIANGLES);
//    drawable->setLocalMbr(_mbr);
    drawable->setDrawPriority(_drawPriority);
    drawable->setDrawOffset(_drawOffset);
    drawable->setTexIDs(_texIDs);
    drawable->setOnOff(enable);
    drawable->setVisibleRange(_minVis, _maxVis, _minVisBand, _maxVisBand);
    drawable->setRequestZBuffer(self.readZBuffer);
    drawable->setWriteZBuffer(self.writeZBuffer);
    drawable->setProgram(_programID);
    
    int thisSampleX = _sampleX, thisSampleY = _sampleY;
    
    Mbr localMbr;
    Point3f dispPts[4];
    std::vector<Point2f> pts;
    _mbr.asPoints(pts);
    std::vector<GeoCoord> geoCoords(4);
    for (unsigned int ii=0;ii<4;ii++)
    {
        geoCoords[ii] = srcSystem->localToGeographic(Point3f(pts[ii].x(),pts[ii].y(),0.0));
        dispPts[ii] = coordAdapter->localToDisplay(localSys->geographicToLocal(geoCoords[ii]));
    }
    
    std::vector<Point3f> locs;
    std::vector<TexCoord> texCoords;
    
    Point2f texIncr;
    // Without rotation, we'll just follow the boundaries
    if (_rotation == 0.0)
    {
        Point3f srcLL,srcUR;
        srcLL = Point3f(pts[0].x(),pts[0].y(),0.0);
        srcUR = Point3f(pts[2].x(),pts[2].y(),0.0);
        
        // Calculate a reasonable sample size
        [self calcSampleX:thisSampleX sampleY:thisSampleY fromPoints:dispPts];
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
                Point3d dispLoc = coordAdapter->localToDisplay(CoordSystemConvert3d(srcSystem, localSys, srcLoc));
                Point3f dispLoc3f = Point3f(dispLoc.x(),dispLoc.y(),dispLoc.z());
                
                locs[iy*(thisSampleX+1)+ix] = dispLoc3f;
                TexCoord texCoord(ix * texIncr.x(), 1.0-iy * texIncr.y());
                texCoords[iy*(thisSampleX+1)+ix] = texCoord;
                
                drawable->addPoint(dispLoc3f);
                drawable->addTexCoord(0,texCoord);
                drawable->addNormal(dispLoc3f);
            }
    } else {
        // Note: Not sure this works with specific coordinate systems
        // With rotation, we need to handle this differently
        // Convert the four corners into place
        // Rotate around the center
        Point3f center = (dispPts[0] + dispPts[1] + dispPts[2] + dispPts[3])/4.0;
        Eigen::Affine3f rot(AngleAxisf(_rotation,center));
        Eigen::Matrix4f mat = rot.matrix();
        
        // Rotate the corners
        for (unsigned int ii=0;ii<4;ii++)
        {
            Vector3f &dispPt = dispPts[ii];
            Vector4f dispPost = mat * Vector4f(dispPt.x(),dispPt.y(),dispPt.z(),1.0);
            dispPt = Point3f(dispPost.x(),dispPost.y(),dispPost.z());
        }
        
        // Calculate a reasonable sample size in both directions
        [self calcSampleX:thisSampleX sampleY:thisSampleY fromPoints:dispPts];
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
    *draw = drawable;
    
    // Build the skirts
    if (skirtDraw && !coordAdapter->isFlat())
    {
        BasicDrawable *skirtDrawable = new BasicDrawable("Spherical Earth Chunk Skirts");
        skirtDrawable->setType(GL_TRIANGLES);
        skirtDrawable->setLocalMbr(_mbr);
        skirtDrawable->setDrawPriority(0);
        skirtDrawable->setDrawOffset(_drawOffset);
        skirtDrawable->setTexIDs(_texIDs);
        skirtDrawable->setOnOff(enable);
        skirtDrawable->setVisibleRange(_minVis, _maxVis);
        skirtDrawable->setRequestZBuffer(true);
        skirtDrawable->setWriteZBuffer(false);
        skirtDrawable->setProgram(_programID);
        
        // Bottom skirt
        std::vector<Point3f> skirtLocs;
        std::vector<TexCoord> skirtTexCoords;
        for (unsigned int ix=0;ix<=thisSampleX;ix++)
        {
            skirtLocs.push_back(locs[ix]);
            skirtTexCoords.push_back(texCoords[ix]);
        }
        [self buildSkirt:skirtDrawable pts:skirtLocs tex:skirtTexCoords];
        // Top skirt
        skirtLocs.clear();
        skirtTexCoords.clear();
        for (int ix=thisSampleX;ix>=0;ix--)
        {
            skirtLocs.push_back(locs[(thisSampleY)*(thisSampleX+1)+ix]);
            skirtTexCoords.push_back(texCoords[(thisSampleY)*(thisSampleX+1)+ix]);
        }
        [self buildSkirt:skirtDrawable pts:skirtLocs tex:skirtTexCoords];
        // Left skirt
        skirtLocs.clear();
        skirtTexCoords.clear();
        for (int iy=thisSampleY;iy>=0;iy--)
        {
            skirtLocs.push_back(locs[(thisSampleX+1)*iy+0]);
            skirtTexCoords.push_back(texCoords[(thisSampleX+1)*iy+0]);
        }
        [self buildSkirt:skirtDrawable pts:skirtLocs tex:skirtTexCoords];
        // right skirt
        skirtLocs.clear();
        skirtTexCoords.clear();
        for (int iy=0;iy<=thisSampleY;iy++)
        {
            skirtLocs.push_back(locs[(thisSampleX+1)*iy+(thisSampleX)]);
            skirtTexCoords.push_back(texCoords[(thisSampleX+1)*iy+(thisSampleX)]);
        }
        [self buildSkirt:skirtDrawable pts:skirtLocs tex:skirtTexCoords];
        
        *skirtDraw = skirtDrawable;
    }
}

@end

@implementation WhirlyKitSphericalChunkInfo

@end

namespace WhirlyKit
{
     
// Used to track scene data associated with a chunk
class ChunkSceneRep : public Identifiable
{
public:
    ChunkSceneRep(SimpleIdentity theId) : Identifiable(theId) { }
    ChunkSceneRep() : usesAtlas(false) { subTex.texId = EmptyIdentity; }
    SimpleIDSet drawIDs;
    SimpleIDSet texIDs;
    bool usesAtlas;
    // Set if we're using a dynamic texture atlas
    SubTexture subTex;
    
    // Remove elements from the scene
    void clear(Scene *scene,DynamicTextureAtlas *texAtlas,DynamicDrawableAtlas *drawAtlas,ChangeSet &changeRequests)
    {
        if (usesAtlas && drawAtlas)
        {
            for (SimpleIDSet::iterator it = drawIDs.begin();
                 it != drawIDs.end(); ++it)
                drawAtlas->removeDrawable(*it, changeRequests);
        } else {
            for (SimpleIDSet::iterator it = drawIDs.begin();
                 it != drawIDs.end(); ++it)
                changeRequests.push_back(new RemDrawableReq(*it));
        }
        if (usesAtlas && texAtlas)
        {
            if (subTex.texId != EmptyIdentity)
                texAtlas->removeTexture(subTex, changeRequests);
        } else {
            for (SimpleIDSet::iterator it = texIDs.begin();
                 it != texIDs.end(); ++it)
                changeRequests.push_back(new RemTextureReq(*it));
        }
    }
    
    // Enable drawables
    void enable(DynamicTextureAtlas *texAtlas,DynamicDrawableAtlas *drawAtlas,ChangeSet &changes)
    {
        if (usesAtlas && drawAtlas)
        {
            for (SimpleIDSet::iterator it = drawIDs.begin();
                 it != drawIDs.end(); ++it)
                drawAtlas->setEnableDrawable(*it, true);
        } else {
            for (SimpleIDSet::iterator it = drawIDs.begin();
                 it != drawIDs.end(); ++it)
                changes.push_back(new OnOffChangeRequest(*it, true));
        }
    }
    
    // Disable drawables
    void disable(DynamicTextureAtlas *texAtlas,DynamicDrawableAtlas *drawAtlas,ChangeSet &changes)
    {
        if (usesAtlas && drawAtlas)
        {
            for (SimpleIDSet::iterator it = drawIDs.begin();
                 it != drawIDs.end(); ++it)
                drawAtlas->setEnableDrawable(*it, false);
        } else {
            for (SimpleIDSet::iterator it = drawIDs.begin();
                 it != drawIDs.end(); ++it)
                changes.push_back(new OnOffChangeRequest(*it, false));
        }
    }
};
    
SphericalChunkManager::SphericalChunkManager()
    : borderTexel(0), texAtlas(NULL), drawAtlas(NULL)
{
    pthread_mutex_init(&repLock,NULL);
    pthread_mutex_init(&requestLock,NULL);
    pthread_mutex_init(&atlasLock,NULL);
}
    
SphericalChunkManager::~SphericalChunkManager()
{
    pthread_mutex_destroy(&repLock);
    pthread_mutex_destroy(&requestLock);
    pthread_mutex_destroy(&atlasLock);
}
    
/// Add the given chunk (enabled or disabled)
SimpleIdentity SphericalChunkManager::addChunk(WhirlyKitSphericalChunk *chunk,bool doEdgeMatching,bool enable,ChangeSet &changes)
{
    SimpleIdentity chunkID = EmptyIdentity;
    WhirlyKitSphericalChunkInfo *chunkInfo = [[WhirlyKitSphericalChunkInfo alloc] init];
    chunkInfo->enable = enable;
    chunkInfo->chunk = chunk;
    chunkInfo->chunkId = Identifiable::genId();
    chunkID = chunkInfo->chunkId;

    ChunkRequest request(ChunkAdd,chunkInfo,chunkInfo->chunk);
    request.doEdgeMatching = doEdgeMatching;
    // If it needs the altases, just queue it up
    if (chunkInfo->chunk.loadImage)
    {
        pthread_mutex_lock(&requestLock);
        requests.push(request);
        pthread_mutex_unlock(&requestLock);
    } else {
        // If it doesn't, just run it right now
        processChunkRequest(request,changes);
    }
    
    // Get rid of an unnecessary circularity
//    chunkInfo->chunk = nil;
    
    // And possibly run the outstanding request queue
//    processRequests(changes);
    
    return chunkID;
}
    
bool SphericalChunkManager::modifyChunkTextures(SimpleIdentity chunkID,const std::vector<SimpleIdentity> &texIDs,ChangeSet &changes)
{
    SimpleIDSet drawIDs;
//    SimpleIDSet oldTexIDs;
    
    pthread_mutex_lock(&repLock);
    ChunkSceneRepRef dummyRef(new ChunkSceneRep(chunkID));
    ChunkRepSet::iterator it = chunkReps.find(dummyRef);
    if (it != chunkReps.end())
    {
        drawIDs = (*it)->drawIDs;
//        oldTexIDs = (*it)->texIDs;
    }
    pthread_mutex_unlock(&repLock);

    // Make sure we have the same number of textures
//    if (oldTexIDs.size() != texIDs.size())
//        return false;
    
    for (SimpleIDSet::iterator it = drawIDs.begin(); it != drawIDs.end(); ++it)
        changes.push_back(new DrawTexturesChangeRequest(*it,texIDs));
    
    return true;
}

/// Enable or disable the given chunk
void SphericalChunkManager::enableChunk(SimpleIdentity chunkID,bool enable,ChangeSet &changes)
{
    pthread_mutex_lock(&requestLock);
    if (enable)
        requests.push(ChunkRequest(ChunkEnable,chunkID));
    else
        requests.push(ChunkRequest(ChunkDisable,chunkID));
    pthread_mutex_unlock(&requestLock);
    
    if (!texAtlas)
        processRequests(changes);
}

/// Remove the given chunks
void SphericalChunkManager::removeChunks(SimpleIDSet &chunkIDs,ChangeSet &changes)
{
    pthread_mutex_lock(&requestLock);
    for (SimpleIDSet::iterator it = chunkIDs.begin(); it != chunkIDs.end(); ++it)
        requests.push(ChunkRequest(ChunkRemove,*it));
    pthread_mutex_unlock(&requestLock);
    
    if (!texAtlas)
        processRequests(changes);
}

int SphericalChunkManager::getNumChunks()
{
    int numChunks = 0;
    pthread_mutex_lock(&repLock);
    numChunks = chunkReps.size();
    pthread_mutex_unlock(&repLock);
    
    return chunkReps.size();
}
    
/// Process outstanding requests
void SphericalChunkManager::processRequests(ChangeSet &changes)
{
    // Process the changes and then flush them out
    while (!requests.empty())
    {
        pthread_mutex_lock(&requestLock);
        ChunkRequest request = requests.front();
        requests.pop();
        pthread_mutex_unlock(&requestLock);
        processChunkRequest(request,changes);
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
            ChunkSceneRepRef chunkRep(new ChunkSceneRep(request.chunkInfo->chunkId));
            WhirlyKitSphericalChunk *chunk = request.chunk;
            
            // May need to set up the texture
            SimpleIdentity texId = EmptyIdentity;
            chunkRep->usesAtlas = false;
            Texture *newTex = NULL;
            if (chunk.loadImage)
            {
                // Let's just deal with square images
                int square = std::max(chunk.loadImage.width,chunk.loadImage.height);
                newTex = [chunk.loadImage buildTexture:borderTexel destWidth:square destHeight:square];
            }
            if (texAtlas)
            {
                if (newTex)
                {
                    pthread_mutex_lock(&atlasLock);
                    std::vector<Texture *> newTexs;
                    texAtlas->addTexture(newTexs, NULL, NULL, chunkRep->subTex, scene->getMemManager(), changes, borderTexel);
                    chunkRep->usesAtlas = true;
                    delete newTex;
                    pthread_mutex_unlock(&atlasLock);
                }
            } else {
                if (newTex)
                {
                    chunk.texIDs.push_back(newTex->getId());
                    chunkRep->texIDs.insert(newTex->getId());
                    changes.push_back(new AddTextureReq(newTex));
                }
                texId = EmptyIdentity;
                if (!chunk.texIDs.empty())
                    texId = chunk.texIDs.at(0);
            }
            
            // Build the main drawable and possibly skirt
            BasicDrawable *drawable = nil,*skirtDraw = nil;
            [chunk buildDrawable:&drawable skirtDraw:(request.doEdgeMatching ? &skirtDraw : nil) enabled:request.chunkInfo->enable adapter:coordAdapter];
            
            // Note: Debugging
            //            int color = drand48()*50+205;
            //            RGBAColor randomColor(color/4,color/4,color/4,255/4);
            //            drawable->setColor(randomColor);
            
            if (skirtDraw)
            {
                chunkRep->drawIDs.insert(skirtDraw->getId());
                if (chunkRep->usesAtlas && drawAtlas)
                {
                    pthread_mutex_lock(&atlasLock);
                    skirtDraw->applySubTexture(0,chunkRep->subTex);
                    drawAtlas->addDrawable(skirtDraw, changes);
                    delete skirtDraw;
                    pthread_mutex_unlock(&atlasLock);
                } else {
                    if (texAtlas)
                        skirtDraw->applySubTexture(0,chunkRep->subTex);
                    else
                        skirtDraw->setTexId(0,texId);
                        changes.push_back(new AddDrawableReq(skirtDraw));
                }
            }
            
            chunkRep->drawIDs.insert(drawable->getId());
            if (chunkRep->usesAtlas && drawAtlas)
            {
                pthread_mutex_lock(&atlasLock);
                drawable->applySubTexture(0,chunkRep->subTex);
                drawAtlas->addDrawable(drawable, changes);
                delete drawable;
                pthread_mutex_unlock(&atlasLock);
            } else {
                if (texAtlas)
                    drawable->applySubTexture(0,chunkRep->subTex);
                else
                    drawable->setTexId(0,texId);
                changes.push_back(new AddDrawableReq(drawable));
            }
            
            pthread_mutex_lock(&repLock);
            chunkReps.insert(chunkRep);
            pthread_mutex_unlock(&repLock);
        }
            break;
        case ChunkEnable:
        {
            pthread_mutex_lock(&repLock);
            ChunkSceneRepRef dummyRef(new ChunkSceneRep(request.chunkId));
            ChunkRepSet::iterator it = chunkReps.find(dummyRef);
            if (it != chunkReps.end())
                (*it)->enable(texAtlas,drawAtlas,changes);
            pthread_mutex_unlock(&repLock);
        }
            break;
        case ChunkDisable:
        {
            pthread_mutex_lock(&repLock);
            ChunkSceneRepRef dummyRef(new ChunkSceneRep(request.chunkId));
            ChunkRepSet::iterator it = chunkReps.find(dummyRef);
            if (it != chunkReps.end())
                (*it)->disable(texAtlas,drawAtlas,changes);
            pthread_mutex_unlock(&repLock);
        }
            break;
        case ChunkRemove:
        {
            pthread_mutex_lock(&repLock);
            ChunkSceneRepRef dummyRef(new ChunkSceneRep(request.chunkId));
            ChunkRepSet::iterator it = chunkReps.find(dummyRef);
            if (it != chunkReps.end())
                (*it)->clear(scene,texAtlas,drawAtlas,changes);
            chunkReps.erase(it);
            pthread_mutex_unlock(&repLock);
        }
            break;
    }
}


}
