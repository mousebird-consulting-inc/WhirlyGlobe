/*
 *  LoftLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/16/11.
 *  Copyright 2011-2012 mousebird consulting. All rights reserved.
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

#import "LoftLayer.h"
#import "GridClipper.h"
#import "Tesselator.h"
#import "UIColor+Stuff.h"
#import "NSDictionary+Stuff.h"

using namespace WhirlyKit;
using namespace WhirlyGlobe;

// Used to describe the drawables we want to construct for a given vector
@interface LoftedPolyInfo : NSObject
{
@public
    SimpleIdentity sceneRepId;
    // For a creation request
    ShapeSet    shapes;
    UIColor     *color;
    NSString    *key;
    float       height;
    float       fade;
    float       minVis,maxVis;
    int         priority;
    bool        top,side;
    NSObject<WhirlyKitLoftedPolyCache> *cache;
}

@property (nonatomic) UIColor *color;
@property (nonatomic) NSString *key;
@property (nonatomic,assign) float fade;

- (void)parseDesc:(NSDictionary *)desc key:(NSString *)key;

@end

@implementation LoftedPolyInfo

@synthesize color;
@synthesize key;
@synthesize fade;

- (id)initWithShapes:(ShapeSet *)inShapes desc:(NSDictionary *)desc key:(NSString *)inKey
{
    if ((self = [super init]))
    {
        if (inShapes)
            shapes = *inShapes;
        [self parseDesc:desc key:inKey];
    }
    
    return self;
}

- (id)initWithSceneRepId:(SimpleIdentity)inId desc:(NSDictionary *)desc
{
    if ((self = [super init]))
    {
        sceneRepId = inId;
        [self parseDesc:desc key:nil];
    }
    
    return self;
}


- (void)parseDesc:(NSDictionary *)dict key:(NSString *)inKey
{
    self.color = [dict objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]];
    priority = [dict intForKey:@"priority" default:0];
    height = [dict floatForKey:@"height" default:.01];
    minVis = [dict floatForKey:@"minVis" default:DrawVisibleInvalid];
    maxVis = [dict floatForKey:@"maxVis" default:DrawVisibleInvalid];
    fade = [dict floatForKey:@"fade" default:0.0];
    top = [dict boolForKey:@"top" default:true];
    side = [dict boolForKey:@"side" default:true];
    self.key = inKey;
}

@end

namespace WhirlyKit
{
    
// Read the lofted poly representation from a cache file
// We're just saving the MBR and triangle mesh here
bool LoftedPolySceneRep::readFromCache(NSObject<WhirlyKitLoftedPolyCache> *cache,NSString *key)
{    
    if (!cache)
        return false;

    NSData *data = [cache readLoftedPolyData:key];
    if (!data)
        return false;
    
    try {
        // MBR first
        float ll_x,ll_y,ur_x,ur_y;
        unsigned int loc = 0;
        [data getBytes:&ll_x range:NSMakeRange(loc, sizeof(float))];  loc += sizeof(float);  if (loc > [data length])  throw 1;
        [data getBytes:&ll_y range:NSMakeRange(loc, sizeof(float))];  loc += sizeof(float);  if (loc > [data length])  throw 1;
        [data getBytes:&ur_x range:NSMakeRange(loc, sizeof(float))];  loc += sizeof(float);  if (loc > [data length])  throw 1;
        [data getBytes:&ur_y range:NSMakeRange(loc, sizeof(float))];  loc += sizeof(float);  if (loc > [data length])  throw 1;
        shapeMbr.addGeoCoord(GeoCoord(ll_x,ll_y));
        shapeMbr.addGeoCoord(GeoCoord(ur_x,ur_y));
        
        // Triangle meshes
        unsigned int numMesh = 0;
        [data getBytes:&numMesh range:NSMakeRange(loc, sizeof(unsigned int))];  loc += sizeof(unsigned int);  if (loc > [data length])  throw 1;
        triMesh.resize(numMesh);
        for (unsigned int ii=0;ii<numMesh;ii++)
        {
            VectorRing &ring = triMesh[ii];
            unsigned int numPt = 0;
            [data getBytes:&numPt range:NSMakeRange(loc, sizeof(unsigned int))];  loc += sizeof(unsigned int);  if (loc > [data length])  throw 1;
            ring.resize(numPt);
            for (unsigned int jj=0;jj<numPt;jj++)
            {
                Point2f &pt = ring[jj];
                float x,y;
                [data getBytes:&x range:NSMakeRange(loc, sizeof(float))];  loc += sizeof(float);  if (loc > [data length])  throw 1;
                [data getBytes:&y range:NSMakeRange(loc, sizeof(float))];  loc += sizeof(float);  if (loc > [data length])  throw 1;
                pt.x() = x;
                pt.y() = y;                
            }
        }        
    }
    catch (...)
    {
        return false;
    }
    
    return true;
}
    
// Write the lofted poly representation to a cache
// Just the MBR and triangle mesh
bool LoftedPolySceneRep::writeToCache(NSObject<WhirlyKitLoftedPolyCache> *cache,NSString *key)
{
    NSMutableData *data = [NSMutableData dataWithCapacity:0];
    
    // MBR first
    GeoCoord ll = shapeMbr.ll(), ur = shapeMbr.ur();
    [data appendBytes:&ll.x() length:sizeof(float)];
    [data appendBytes:&ll.y() length:sizeof(float)];
    [data appendBytes:&ur.x() length:sizeof(float)];
    [data appendBytes:&ur.y() length:sizeof(float)];
    
    // Triangle meshes
    unsigned int numMesh = triMesh.size();
    [data appendBytes:&numMesh length:sizeof(unsigned int)];
    for (unsigned int ii=0;ii<numMesh;ii++)
    {
        VectorRing &ring = triMesh[ii];
        unsigned int numPt = ring.size();
        [data appendBytes:&numPt length:sizeof(unsigned int)];
        for (unsigned int jj=0;jj<numPt;jj++)
        {
            Point2f &pt = ring[jj];
            [data appendBytes:&pt.x() length:sizeof(float)];
            [data appendBytes:&pt.y() length:sizeof(float)];
        }
    }

    return [cache writeLoftedPolyData:data cacheName:key];
}
    
/* Drawable Builder
 Used to construct drawables with multiple shapes in them.
 Eventually, will move this out to be a more generic object.
 */
