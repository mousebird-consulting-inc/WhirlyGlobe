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

@synthesize mbr;
@synthesize texId;
@synthesize loadImage;
@synthesize drawOffset;
@synthesize drawPriority;
@synthesize sampleX,sampleY;
@synthesize minSampleX,minSampleY;
@synthesize eps;
@synthesize minVis,maxVis;
@synthesize minVisBand,maxVisBand;
@synthesize rotation;
@synthesize readZBuffer;
@synthesize writeZBuffer;

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    // Adaptive sample is the default
    sampleX = sampleY = 12;
    minSampleX = minSampleY = 1;
    eps = 0.0005;
    minVis = DrawVisibleInvalid;
    maxVis = DrawVisibleInvalid;
    readZBuffer = false;
    writeZBuffer = true;
    
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
            draw->addTexCoord(texCoord);
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
    
    // Note: Debugging
    //    NSLog(@"Sampling: (%d,%d)",thisSampleX,thisSampleY);
}

- (void)buildDrawable:(BasicDrawable **)draw skirtDraw:(BasicDrawable **)skirtDraw enabled:(bool)enable adapter:(CoordSystemDisplayAdapter *)coordAdapter
{
    CoordSystem *localSys = coordAdapter->getCoordSystem();
    
    BasicDrawable *drawable = new BasicDrawable("Spherical Earth Chunk");
    drawable->setType(GL_TRIANGLES);
    drawable->setLocalMbr(mbr);
    drawable->setDrawPriority(drawPriority);
    drawable->setDrawOffset(drawOffset);
    drawable->setTexId(texId);
    drawable->setOnOff(enable);
    drawable->setVisibleRange(minVis, maxVis, minVisBand, maxVisBand);
    drawable->setRequestZBuffer(self.readZBuffer);
    drawable->setWriteZBuffer(self.writeZBuffer);
    
    int thisSampleX = sampleX, thisSampleY = sampleY;
    
    Point3f dispPts[4];
    dispPts[0] = coordAdapter->localToDisplay(localSys->geographicToLocal(mbr.ll()));
    dispPts[1] = coordAdapter->localToDisplay(localSys->geographicToLocal(mbr.lr()));
    dispPts[2] = coordAdapter->localToDisplay(localSys->geographicToLocal(mbr.ur()));
    dispPts[3] = coordAdapter->localToDisplay(localSys->geographicToLocal(mbr.ul()));
    
    std::vector<Point3f> locs;
    std::vector<TexCoord> texCoords;
    
    Point2f texIncr;
    // Without rotation, we'll just follow the geographic boundaries
    if (rotation == 0.0)
    {
        Point3f localLL,localUR;
        localLL = localSys->geographicToLocal(mbr.ll());
        localUR = localSys->geographicToLocal(mbr.ur());
        
        // Calculate a reasonable sample size
        [self calcSampleX:thisSampleX sampleY:thisSampleY fromPoints:dispPts];
        locs.resize((thisSampleX+1)*(thisSampleY+1));
        texCoords.resize((thisSampleX+1)*(thisSampleY+1));
        texIncr = Point2f(1.0/thisSampleX,1.0/thisSampleY);
        
        Point2f localIncr((localUR.x()-localLL.x())/thisSampleX,(localUR.y()-localLL.y())/thisSampleY);
        
        // Vertices
        for (unsigned int iy=0;iy<thisSampleY+1;iy++)
            for (unsigned int ix=0;ix<thisSampleX+1;ix++)
            {
                Point3f loc(localLL.x() + ix * localIncr.x(), localLL.y() + iy * localIncr.y(), 0.0);
                Point3f dispLoc = coordAdapter->localToDisplay(loc);
                
                locs[iy*(thisSampleX+1)+ix] = dispLoc;
                TexCoord texCoord(ix * texIncr.x(), 1.0-iy * texIncr.y());
                texCoords[iy*(thisSampleX+1)+ix] = texCoord;
                
                drawable->addPoint(dispLoc);
                drawable->addTexCoord(texCoord);
                drawable->addNormal(dispLoc);
            }
    } else {
        // With rotation, we need to handle this differently
        // Convert the four corners into place
        // Rotate around the center
        Point3f center = (dispPts[0] + dispPts[1] + dispPts[2] + dispPts[3])/4.0;
        Eigen::Affine3f rot(AngleAxisf(rotation,center));
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
                if (!coordAdapter->isFlat())
                    dispLoc.normalize();
                
                locs[iy*(thisSampleX+1)+ix] = dispLoc;
                TexCoord texCoord(ix * texIncr.x(), 1.0-iy * texIncr.y());
                texCoords[iy*(thisSampleX+1)+ix] = texCoord;
                
                drawable->addPoint(dispLoc);
                drawable->addTexCoord(texCoord);
                drawable->addNormal(dispLoc);
            }
    }
    
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
        skirtDrawable->setLocalMbr(mbr);
        skirtDrawable->setDrawPriority(0);
        skirtDrawable->setDrawOffset(drawOffset);
        skirtDrawable->setTexId(texId);
        skirtDrawable->setOnOff(enable);
        skirtDrawable->setVisibleRange(minVis, maxVis);
        skirtDrawable->setRequestZBuffer(true);
        skirtDrawable->setWriteZBuffer(false);
        
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
    pthread_mutex_init(&chunkLock,NULL);
}
    
