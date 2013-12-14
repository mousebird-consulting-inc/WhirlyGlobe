/*
 *  LoftManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/30/13.
 *  Copyright 2011-2013 mousebird consulting. All rights reserved.
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

#import "LoftManager.h"
#import "VectorData.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"
#import "GridClipper.h"
#import "Tesselator.h"

using namespace Eigen;
using namespace WhirlyKit;

// Used to describe the drawables we want to construct for a given vector
@interface WhirlyKitLoftedPolyInfo : NSObject
{
@public
    SimpleIdentity sceneRepId;
    // For a creation request
    ShapeSet    shapes;
    float       height;
    float       base;
    float       minVis,maxVis;
    int         priority;
    bool        top,side;
    bool        layered;
    bool        outline;
    UIColor     *outlineColor;
    float       outlineWidth;
    bool        readZBuffer;
    bool        writeZBuffer;
    bool        enable;
    NSObject<WhirlyKitLoftedPolyCache> *cache;
}

@property (nonatomic) UIColor *color;
@property (nonatomic) NSString *key;
@property (nonatomic,assign) float fade;

- (void)parseDesc:(NSDictionary *)desc key:(NSString *)key;

@end

@implementation WhirlyKitLoftedPolyInfo

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
    priority = [dict intForKey:@"drawPriority" default:0];
    priority = [dict intForKey:@"priority" default:priority];
    height = [dict floatForKey:@"height" default:.01];
    base = [dict floatForKey:@"base" default:0.0];
    minVis = [dict floatForKey:@"minVis" default:DrawVisibleInvalid];
    maxVis = [dict floatForKey:@"maxVis" default:DrawVisibleInvalid];
    _fade = [dict floatForKey:@"fade" default:0.0];
    top = [dict boolForKey:@"top" default:true];
    side = [dict boolForKey:@"side" default:true];
    layered = [dict boolForKey:@"layered" default:false];
    outline = [dict boolForKey:@"outline" default:false];
    outlineColor = [dict objectForKey:@"outlineColor" checkType:[UIColor class] default:[UIColor whiteColor]];
    outlineWidth = [dict floatForKey:@"outlineWidth" default:1.0];
    readZBuffer = [dict boolForKey:@"zbufferread" default:YES];
    writeZBuffer = [dict boolForKey:@"zbufferwrite" default:NO];
    enable = [dict boolForKey:@"enable" default:true];
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
    
    // Note: Cache no longer working
    return false;
    
//    NSData *data = [cache readLoftedPolyData:key];
//    if (!data)
//        return false;
//    
//    try {
//        // MBR first
//        float ll_x,ll_y,ur_x,ur_y;
//        unsigned int loc = 0;
//        [data getBytes:&ll_x range:NSMakeRange(loc, sizeof(float))];  loc += sizeof(float);  if (loc > [data length])  throw 1;
//        [data getBytes:&ll_y range:NSMakeRange(loc, sizeof(float))];  loc += sizeof(float);  if (loc > [data length])  throw 1;
//        [data getBytes:&ur_x range:NSMakeRange(loc, sizeof(float))];  loc += sizeof(float);  if (loc > [data length])  throw 1;
//        [data getBytes:&ur_y range:NSMakeRange(loc, sizeof(float))];  loc += sizeof(float);  if (loc > [data length])  throw 1;
//        shapeMbr.addGeoCoord(GeoCoord(ll_x,ll_y));
//        shapeMbr.addGeoCoord(GeoCoord(ur_x,ur_y));
//        
//        // Triangle meshes
//        unsigned int numMesh = 0;
//        [data getBytes:&numMesh range:NSMakeRange(loc, sizeof(unsigned int))];  loc += sizeof(unsigned int);  if (loc > [data length])  throw 1;
//        triMesh.resize(numMesh);
//        for (unsigned int ii=0;ii<numMesh;ii++)
//        {
//            VectorRing &ring = triMesh[ii];
//            unsigned int numPt = 0;
//            [data getBytes:&numPt range:NSMakeRange(loc, sizeof(unsigned int))];  loc += sizeof(unsigned int);  if (loc > [data length])  throw 1;
//            ring.resize(numPt);
//            for (unsigned int jj=0;jj<numPt;jj++)
//            {
//                Point2f &pt = ring[jj];
//                float x,y;
//                [data getBytes:&x range:NSMakeRange(loc, sizeof(float))];  loc += sizeof(float);  if (loc > [data length])  throw 1;
//                [data getBytes:&y range:NSMakeRange(loc, sizeof(float))];  loc += sizeof(float);  if (loc > [data length])  throw 1;
//                pt.x() = x;
//                pt.y() = y;
//            }
//        }
//    }
//    catch (...)
//    {
//        return false;
//    }
    
    return true;
}

// Write the lofted poly representation to a cache
// Just the MBR and triangle mesh
bool LoftedPolySceneRep::writeToCache(NSObject<WhirlyKitLoftedPolyCache> *cache,NSString *key)
{
    // Note: Cache no longer working
    return false;
    
//    NSMutableData *data = [NSMutableData dataWithCapacity:0];
//    
//    // MBR first
//    GeoCoord ll = shapeMbr.ll(), ur = shapeMbr.ur();
//    [data appendBytes:&ll.x() length:sizeof(float)];
//    [data appendBytes:&ll.y() length:sizeof(float)];
//    [data appendBytes:&ur.x() length:sizeof(float)];
//    [data appendBytes:&ur.y() length:sizeof(float)];
//    
//    // Triangle meshes
//    unsigned int numMesh = triMesh.size();
//    [data appendBytes:&numMesh length:sizeof(unsigned int)];
//    for (unsigned int ii=0;ii<numMesh;ii++)
//    {
//        VectorRing &ring = triMesh[ii];
//        unsigned int numPt = ring.size();
//        [data appendBytes:&numPt length:sizeof(unsigned int)];
//        for (unsigned int jj=0;jj<numPt;jj++)
//        {
//            Point2f &pt = ring[jj];
//            [data appendBytes:&pt.x() length:sizeof(float)];
//            [data appendBytes:&pt.y() length:sizeof(float)];
//        }
//    }
//    
//    return [cache writeLoftedPolyData:data cacheName:key];
}

/* Drawable Builder
 Used to construct drawables with multiple shapes in them.
 Eventually, will move this out to be a more generic object.
 */