class DrawableBuilder2
{
public:
    DrawableBuilder2(Scene *scene,LoftedPolySceneRep *sceneRep,
                     LoftedPolyInfo *polyInfo,const GeoMbr &inDrawMbr)
    : scene(scene), sceneRep(sceneRep), polyInfo(polyInfo), drawable(NULL)
    {
        primType = GL_TRIANGLES;
        drawMbr = inDrawMbr;
    }
    
    ~DrawableBuilder2()
    {
        flush();
    }
    
    // Initialize or flush a drawable, as needed
    void setupDrawable(int numToAdd)
    {
        if (!drawable || (drawable->getNumPoints()+numToAdd > MaxDrawablePoints))
        {
            // We're done with it, toss it to the scene
            if (drawable)
                flush();
            
            drawable = new BasicDrawable();
            drawable->setType(primType);
            // Adjust according to the vector info
            //            drawable->setOnOff(polyInfo->enable);
            //            drawable->setDrawOffset(vecInfo->drawOffset);
            drawable->setColor([polyInfo.color asRGBAColor]);
            drawable->setAlpha(true);
            drawable->setForceZBufferOn(true);
            //            drawable->setDrawPriority(vecInfo->priority);
            //            drawable->setVisibleRange(vecInfo->minVis,vecInfo->maxVis);
        }
    }
    
    // Add a triangle, keeping track of limits
    void addLoftTriangle(Point2f verts[3])
    {
        CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        setupDrawable(3);
        
        int startVert = drawable->getNumPoints();
        for (unsigned int ii=0;ii<3;ii++)
        {
            // Get some real world coordinates and corresponding normal
            Point2f &geoPt = verts[ii];
            GeoCoord geoCoord = GeoCoord(geoPt.x(),geoPt.y());
            Point3f localPt = coordAdapter->getCoordSystem()->geographicToLocal(geoCoord);
            Point3f dispPt = coordAdapter->localToDisplay(localPt);
            Point3f norm = coordAdapter->normalForLocal(localPt);
            Point3f pt1 = dispPt + norm * polyInfo->height;
            
            drawable->addPoint(pt1);
            drawable->addNormal(norm);
        }
        
        BasicDrawable::Triangle tri;
        tri.verts[0] = startVert;
        tri.verts[1] = startVert+1;
        tri.verts[2] = startVert+2;
        drawable->addTriangle(tri);
    }

