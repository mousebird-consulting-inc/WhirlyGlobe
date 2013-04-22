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
@synthesize eps;
@synthesize minVis,maxVis;
@synthesize minVisBand,maxVisBand;
@synthesize rotation;

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    // Adaptive sample is the default
    sampleX = sampleY = 12;
    eps = 0.0005;
    minVis = DrawVisibleInvalid;
    maxVis = DrawVisibleInvalid;
    
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
        thisSampleX = std::max(1,thisSampleX);
        thisSampleY = std::max(1,thisSampleY);
        thisSampleX = std::min(thisSampleX,sampleX);
        thisSampleY = std::min(thisSampleY,sampleY);
    }    
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
        skirtDrawable->setDrawPriority(drawPriority);
        skirtDrawable->setDrawOffset(drawOffset);
        skirtDrawable->setTexId(texId);
        skirtDrawable->setOnOff(enable);
        skirtDrawable->setVisibleRange(minVis, maxVis);
        skirtDrawable->setForceZBufferOn(true);
        
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
    int borderTexel;
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
    borderTexel = 0;
    
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
            ChunkSceneRepRef chunkRep(new ChunkSceneRep(request.chunkInfo->chunkId));
            WhirlyKitSphericalChunk *chunk = request.chunk;
    
            GeoMbr geoMbr = chunk.mbr;
            
            // If the atlases aren't initialized, do that
            if (chunk.loadImage && useDynamicAtlas && !texAtlas)
            {
                // At 256 pixels square we can hold 64 tiles in a texture atlas
                int DrawBufferSize = 2 * (8 + 1) * (8 + 1) * SingleVertexSize * 64;
                // Two triangles per grid cell in a tile
                int ElementBufferSize = 2 * 6 * (8 + 1) * (8 + 1) * SingleElementSize * 64;
                // Note: We should be able to set one of the compressed texture formats on the layer
                texAtlas = new DynamicTextureAtlas(2048,64,GL_UNSIGNED_BYTE);
                drawAtlas = new DynamicDrawableAtlas("Tile Quad Loader",SingleVertexSize,SingleElementSize,DrawBufferSize,ElementBufferSize,scene->getMemManager());
                borderTexel = 1;
            }
            
            // May need to set up the texture
            SimpleIdentity texId = EmptyIdentity;
            chunkRep->usesAtlas = false;
            if (chunk.loadImage && texAtlas)
            {
                Texture *newTex = [chunk.loadImage buildTexture:borderTexel destWidth:0 destHeight:0];
                if (newTex)
                {
                        texAtlas->addTexture(newTex, NULL, chunkRep->subTex, scene->getMemManager(), changes, borderTexel);
                    chunkRep->usesAtlas = true;
                }
            } else
                texId = chunk.texId;

            // Build the main drawable and possibly skirt
            BasicDrawable *drawable = nil,*skirtDraw = nil;
            [chunk buildDrawable:&drawable skirtDraw:(ignoreEdgeMatching ? nil : &skirtDraw) enabled:request.chunkInfo->enable adapter:coordAdapter];
            
            if (skirtDraw)
            {
                chunkRep->drawIDs.insert(skirtDraw->getId());
                if (chunkRep->usesAtlas)
                {
                    skirtDraw->applySubTexture(chunkRep->subTex);
                    drawAtlas->addDrawable(skirtDraw, changes);
                    chunkRep->drawables.push_back(skirtDraw);
                } else
                    changes.push_back(new AddDrawableReq(skirtDraw));
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