class DrawableBuilder2
{
public:
    DrawableBuilder2(Scene *scene,ChangeSet &changes,LoftedPolySceneRep *sceneRep,
                     WhirlyKitLoftedPolyInfo *polyInfo,int primType,const GeoMbr &inDrawMbr)
    : scene(scene), sceneRep(sceneRep), polyInfo(polyInfo), drawable(NULL), primType(primType), changes(changes)
    {
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
            
            drawable = new BasicDrawable("Lofted Poly");
            drawable->setType(primType);
            // Adjust according to the vector info
            //            drawable->setOnOff(polyInfo->enable);
            //            drawable->setDrawOffset(vecInfo->drawOffset);
            drawable->setColor([((primType == GL_TRIANGLES) ? polyInfo.color : polyInfo->outlineColor) asRGBAColor]);
            if (primType == GL_LINES)
                drawable->setLineWidth(polyInfo->outlineWidth);
            drawable->setDrawPriority(polyInfo->priority);
            drawable->setRequestZBuffer(polyInfo->readZBuffer);
            drawable->setWriteZBuffer(polyInfo->writeZBuffer);
            drawable->setVisibleRange(polyInfo->minVis,polyInfo->maxVis);
        }
    }
    
    // Add a triangle, keeping track of limits
    void addLoftTriangle(Point2f verts[3],float height)
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
            Point3f pt1 = dispPt + norm * height;
            
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
    void addPolyGroup(VectorTrianglesRef mesh)
    {
        for (unsigned int ii=0;ii<mesh->tris.size();ii++)
        {
            VectorRing tri;
            mesh->getTriangle(ii, tri);
            if (tri.size() == 3)
            {
                Point2f verts[3];
                verts[2] = tri[0];  verts[1] = tri[1];  verts[0] = tri[2];
                addLoftTriangle(verts,polyInfo->height);
                // If they've got a base, we want to see it from the underside, probably
                if (polyInfo->base > 0.0)
                {
                    Point2f verts[3];
                    verts[1] = tri[0];  verts[2] = tri[1];  verts[0] = tri[2];
                    addLoftTriangle(verts,polyInfo->base);                    
                }
            }
        }
    }
    
    // Add a set of outlines
    void addOutline(std::vector<VectorRing> &rings)
    {
        if (primType != GL_LINES)
            return;
        
        setupDrawable(0);
        CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        
        for (unsigned int ii=0;ii<rings.size();ii++)
        {
            VectorRing &verts = rings[ii];
            Point3f prevPt,prevNorm,firstPt,firstNorm;
            for (unsigned int jj=0;jj<verts.size();jj++)
            {
                // Convert to real world coordinates and offset from the globe
                Point2f &geoPt = verts[jj];
                GeoCoord geoCoord = GeoCoord(geoPt.x(),geoPt.y());
                Point3f localPt = coordAdapter->getCoordSystem()->geographicToLocal(geoCoord);
                Point3f norm = coordAdapter->normalForLocal(localPt);
                Point3f pt = coordAdapter->localToDisplay(localPt);
                pt += norm * polyInfo->height;
                
                // Add to drawable
                // Depending on the type, we do this differently
                if (jj > 0)
                {
                    drawable->addPoint(prevPt);
                    drawable->addPoint(pt);
                    drawable->addNormal(prevNorm);
                    drawable->addNormal(norm);
                } else {
                    firstPt = pt;
                    firstNorm = norm;
                }
                prevPt = pt;
                prevNorm = norm;
            }
            
            // Close the loop
            {
                drawable->addPoint(prevPt);
                drawable->addPoint(firstPt);
                drawable->addNormal(prevNorm);
                drawable->addNormal(firstNorm);
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
            if (polyInfo->base > 0.0)
                pt0 = pt0 + norm * polyInfo->base;
            
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
                crossNorm.normalize();
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
                changes.push_back(new AddDrawableReq(drawable));
            } else
                delete drawable;
            drawable = NULL;
        }
    }
    