    // Add a whole mess of triangles, adding
    //  in the height
    void addPolyGroup(std::vector<VectorRing> &rings)
    {
        for (unsigned int ii=0;ii<rings.size();ii++)
        {
            VectorRing &tri = rings[ii];
            if (tri.size() == 3)
            {
                Point2f verts[3];
                verts[2] = tri[0];  verts[1] = tri[1];  verts[0] = tri[2];
                addLoftTriangle(verts);
            }
        }
    }
    
    void addSkirtPoints(VectorRing &pts)
    {            
        CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        
        // Decide if we'll appending to an existing drawable or
        //  create a new one
        int ptCount = 4*(pts.size()+1);
        setupDrawable(ptCount);
        
        Point3f prevPt0,prevPt1,prevNorm,firstPt0,firstPt1,firstNorm;
        for (unsigned int jj=0;jj<pts.size();jj++)
        {
            // Get some real world coordinates and corresponding normal
            Point2f &geoPt = pts[jj];
            GeoCoord geoCoord = GeoCoord(geoPt.x(),geoPt.y());
            Point3f localPt = coordAdapter->getCoordSystem()->geographicToLocal(geoCoord);
            Point3f norm = coordAdapter->normalForLocal(localPt);
            Point3f pt0 = coordAdapter->localToDisplay(localPt);
            Point3f pt1 = pt0 + norm * polyInfo->height;
                        
            // Add to drawable
            if (jj > 0)
            {
                int startVert = drawable->getNumPoints();
                drawable->addPoint(prevPt0);
                drawable->addPoint(prevPt1);
                drawable->addPoint(pt1);
                drawable->addPoint(pt0);

                // Normal points out
                Point3f crossNorm = norm.cross(pt1-prevPt1);
                crossNorm *= -1;
                
                drawable->addNormal(crossNorm);
                drawable->addNormal(crossNorm);
                drawable->addNormal(crossNorm);
                drawable->addNormal(crossNorm);
                
                BasicDrawable::Triangle triA,triB;
                triA.verts[0] = startVert+0;
                triA.verts[1] = startVert+1;
                triA.verts[2] = startVert+3;
                triB.verts[0] = startVert+1;
                triB.verts[1] = startVert+2;
                triB.verts[2] = startVert+3;
                
                drawable->addTriangle(triA);
                drawable->addTriangle(triB);
            } else {
                firstPt0 = pt0;
                firstPt1 = pt1;
                firstNorm = norm;
            }
            prevPt0 = pt0;  prevPt1 = pt1;
            prevNorm = norm;
        }
        
        // Close the loop
        if (primType == GL_LINES)
        {
            int startVert = drawable->getNumPoints();
            drawable->addPoint(prevPt0);
            drawable->addPoint(prevPt1);
            drawable->addPoint(firstPt1);
            drawable->addPoint(firstPt0);

            Point3f crossNorm = prevNorm.cross(firstPt1-prevPt1);
            crossNorm *= -1;
            drawable->addNormal(crossNorm);
            drawable->addNormal(crossNorm);
            drawable->addNormal(crossNorm);
            drawable->addNormal(crossNorm);

            BasicDrawable::Triangle triA,triB;
            triA.verts[0] = startVert+0;
            triA.verts[1] = startVert+1;
            triA.verts[2] = startVert+3;
            triB.verts[0] = startVert+1;
            triB.verts[1] = startVert+2;
            triB.verts[2] = startVert+3;
        }
    }
    
    void flush()
    {
        if (drawable)
        {
            if (drawable->getNumPoints() > 0)
            {
                drawable->setLocalMbr(Mbr(Point2f(drawMbr.ll().x(),drawMbr.ll().y()),Point2f(drawMbr.ur().x(),drawMbr.ur().y())));
                sceneRep->drawIDs.insert(drawable->getId());
                if (polyInfo.fade > 0)
                {
                    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
                    drawable->setFade(curTime,curTime+polyInfo.fade);
                }
                scene->addChangeRequest(new AddDrawableReq(drawable));
            } else
                delete drawable;
            drawable = NULL;
        }
    }
    
protected:   
    Scene *scene;
    LoftedPolySceneRep *sceneRep;
    GeoMbr drawMbr;
    BasicDrawable *drawable;
    LoftedPolyInfo *polyInfo;
    GLenum primType;
};

}

@implementation WhirlyKitLoftLayer

@synthesize gridSize;