SphericalChunkManager::~SphericalChunkManager()
{
    pthread_mutex_destroy(&chunkLock);    
}
    
/// Add the given chunk (enabled or disabled)
SimpleIdentity SphericalChunkManager::addChunk(WhirlyKitSphericalChunk *chunk,bool doEdgeMatching,bool enable,ChangeSet &changes)
{
    WhirlyKitSphericalChunkInfo *chunkInfo = [[WhirlyKitSphericalChunkInfo alloc] init];
    chunkInfo->enable = enable;
    chunkInfo->chunk = chunk;
    chunkInfo->chunkId = Identifiable::genId();

    ChunkRequest request(ChunkAdd,chunkInfo,chunkInfo->chunk);
    request.doEdgeMatching = doEdgeMatching;
    // If it needs the altases, just queue it up
    if (chunkInfo->chunk.loadImage)
        requests.push(request);
    else {
        // If it doesn't, just run it right now
        processChunkRequest(request,changes);
    }
    
    // Get rid of an unnecessary circularity
    chunkInfo->chunk = nil;
    
    // And possibly run the outstanding request queue
    processRequests(changes);
    
    return chunkInfo->chunkId;
}

/// Enable or disable the given chunk
void SphericalChunkManager::enableChunk(SimpleIdentity chunkID,bool enable,ChangeSet &changes)
{
    if (enable)
        requests.push(ChunkRequest(ChunkEnable,chunkID));
    else
        requests.push(ChunkRequest(ChunkDisable,chunkID));
    
    processRequests(changes);
}

/// Remove the given chunks
void SphericalChunkManager::removeChunks(SimpleIDSet &chunkIDs,ChangeSet &changes)
{
    for (SimpleIDSet::iterator it = chunkIDs.begin(); it != chunkIDs.end(); ++it)
        requests.push(ChunkRequest(ChunkRemove,*it));
    
    processRequests(changes);
}

int SphericalChunkManager::getNumChunks()
{
    return chunkReps.size();
}
    
/// Process outstanding requests
void SphericalChunkManager::processRequests(ChangeSet &changes)
{
    // Process the changes and then flush them out
    while (!requests.empty())
    {
        ChunkRequest request = requests.front();
        requests.pop();
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
            
            GeoMbr geoMbr = chunk.mbr;
            
            // May need to set up the texture
            SimpleIdentity texId = EmptyIdentity;
            chunkRep->usesAtlas = false;
            Texture *newTex = NULL;
            if (chunk.loadImage)
            {
                // Let's just deal with square images
                int square = std::max(chunk.loadImage->width,chunk.loadImage->height);
                newTex = [chunk.loadImage buildTexture:borderTexel destWidth:square destHeight:square];
            }
            if (texAtlas)
            {
                if (newTex)
                {
                    texAtlas->addTexture(newTex, NULL, NULL, chunkRep->subTex, scene->getMemManager(), changes, borderTexel);
                    chunkRep->usesAtlas = true;
                    delete newTex;
                }
            } else {
                if (newTex)
                {
                    chunk.texId = newTex->getId();
                    chunkRep->texIDs.insert(newTex->getId());
                    changes.push_back(new AddTextureReq(newTex));
                }
                texId = chunk.texId;
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
                    skirtDraw->applySubTexture(chunkRep->subTex);
                    drawAtlas->addDrawable(skirtDraw, changes);
                    delete skirtDraw;
                } else {
                    if (texAtlas)
                        skirtDraw->applySubTexture(chunkRep->subTex);
                        else
                            skirtDraw->setTexId(texId);
                            changes.push_back(new AddDrawableReq(skirtDraw));
                            }
            }
            
            chunkRep->drawIDs.insert(drawable->getId());
            if (chunkRep->usesAtlas && drawAtlas)
            {
                drawable->applySubTexture(chunkRep->subTex);
                drawAtlas->addDrawable(drawable, changes);
                delete drawable;
            } else {
                if (texAtlas)
                    drawable->applySubTexture(chunkRep->subTex);
                    else
                        drawable->setTexId(texId);
                        changes.push_back(new AddDrawableReq(drawable));
                        }
            
            chunkReps.insert(chunkRep);
        }
            break;
        case ChunkEnable:
        {
            ChunkSceneRepRef dummyRef(new ChunkSceneRep(request.chunkId));
            ChunkRepSet::iterator it = chunkReps.find(dummyRef);
            if (it == chunkReps.end())
                return;
            (*it)->enable(texAtlas,drawAtlas,changes);
        }
            break;
        case ChunkDisable:
        {
            ChunkSceneRepRef dummyRef(new ChunkSceneRep(request.chunkId));
            ChunkRepSet::iterator it = chunkReps.find(dummyRef);
            if (it == chunkReps.end())
                return;
            (*it)->disable(texAtlas,drawAtlas,changes);
        }
            break;
        case ChunkRemove:
        {
            ChunkSceneRepRef dummyRef(new ChunkSceneRep(request.chunkId));
            ChunkRepSet::iterator it = chunkReps.find(dummyRef);
            if (it == chunkReps.end())
                return;
            
            (*it)->clear(scene,texAtlas,drawAtlas,changes);
            
            chunkReps.erase(it);
        }
            break;
    }
}


}
