/*
 *  SphericalEarthChunkLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/29/12.
 *  Copyright 2011-2012 mousebird consulting
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
#import "SphericalEarthChunkLayer.h"
#import "DynamicTextureAtlas.h"
#import "DynamicDrawableAtlas.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{

// Used to track scene data associated with a chunk
class ChunkSceneRep : public Identifiable
{
public:
    ChunkSceneRep(SimpleIdentity theId) : Identifiable(theId) { }
    ChunkSceneRep() : usesAtlas(false) { subTex.texId = EmptyIdentity; }
    ~ChunkSceneRep()
    {
        for (unsigned int ii=0;ii<drawables.size();ii++)
            delete drawables[ii];
    }
    SimpleIDSet drawIDs;
    bool usesAtlas;
    // Set if we're using a dynamic texture atlas
    SubTexture subTex;
    // If we're using atlases, we have to keep the geometry around for enable/disable
    std::vector<BasicDrawable *> drawables;
    
    // Remove elements from the scene
    void clear(Scene *scene,DynamicTextureAtlas *texAtlas,DynamicDrawableAtlas *drawAtlas,std::vector<ChangeRequest *> &changeRequests)
    {
        if (usesAtlas)
        {
            if (subTex.texId != EmptyIdentity)
                texAtlas->removeTexture(subTex, changeRequests);
            for (SimpleIDSet::iterator it = drawIDs.begin();
                 it != drawIDs.end(); ++it)
                drawAtlas->removeDrawable(*it, changeRequests);
        } else {
            for (SimpleIDSet::iterator it = drawIDs.begin();
                 it != drawIDs.end(); ++it)
                changeRequests.push_back(new RemDrawableReq(*it));
        }
    }
    
    // Enable drawables
    void enable(DynamicTextureAtlas *texAtlas,DynamicDrawableAtlas *drawAtlas,std::vector<ChangeRequest *> &changes)
    {
        if (usesAtlas)
        {
            for (unsigned int ii=0;ii<drawables.size();ii++)
                drawAtlas->addDrawable(drawables[ii], changes);
        } else {
            for (SimpleIDSet::iterator it = drawIDs.begin();
                 it != drawIDs.end(); ++it)
                changes.push_back(new OnOffChangeRequest(*it, true));
        }
    }
    
    // Disable drawables
    void disable(DynamicTextureAtlas *texAtlas,DynamicDrawableAtlas *drawAtlas,std::vector<ChangeRequest *> &changes)
    {
        if (usesAtlas)
        {
            for (unsigned int ii=0;ii<drawables.size();ii++)
                drawAtlas->removeDrawable(drawables[ii]->getId(), changes);
        } else {
            for (SimpleIDSet::iterator it = drawIDs.begin();
                 it != drawIDs.end(); ++it)
                changes.push_back(new OnOffChangeRequest(*it, false));
        }
    }
};
    
typedef boost::shared_ptr<ChunkSceneRep> ChunkSceneRepRef;
typedef std::set<ChunkSceneRepRef,IdentifiableRefSorter> ChunkRepSet;

}

@implementation WhirlyKitSphericalChunk

@synthesize mbr;
@synthesize texId;
@synthesize loadImage;
@synthesize drawOffset;
@synthesize drawPriority;
@synthesize sampleX,sampleY;
@synthesize minVis,maxVis;
@synthesize minVisBand,maxVisBand;
@synthesize rotation;

// Default sample size for chunks
static const int DefaultSampleX = 8;
static const int DefaultSampleY = 8;

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    sampleX = sampleY = 8;
    minVis = DrawVisibleInvalid;
    maxVis = DrawVisibleInvalid;
    
    return self;
}

@end

// Used to pass chunks between main and layer threads
@interface SphericalChunkInfo : NSObject
{
@public
    bool enable;
    WhirlyKit::SimpleIdentity chunkId;
    WhirlyKitSphericalChunk *chunk;
}
@end

@implementation SphericalChunkInfo

@end

// Used to track requests that come in so we can queue them
typedef enum {ChunkAdd,ChunkRemove,ChunkEnable,ChunkDisable} ChunkRequestType;
class ChunkRequest
{
public:
    ChunkRequest() { }
    ChunkRequest(ChunkRequestType type,SphericalChunkInfo *chunkInfo,WhirlyKitSphericalChunk *chunk) :
        type(type), chunkId(EmptyIdentity), chunkInfo(chunkInfo), chunk(chunk) { }
    ChunkRequest(ChunkRequestType type,SimpleIdentity chunkId) :
        type(type), chunkId(chunkId), chunkInfo(NULL), chunk(NULL) { }
    ChunkRequestType type;
    SimpleIdentity chunkId;
    SphericalChunkInfo *chunkInfo;
    WhirlyKitSphericalChunk *chunk;
};

@implementation WhirlyKitSphericalChunkLayer
{
    ChunkRepSet chunkReps;
    WhirlyKitLayerThread *layerThread;
    WhirlyKit::Scene *scene;
    DynamicTextureAtlas *texAtlas;
    DynamicDrawableAtlas *drawAtlas;
    // Outstanding requests to process
    std::queue<ChunkRequest> requests;
}

@synthesize ignoreEdgeMatching;
@synthesize useDynamicAtlas;

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    ignoreEdgeMatching = false;
    useDynamicAtlas = true;
    
    return self;
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(Scene *)inScene
{
    layerThread = inLayerThread;
    scene = inScene;
}

- (void)shutdown
{
    std::vector<ChangeRequest *> changeRequests;

    for (ChunkRepSet::iterator it = chunkReps.begin();
         it != chunkReps.end(); ++it)
        (*it)->clear(scene,texAtlas,drawAtlas,changeRequests);
    chunkReps.clear();

    if (texAtlas)
    {
        texAtlas->shutdown(changeRequests);
        delete texAtlas;
        texAtlas = NULL;
    }
    
    if (drawAtlas)
    {
        drawAtlas->shutdown(changeRequests);
        delete drawAtlas;
        drawAtlas = NULL;
    }

    [layerThread addChangeRequests:(changeRequests)];
    
    scene = NULL;
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

// Note: This is the hardcoded vertex size for our case.  Should make this flexible.
static const int SingleVertexSize = 3*sizeof(float) + 2*sizeof(float) +  4*sizeof(unsigned char) + 3*sizeof(float);
static const int SingleElementSize = sizeof(GLushort);

// Add chunks to the outstanding list and possibly execute the list
- (void)runAddChunk:(SphericalChunkInfo *)chunkInfo
{
    std::vector<ChangeRequest *> changes;
    
    ChunkRequest request(ChunkAdd,chunkInfo,chunkInfo->chunk);
    // If it needs the altases, just queue it up
    if (chunkInfo->chunk.loadImage)
        requests.push(request);
    else {
        // If it doesn't, just run it right now
        [self processChunkRequest:request changes:changes];
    }
    
    // Get rid of an unnecessary circularity
    chunkInfo->chunk = nil;
    
    // Push out changes (if there are any)
    [layerThread addChangeRequests:changes];
    
    // And possibly run the outstanding request queue
    [self processQueue];
}

// Process outstanding requests
- (void)processQueue
{
    // Nothing to do
    if (requests.empty())
        return;
    
    // If there's an outstanding swap going on, can't do this now, wait for the wakeUp
    if (drawAtlas && drawAtlas->waitingOnSwap())
        return;
    
    // Process the changes and then flush them out
    std::vector<ChangeRequest *> changes;
    while (!requests.empty())
    {
        ChunkRequest request = requests.front();
        requests.pop();
        [self processChunkRequest:request changes:changes];
    }
    
    if (drawAtlas)
        if (drawAtlas->hasUpdates() && !drawAtlas->waitingOnSwap())
            drawAtlas->swap(changes, self, @selector(wakeUp));
    
    [layerThread addChangeRequests:changes];
}

// Called by the main thread to wake us up after a buffer swap
- (void)wakeUp
{
    if ([NSThread currentThread] != layerThread)
    {
        [self performSelector:@selector(wakeUp) onThread:layerThread withObject:nil waitUntilDone:NO];
        return;
    }
    
    [self processQueue];
}

// Process a single chunk request (add, enable, disable)
- (void)processChunkRequest:(ChunkRequest &)request changes:(std::vector<ChangeRequest *> &)changes
{
    switch (request.type)
    {
        case ChunkAdd:
        {
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    CoordSystem *localSys = coordAdapter->getCoordSystem();
            ChunkSceneRepRef chunkRep(new ChunkSceneRep(request.chunkInfo->chunkId));
            WhirlyKitSphericalChunk *chunk = request.chunk;
    
        GeoMbr geoMbr = chunk.mbr;
        
        // If the atlases aren't initialized, do that
        if (chunk.loadImage && useDynamicAtlas && !texAtlas)
        {
            // At 256 pixels square we can hold 64 tiles in a texture atlas
            int DrawBufferSize = 2 * (DefaultSampleX + 1) * (DefaultSampleY + 1) * SingleVertexSize * 64;
            // Two triangles per grid cell in a tile
            int ElementBufferSize = 2 * 6 * (DefaultSampleX + 1) * (DefaultSampleY + 1) * SingleElementSize * 64;
            // Note: We should be able to set one of the compressed texture formats on the layer
            texAtlas = new DynamicTextureAtlas(2048,64,GL_UNSIGNED_BYTE);
            drawAtlas = new DynamicDrawableAtlas("Tile Quad Loader",SingleVertexSize,SingleElementSize,DrawBufferSize,ElementBufferSize,scene->getMemManager());
        }
        
        // May need to set up the texture
        SimpleIdentity texId = EmptyIdentity;
        chunkRep->usesAtlas = false;
        if (chunk.loadImage && texAtlas)
        {
            Texture *newTex = [chunk.loadImage buildTexture];
            if (newTex)
            {
                    texAtlas->addTexture(newTex, chunkRep->subTex, scene->getMemManager(), changes);
                chunkRep->usesAtlas = true;
            }
        } else
            texId = chunk.texId;
        
        BasicDrawable *drawable = new BasicDrawable("Spherical Earth Chunk");
        drawable->setType(GL_TRIANGLES);
        drawable->setLocalMbr(geoMbr);
        drawable->setDrawPriority(chunk.drawPriority);
        drawable->setDrawOffset(chunk.drawOffset);
        drawable->setTexId(texId);
            drawable->setOnOff(request.chunkInfo->enable);
        drawable->setVisibleRange(chunk.minVis, chunk.maxVis, chunk.minVisBand, chunk.maxVisBand);
        
        std::vector<Point3f> locs((chunk.sampleX+1)*(chunk.sampleY+1));
        std::vector<TexCoord> texCoords((chunk.sampleX+1)*(chunk.sampleY+1));

        Point2f texIncr(1.0/chunk.sampleX,1.0/chunk.sampleY);
        // Without rotation, we'll just follow the geographic boundaries
        if (chunk.rotation == 0.0)
        {
            Point3f localLL,localUR;
            localLL = localSys->geographicToLocal(geoMbr.ll());
            localUR = localSys->geographicToLocal(geoMbr.ur());
            Point2f localIncr((localUR.x()-localLL.x())/chunk.sampleX,(localUR.y()-localLL.y())/chunk.sampleY);
            
            // Vertices
            for (unsigned int iy=0;iy<chunk.sampleY+1;iy++)
                for (unsigned int ix=0;ix<chunk.sampleX+1;ix++)
                {
                    Point3f loc(localLL.x() + ix * localIncr.x(), localLL.y() + iy * localIncr.y(), 0.0);
                    Point3f dispLoc = coordAdapter->localToDisplay(loc);

                    locs[iy*(chunk.sampleX+1)+ix] = dispLoc;
                    TexCoord texCoord(ix * texIncr.x(), 1.0-iy * texIncr.y());
                    texCoords[iy*(chunk.sampleX+1)+ix] = texCoord;
                    
                    drawable->addPoint(dispLoc);
                    drawable->addTexCoord(texCoord);
                    drawable->addNormal(dispLoc);
                }
        } else {
            // With rotation, we need to handle this differently
            // Convert the four corners into place
            Point3f dispPts[4];
            dispPts[0] = coordAdapter->localToDisplay(localSys->geographicToLocal(geoMbr.ll()));
            dispPts[1] = coordAdapter->localToDisplay(localSys->geographicToLocal(geoMbr.lr()));
            dispPts[2] = coordAdapter->localToDisplay(localSys->geographicToLocal(geoMbr.ur()));
            dispPts[3] = coordAdapter->localToDisplay(localSys->geographicToLocal(geoMbr.ul()));
            
            // Rotate around the center
            Point3f center = (dispPts[0] + dispPts[1] + dispPts[2] + dispPts[3])/4.0;
            Eigen::Affine3f rot(AngleAxisf(chunk.rotation,center));
            Eigen::Matrix4f mat = rot.matrix();

            // Rotate the corners
            for (unsigned int ii=0;ii<4;ii++)
            {
                Vector3f &dispPt = dispPts[ii];
                Vector4f dispPost = mat * Vector4f(dispPt.x(),dispPt.y(),dispPt.z(),1.0);
                dispPt = Point3f(dispPost.x(),dispPost.y(),dispPost.z());
            }
            
            Point3f vecA = dispPts[1] - dispPts[0];
            Point3f vecB = dispPts[2] - dispPts[3];
        
            // Vertices
            for (unsigned int iy=0;iy<chunk.sampleY+1;iy++)
                for (unsigned int ix=0;ix<chunk.sampleX+1;ix++)
                {
                        Point3f ptA = dispPts[0] + ix * vecA / chunk.sampleX;
                        Point3f ptB = dispPts[3] + ix * vecB / chunk.sampleX;
                        Point3f dispLoc = ptA + iy * (ptB-ptA) / chunk.sampleY;
                        if (!coordAdapter->isFlat())
                            dispLoc.normalize();

                    locs[iy*(chunk.sampleX+1)+ix] = dispLoc;
                    TexCoord texCoord(ix * texIncr.x(), 1.0-iy * texIncr.y());
                    texCoords[iy*(chunk.sampleX+1)+ix] = texCoord;
                    
                    drawable->addPoint(dispLoc);
                    drawable->addTexCoord(texCoord);
                    drawable->addNormal(dispLoc);
                }
        }
        
        // Two triangles per cell
        for (unsigned int iy=0;iy<chunk.sampleY;iy++)
        {
            for (unsigned int ix=0;ix<chunk.sampleX;ix++)
            {
                BasicDrawable::Triangle triA,triB;
                triA.verts[0] = (iy+1)*(chunk.sampleX+1)+ix;
                triA.verts[1] = iy*(chunk.sampleX+1)+ix;
                triA.verts[2] = (iy+1)*(chunk.sampleX+1)+(ix+1);
                triB.verts[0] = triA.verts[2];
                triB.verts[1] = triA.verts[1];
                triB.verts[2] = iy*(chunk.sampleX+1)+(ix+1);
                drawable->addTriangle(triA);
                drawable->addTriangle(triB);
            }
        }
        
        // Build the skirts
        if (!ignoreEdgeMatching && !coordAdapter->isFlat())
        {
            BasicDrawable *skirtDrawable = new BasicDrawable("Spherical Earth Chunk Skirts");
            skirtDrawable->setType(GL_TRIANGLES);
            skirtDrawable->setLocalMbr(geoMbr);
            skirtDrawable->setDrawPriority(chunk.drawPriority);
            skirtDrawable->setDrawOffset(chunk.drawOffset);
            skirtDrawable->setTexId(chunk.texId);
                skirtDrawable->setOnOff(request.chunkInfo->enable);
            skirtDrawable->setVisibleRange(chunk.minVis, chunk.maxVis);
            skirtDrawable->setForceZBufferOn(true);
            
            // Bottom skirt
            std::vector<Point3f> skirtLocs;
            std::vector<TexCoord> skirtTexCoords;
            for (unsigned int ix=0;ix<=chunk.sampleX;ix++)
            {
                skirtLocs.push_back(locs[ix]);
                skirtTexCoords.push_back(texCoords[ix]);
            }
            [self buildSkirt:skirtDrawable pts:skirtLocs tex:skirtTexCoords];
            // Top skirt
            skirtLocs.clear();
            skirtTexCoords.clear();
            for (int ix=chunk.sampleX;ix>=0;ix--)
            {
                skirtLocs.push_back(locs[(chunk.sampleY)*(chunk.sampleX+1)+ix]);
                skirtTexCoords.push_back(texCoords[(chunk.sampleY)*(chunk.sampleX+1)+ix]);
            }
            [self buildSkirt:skirtDrawable pts:skirtLocs tex:skirtTexCoords];
            // Left skirt
            skirtLocs.clear();
            skirtTexCoords.clear();
            for (int iy=chunk.sampleY;iy>=0;iy--)
            {
                skirtLocs.push_back(locs[(chunk.sampleX+1)*iy+0]);
                skirtTexCoords.push_back(texCoords[(chunk.sampleX+1)*iy+0]);
            }
            [self buildSkirt:skirtDrawable pts:skirtLocs tex:skirtTexCoords];
            // right skirt
            skirtLocs.clear();
            skirtTexCoords.clear();
            for (int iy=0;iy<=chunk.sampleY;iy++)
            {
                skirtLocs.push_back(locs[(chunk.sampleX+1)*iy+(chunk.sampleX)]);
                skirtTexCoords.push_back(texCoords[(chunk.sampleX+1)*iy+(chunk.sampleX)]);
            }
            [self buildSkirt:skirtDrawable pts:skirtLocs tex:skirtTexCoords];

            chunkRep->drawIDs.insert(skirtDrawable->getId());
            if (chunkRep->usesAtlas)
            {
                skirtDrawable->applySubTexture(chunkRep->subTex);
                    drawAtlas->addDrawable(skirtDrawable, changes);
                chunkRep->drawables.push_back(skirtDrawable);
            } else
                    changes.push_back(new AddDrawableReq(skirtDrawable));
        }

        chunkRep->drawIDs.insert(drawable->getId());
        if (chunkRep->usesAtlas)
        {
            drawable->applySubTexture(chunkRep->subTex);
                drawAtlas->addDrawable(drawable, changes);
            chunkRep->drawables.push_back(drawable);
        } else
                changes.push_back(new AddDrawableReq(drawable));
    
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
    
    std::vector<ChangeRequest *> changeRequests;
    (*it)->clear(scene,texAtlas,drawAtlas,changeRequests);
    [layerThread addChangeRequests:(changeRequests)];
    
    chunkReps.erase(it);
}
            break;
    }
}

// Do the work to remove the chunk from the scene
- (void)runRemoveChunk:(NSNumber *)num
{
    requests.push(ChunkRequest(ChunkRemove,[num integerValue]));
    
    [self processQueue];
}

- (void)runToggleChunk:(NSArray *)args
{
    SimpleIdentity chunkId = [[args objectAtIndex:0] integerValue];
    bool enable = [[args objectAtIndex:1] boolValue];
    
    std::vector<ChangeRequest *> changes;
    if (enable)
        requests.push(ChunkRequest(ChunkEnable,chunkId));
    else
        requests.push(ChunkRequest(ChunkDisable,chunkId));

    [self processQueue];
}

/// Add a single chunk on the spherical earth.  This returns and ID
///  we can use to remove it later.
- (WhirlyKit::SimpleIdentity)addChunk:(WhirlyKitSphericalChunk *)chunk enable:(bool)enable
{
    if (!layerThread)
    {
        NSLog(@"WhirlyKitSphericalChunk: addChunk called before layer initialized.");
        return EmptyIdentity;
    }

    SphericalChunkInfo *chunkInfo = [[SphericalChunkInfo alloc] init];
    chunkInfo->enable = enable;
    chunkInfo->chunk = chunk;
    chunkInfo->chunkId = Identifiable::genId();
    if ([NSThread currentThread] == layerThread)
        [self runAddChunk:chunkInfo];
    else
        [self performSelector:@selector(runAddChunk:) onThread:layerThread withObject:chunkInfo waitUntilDone:NO];
    
    return chunkInfo->chunkId;
}

/// Remove the named chunk
- (void)removeChunk:(WhirlyKit::SimpleIdentity)chunkId
{
    if (!layerThread)
    {
        NSLog(@"WhirlyKitSphericalChunk: removeChunk called before layer initialized.");
        return;
    }
    
    NSNumber *numId = [NSNumber numberWithInt:chunkId];
    if ([NSThread currentThread] == layerThread)
        [self runRemoveChunk:numId];
    else
        [self performSelector:@selector(runRemoveChunk:) onThread:layerThread withObject:numId waitUntilDone:NO];
}

- (void)toogleChunk:(WhirlyKit::SimpleIdentity)chunkId enable:(bool)enable
{
    if (!layerThread)
    {
        NSLog(@"WhirlyKitSphericalChunk: toogleChunk called before layer initialized.");
        return;
    }
    
    NSArray *args = [NSArray arrayWithObjects:[NSNumber numberWithInt:chunkId],[NSNumber numberWithBool:enable],nil];    
    if ([NSThread currentThread] == layerThread)
        [self runToggleChunk:args];
    else
        [self performSelector:@selector(runToggleChunk:) onThread:layerThread withObject:args waitUntilDone:NO];
}

- (int)numChunk
{
    if ([NSThread currentThread] != layerThread)
        return 0;
    
    return chunkReps.size();
}

@end