- (id)init
{
    if ((self = [super init]))
    {
        gridSize = 10.0 / 180.0 * M_PI;  // Default to 10 degrees
    }
    
    return self;
}

- (void)clear
{
    for (LoftedPolySceneRepMap::iterator it = polyReps.begin();
         it != polyReps.end(); ++it)
        delete it->second;
    polyReps.clear();   
    
    scene = NULL;
}

- (void)dealloc
{
    [self clear];
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(Scene *)inScene
{
	scene = inScene;
    layerThread = inLayerThread;
}

- (void)shutdown
{
    std::vector<ChangeRequest *> changeRequests;
    
    for (LoftedPolySceneRepMap::iterator it = polyReps.begin();
         it != polyReps.end(); ++it)
    {
        LoftedPolySceneRep *sceneRep = it->second;
        for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
             idIt != sceneRep->drawIDs.end(); ++idIt)
            changeRequests.push_back(new RemDrawableReq(*idIt));
    }
    
    [self clear];
}

// From a scene rep and a description, add the given polygons to the drawable builder
- (void)addGeometryToBuilder:(LoftedPolySceneRep *)sceneRep polyInfo:(LoftedPolyInfo *)polyInfo drawMbr:(GeoMbr &)drawMbr
{
    int numShapes = 0;
    
    // Used to toss out drawables as we go
    // Its destructor will flush out the last drawable
    DrawableBuilder2 drawBuild(scene,sceneRep,polyInfo,drawMbr);
    
    // Toss in the polygons for the sides
    if (polyInfo->height != 0.0)
    {
        for (ShapeSet::iterator it = sceneRep->shapes.begin();
             it != sceneRep->shapes.end(); ++it)
        {
            VectorArealRef theAreal = boost::dynamic_pointer_cast<VectorAreal>(*it);
            if (theAreal.get())
            {
                for (unsigned int ri=0;ri<theAreal->loops.size();ri++)
                {
                    if (polyInfo->side)
                    {
                        drawBuild.addSkirtPoints(theAreal->loops[ri]);
                        numShapes++;
                    }
                }
            }
        }
    }
    
    // Tweak the mesh polygons and toss 'em in
    if (polyInfo->top)
        drawBuild.addPolyGroup(sceneRep->triMesh);

//    printf("Added %d shapes and %d triangles from mesh\n",(int)numShapes,(int)sceneRep->triMesh.size());        
}

// Generate drawables for a lofted poly
- (void)runAddPoly:(LoftedPolyInfo *)polyInfo
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    LoftedPolySceneRep *sceneRep = new LoftedPolySceneRep();
    sceneRep->setId(polyInfo->sceneRepId);
    sceneRep->fade = polyInfo.fade;
    polyReps[sceneRep->getId()] = sceneRep;
    
    sceneRep->shapes = polyInfo->shapes;
    
    // Try reading from the cache
    if (!polyInfo.key || !sceneRep->readFromCache(polyInfo->cache,polyInfo.key))
    {
        // If that fails, we'll regenerate everything
        for (ShapeSet::iterator it = polyInfo->shapes.begin();
             it != polyInfo->shapes.end(); ++it)
        {
            VectorArealRef theAreal = boost::dynamic_pointer_cast<VectorAreal>(*it);        
            if (theAreal.get())
            {            
                // Work through the loops
                for (unsigned int ri=0;ri<theAreal->loops.size();ri++)
                {
                    VectorRing &ring = theAreal->loops[ri];					
                    
                    sceneRep->shapeMbr.addGeoCoords(ring);
                    
                    if (coordAdapter->isFlat())
                    {
                        // No grid to worry about, just tesselate
                        TesselateRing(ring, sceneRep->triMesh);
                    } else {                                                    
                        // Clip the polys for the top
                        std::vector<VectorRing> clippedMesh;
                        ClipLoopToGrid(ring,Point2f(0.f,0.f),Point2f(gridSize,gridSize),clippedMesh);

                        for (unsigned int ii=0;ii<clippedMesh.size();ii++)
                        {
                            VectorRing &ring = clippedMesh[ii];
                            // Tesselate the ring, even if it's concave (it's concave a lot)
                            TesselateRing(ring,sceneRep->triMesh);
                        }
                    }
                }
            }
        }
        
        // And save out to the cache if we're doing that
        if (polyInfo->cache)
            sceneRep->writeToCache(polyInfo->cache, polyInfo.key);
    }
    