protected:   
    Scene *scene;
    LoftedPolySceneRep *sceneRep;
    ChangeSet &changes;
    GeoMbr drawMbr;
    BasicDrawable *drawable;
    WhirlyKitLoftedPolyInfo *polyInfo;
    GLenum primType;
};
    
    
LoftManager::LoftManager()
{
    pthread_mutex_init(&loftLock, NULL);
}
LoftManager::~LoftManager()
{
    pthread_mutex_destroy(&loftLock);
    
    for (LoftedPolySceneRepSet::iterator it = loftReps.begin();
         it != loftReps.end(); ++it)
        delete *it;
    loftReps.clear();
}
    
// From a scene rep and a description, add the given polygons to the drawable builder
void LoftManager::addGeometryToBuilder(LoftedPolySceneRep *sceneRep,WhirlyKitLoftedPolyInfo *polyInfo,GeoMbr &drawMbr,ChangeSet &changes)
{
    int numShapes = 0;
    
    // Used to toss out drawables as we go
    // Its destructor will flush out the last drawable
    DrawableBuilder2 drawBuild(scene,changes,sceneRep,polyInfo,GL_TRIANGLES,drawMbr);
    
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
        
        // And do the top outline if it's there
        if (polyInfo->top && polyInfo->outline)
        {
            DrawableBuilder2 drawBuild2(scene,changes,sceneRep,polyInfo,GL_LINES,drawMbr);
            drawBuild2.addOutline(sceneRep->outlines);
            sceneRep->outlines.clear();
        }
    
    //    printf("Added %d shapes and %d triangles from mesh\n",(int)numShapes,(int)sceneRep->triMesh.size());        
}

    
/// Add lofted polygons
SimpleIdentity LoftManager::addLoftedPolys(WhirlyKit::ShapeSet *shapes,NSDictionary *desc,NSString *cacheName,NSObject<WhirlyKitLoftedPolyCache> *cacheHandler,float gridSize,ChangeSet &changes)
{
    WhirlyKitLoftedPolyInfo *polyInfo = [[WhirlyKitLoftedPolyInfo alloc] initWithShapes:shapes desc:desc key:([cacheName isKindOfClass:[NSNull class]] ? nil : cacheName)];
    polyInfo->cache = ([cacheHandler isKindOfClass:[NSNull class]] ? nil :cacheHandler);
    polyInfo->sceneRepId = Identifiable::genId();
    
    SimpleIdentity loftID = EmptyIdentity;

    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    LoftedPolySceneRep *sceneRep = new LoftedPolySceneRep();
    sceneRep->setId(polyInfo->sceneRepId);
    loftID = polyInfo->sceneRepId;
    sceneRep->fade = polyInfo.fade;    
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
                        
                        // May need to add the outline as well
                        if (polyInfo->outline)
                            sceneRep->outlines.push_back(ring);
                        
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
    
    addGeometryToBuilder(sceneRep, polyInfo, sceneRep->shapeMbr, changes);
    
    pthread_mutex_lock(&loftLock);

    loftReps.insert(sceneRep);
    
    pthread_mutex_unlock(&loftLock);
    
    return loftID;
}

