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

#import <set>
#import "SphericalEarthChunkLayer.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{

// Used to track scene data associated with a chunk
class ChunkSceneRep : public Identifiable
{
public:
    ChunkSceneRep(SimpleIdentity theId) : Identifiable(theId) { }
    ChunkSceneRep() { }
    SimpleIDSet drawIDs;
    
    // Remove elements from the scene
    void clear(Scene *scene,std::vector<ChangeRequest *> &changeRequests)
    {
        for (SimpleIDSet::iterator it = drawIDs.begin();
             it != drawIDs.end(); ++it)
            changeRequests.push_back(new RemDrawableReq(*it));
    }
    
    // Enable drawables
    void enable(Scene *scene)
    {
        for (SimpleIDSet::iterator it = drawIDs.begin();
             it != drawIDs.end(); ++it)
            scene->addChangeRequest(new OnOffChangeRequest(*it, true));
    }
    
    // Disable drawables
    void disable(Scene *scene)
    {
        for (SimpleIDSet::iterator it = drawIDs.begin();
             it != drawIDs.end(); ++it)
            scene->addChangeRequest(new OnOffChangeRequest(*it, false));
    }
};
    
typedef boost::shared_ptr<ChunkSceneRep> ChunkSceneRepRef;
typedef std::set<ChunkSceneRepRef,IdentifiableRefSorter> ChunkRepSet;

}

@implementation WhirlyKitSphericalChunk

@synthesize mbr;
@synthesize texId;
@synthesize drawOffset;
@synthesize drawPriority;
@synthesize sampleX,sampleY;
@synthesize minVis,maxVis;
@synthesize minVisBand,maxVisBand;
@synthesize rotation;

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
    NSArray *chunks;
}
@end

@implementation SphericalChunkInfo

@end


@implementation WhirlyKitSphericalChunkLayer
{
    ChunkRepSet chunkReps;
    WhirlyKitLayerThread *layerThread;
    WhirlyKit::Scene *scene;
}

@synthesize ignoreEdgeMatching;

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    ignoreEdgeMatching = false;
    
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
        (*it)->clear(scene,changeRequests);
    chunkReps.clear();
    
    scene->addChangeRequests(changeRequests);
    
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
        draw->addTriangle(BasicDrawable::Triangle(base+0,base+3,base+1));
        draw->addTriangle(BasicDrawable::Triangle(base+1,base+3,base+2));
    }
}

// Do the work of adding the chunk to the scene
- (void)runAddChunk:(SphericalChunkInfo *)chunkInfo
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    CoordSystem *localSys = coordAdapter->getCoordSystem();
    ChunkSceneRepRef chunkRep(new ChunkSceneRep(chunkInfo->chunkId));
    
    // Work through the chunks
#ifdef DEBUG
//    NSTimeInterval now = CFAbsoluteTimeGetCurrent();
#endif
    std::vector<ChangeRequest *> changeRequests;
    for (WhirlyKitSphericalChunk *chunk in chunkInfo->chunks)
    {
        GeoMbr geoMbr = chunk.mbr;
        
        BasicDrawable *drawable = new BasicDrawable();
        drawable->setType(GL_TRIANGLES);
        drawable->setLocalMbr(geoMbr);
        drawable->setDrawPriority(chunk.drawPriority);
        drawable->setDrawOffset(chunk.drawOffset);
        drawable->setTexId(chunk.texId);
        drawable->setOnOff(chunkInfo->enable);
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
                triA.verts[0] = iy*(chunk.sampleX+1)+ix;
                triA.verts[1] = iy*(chunk.sampleX+1)+(ix+1);
                triA.verts[2] = (iy+1)*(chunk.sampleX+1)+(ix+1);
                triB.verts[0] = triA.verts[0];
                triB.verts[1] = triA.verts[2];
                triB.verts[2] = (iy+1)*(chunk.sampleX+1)+ix;
                drawable->addTriangle(triA);
                drawable->addTriangle(triB);
            }
        }
        
        // Build the skirts
        if (!ignoreEdgeMatching && !coordAdapter->isFlat())
        {
            BasicDrawable *skirtDrawable = new BasicDrawable();
            skirtDrawable->setType(GL_TRIANGLES);
            skirtDrawable->setLocalMbr(geoMbr);
            skirtDrawable->setDrawPriority(chunk.drawPriority);
            skirtDrawable->setDrawOffset(chunk.drawOffset);
            skirtDrawable->setTexId(chunk.texId);
            skirtDrawable->setOnOff(chunkInfo->enable);
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
            changeRequests.push_back(new AddDrawableReq(skirtDrawable));
        }
        
        [EAGLContext setCurrentContext:layerThread.glContext];
        WhirlyKitGLSetupInfo *setupInfo = [[WhirlyKitGLSetupInfo alloc] init];
        // Note: This is a giant hack
        setupInfo->minZres = 0.0001;
        drawable->setupGL(setupInfo, scene->getMemManager());
        
        chunkRep->drawIDs.insert(drawable->getId());
        changeRequests.push_back(new AddDrawableReq(drawable));
    }
    
    scene->addChangeRequests(changeRequests);
    
    chunkReps.insert(chunkRep);
}

// Do the work to remove the chunk from the scene
- (void)runRemoveChunk:(NSNumber *)num
{
    SimpleIdentity chunkId = [num integerValue];
    
    ChunkSceneRepRef dummyRef(new ChunkSceneRep(chunkId));
    ChunkRepSet::iterator it = chunkReps.find(dummyRef);
    if (it == chunkReps.end())
        return;
    
    std::vector<ChangeRequest *> changeRequests;
    (*it)->clear(scene,changeRequests);
    scene->addChangeRequests(changeRequests);
    
    chunkReps.erase(it);
}

- (void)runToggleChunk:(NSArray *)args
{
    SimpleIdentity chunkId = [[args objectAtIndex:0] integerValue];
    bool enable = [[args objectAtIndex:1] boolValue];
    
    ChunkSceneRepRef dummyRef(new ChunkSceneRep(chunkId));
    ChunkRepSet::iterator it = chunkReps.find(dummyRef);
    if (it == chunkReps.end())
        return;
    
    if (enable)
        (*it)->enable(scene);
    else
        (*it)->disable(scene);
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
    chunkInfo->chunks = [NSArray arrayWithObject:chunk];
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