//    printf("runAddPoly: handing off %d clipped loops to addGeometry\n",(int)sceneRep->triMesh.size());
    
    [self addGeometryToBuilder:sceneRep polyInfo:polyInfo drawMbr:sceneRep->shapeMbr];
}

// Change the visual representation of a lofted poly
- (void)runChangePoly:(LoftedPolyInfo *)polyInfo
{
    LoftedPolySceneRepMap::iterator it = polyReps.find(polyInfo->sceneRepId);
    if (it != polyReps.end())
    {
        LoftedPolySceneRep *sceneRep = it->second;

        // Clean out old geometry
        for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
             idIt != sceneRep->drawIDs.end(); ++idIt)
            scene->addChangeRequest(new RemDrawableReq(*idIt));
        sceneRep->drawIDs.clear();
        
        // And add the new back
        [self addGeometryToBuilder:sceneRep polyInfo:polyInfo drawMbr:sceneRep->shapeMbr];
    }
}

// Remove the lofted poly
- (void)runRemovePoly:(NSNumber *)num
{
    LoftedPolySceneRepMap::iterator it = polyReps.find([num intValue]);
    if (it != polyReps.end())
    {
        LoftedPolySceneRep *sceneRep = it->second;

        if (sceneRep->fade > 0.0)
        {
            NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
            for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
                 idIt != sceneRep->drawIDs.end(); ++idIt)
                scene->addChangeRequest(new FadeChangeRequest(*idIt,curTime,curTime+sceneRep->fade));                
            
            // Reset the fade and try to delete again later
            [self performSelector:@selector(runRemovePoly:) withObject:num afterDelay:sceneRep->fade];
            sceneRep->fade = 0.0;            
        } else {
            for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
                 idIt != sceneRep->drawIDs.end(); ++idIt)
                scene->addChangeRequest(new RemDrawableReq(*idIt));
            polyReps.erase(it);
        
            delete sceneRep;
        }
    }
}

// Add a lofted poly
- (WhirlyKit::SimpleIdentity) addLoftedPolys:(WhirlyKit::ShapeSet *)shapes desc:(NSDictionary *)desc cacheName:(NSString *)cacheName cacheHandler:(NSObject<WhirlyKitLoftedPolyCache> *)cacheHandler
{
    if (!layerThread || !scene)
    {
        NSLog(@"WhirlyGlobe Loft Layer has not been initialized, yet you're calling addLoftedPoly.  Dropping data on floor.");
        return EmptyIdentity;
    }

    LoftedPolyInfo *polyInfo = [[LoftedPolyInfo alloc] initWithShapes:shapes desc:desc key:([cacheName isKindOfClass:[NSNull class]] ? nil : cacheName)];
    polyInfo->cache = ([cacheHandler isKindOfClass:[NSNull class]] ? nil :cacheHandler);
    polyInfo->sceneRepId = Identifiable::genId();
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runAddPoly:polyInfo];
    else
        [self performSelector:@selector(runAddPoly:) onThread:layerThread withObject:polyInfo waitUntilDone:NO];
    
    return polyInfo->sceneRepId;
}

- (WhirlyKit::SimpleIdentity) addLoftedPoly:(WhirlyKit::VectorShapeRef)shape desc:(NSDictionary *)desc cacheName:(NSString *)cacheName cacheHandler:(NSObject<WhirlyKitLoftedPolyCache> *)cacheHandler
{
    ShapeSet shapes;
    shapes.insert(shape);
    
    return [self addLoftedPolys:&shapes desc:desc cacheName:(NSString *)cacheName cacheHandler:cacheHandler];
}

// Change how the lofted poly is represented
- (void)changeLoftedPoly:(SimpleIdentity)polyID desc:(NSDictionary *)desc
{
    LoftedPolyInfo *polyInfo = [[LoftedPolyInfo alloc] initWithSceneRepId:polyID desc:desc];
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runChangePoly:polyInfo];
    else
        [self performSelector:@selector(runChangePoly:) onThread:layerThread withObject:polyInfo waitUntilDone:NO];
}

// Remove the lofted poly
- (void)removeLoftedPoly:(SimpleIdentity)polyID
{
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runRemovePoly:[NSNumber numberWithInt:polyID]];
    else
        [self performSelector:@selector(runRemovePoly:) onThread:layerThread withObject:[NSNumber numberWithInt:polyID] waitUntilDone:NO];
}

@end