/// Enable/disable lofted polys
void LoftManager::enableLoftedPolys(SimpleIDSet &polyIDs,bool enable,ChangeSet &changes)
{
    pthread_mutex_lock(&loftLock);
    
    for (SimpleIDSet::iterator idIt = polyIDs.begin(); idIt != polyIDs.end(); ++idIt)
    {
        LoftedPolySceneRep dummyRep(*idIt);
        LoftedPolySceneRepSet::iterator it = loftReps.find(&dummyRep);
        if (it != loftReps.end())
        {
            LoftedPolySceneRep *sceneRep = *it;
            for (SimpleIDSet::iterator dIt = sceneRep->drawIDs.begin();
                 dIt != sceneRep->drawIDs.end(); ++dIt)
                changes.push_back(new OnOffChangeRequest(*dIt,enable));
        }
    }

    pthread_mutex_unlock(&loftLock);
}

/// Remove lofted polygons
void LoftManager::removeLoftedPolys(SimpleIDSet &polyIDs,ChangeSet &changes)
{
    pthread_mutex_lock(&loftLock);
    
    for (SimpleIDSet::iterator idIt = polyIDs.begin(); idIt != polyIDs.end(); ++idIt)
    {
        LoftedPolySceneRep dummyRep(*idIt);
        LoftedPolySceneRepSet::iterator it = loftReps.find(&dummyRep);
        if (it != loftReps.end())
        {
            LoftedPolySceneRep *sceneRep = *it;
            
            if (sceneRep->fade > 0.0)
            {
                NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
                                
                for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
                     idIt != sceneRep->drawIDs.end(); ++idIt)
                    changes.push_back(new FadeChangeRequest(*idIt,curTime,curTime+sceneRep->fade));
                
                // Kick off the delete in a bit
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, sceneRep->fade * NSEC_PER_SEC),
                               scene->getDispatchQueue(),
                               ^{
                                   SimpleIDSet theIDs;
                                   theIDs.insert(sceneRep->getId());
                                   ChangeSet delChanges;
                                   removeLoftedPolys(theIDs, delChanges);
                                   scene->addChangeRequests(delChanges);
                               }
                               );
                sceneRep->fade = 0.0;
            } else {
                for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
                     idIt != sceneRep->drawIDs.end(); ++idIt)
                    changes.push_back(new RemDrawableReq(*idIt));
                loftReps.erase(it);
                
                delete sceneRep;
            }
        }
    }
    
    pthread_mutex_unlock(&loftLock);
}
    
}
