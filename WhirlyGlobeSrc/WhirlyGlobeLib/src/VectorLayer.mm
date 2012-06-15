/*
 *  ShapeDisplay.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/26/11.
 *  Copyright 2011 mousebird consulting
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

#import "WhirlyGeometry.h"
#import "VectorLayer.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"
#import "RenderCache.h"

using namespace WhirlyKit;
//using namespace WhirlyGlobe;

// Used to describe the drawable we'll construct for a given vector
@interface VectorInfo : NSObject
{
@public
    // The scene representation (for vectors) we're referring to
    SimpleIdentity              sceneRepId;
    // For creation request, the shapes
    ShapeSet                    shapes;
    BOOL                        enable;
    int                         drawOffset;
    UIColor                     *color;
    int                         priority;
    float                       minVis,maxVis;
    float                       fade;
    NSString                    *cacheName;
}

@property (nonatomic) UIColor *color;
@property (nonatomic) NSString *cacheName;
@property (nonatomic,assign) float fade;

- (void)parseDict:(NSDictionary *)dict;

@end

@implementation VectorInfo

@synthesize color;
@synthesize cacheName;
@synthesize fade;

- (id)initWithShapes:(ShapeSet *)inShapes desc:(NSDictionary *)dict
{
    if ((self = [super init]))
    {
        if (inShapes)
            shapes = *inShapes;
        [self parseDict:dict];
    }
    
    return self;
}

- (id)initWithSceneRepId:(SimpleIdentity)inId desc:(NSDictionary *)dict
{
    if ((self = [super init]))
    {
        sceneRepId = inId;
        [self parseDict:dict];
    }
    
    return self;
}


- (void)parseDict:(NSDictionary *)dict
{
    enable = [dict boolForKey:@"enable" default:YES];
    drawOffset = [dict intForKey:@"drawOffset" default:1];
    self.color = [dict objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]];
    priority = [dict intForKey:@"drawPriority" default:0];
    // This looks like an old bug
    priority = [dict intForKey:@"priority" default:priority];
    minVis = [dict floatForKey:@"minVis" default:DrawVisibleInvalid];
    maxVis = [dict floatForKey:@"maxVis" default:DrawVisibleInvalid];
    fade = [dict floatForKey:@"fade" default:0.0];
}

@end

namespace WhirlyKit
{
    
/* Drawable Builder
    Used to construct drawables with multiple shapes in them.
    Eventually, we'll move this out to be a more generic object.
 */
class DrawableBuilder
{
public:
    DrawableBuilder(Scene *scene,VectorSceneRep *sceneRep,
                    VectorInfo *vecInfo,bool linesOrPoints,RenderCacheWriter *cacheWriter)
    : scene(scene), sceneRep(sceneRep), vecInfo(vecInfo), drawable(NULL), cacheWriter(cacheWriter)
    {
        primType = (linesOrPoints ? GL_LINES : GL_POINTS);
    }
    
    ~DrawableBuilder()
    {
        flush();
    }
        
    void addPoints(VectorRing &pts,bool closed)
    {          
//        CoordSystem *coordSys = scene->getCoordSystem();
        
        // Decide if we'll appending to an existing drawable or
        //  create a new one
        int ptCount = 2*(pts.size()+1);
        if (!drawable || (drawable->getNumPoints()+ptCount > MaxDrawablePoints))
        {
            // We're done with it, toss it to the scene
            if (drawable)
                flush();
                
            drawable = new BasicDrawable();
            drawMbr.reset();
            drawable->setType(primType);
            // Adjust according to the vector info
            drawable->setOnOff(vecInfo->enable);
            drawable->setDrawOffset(vecInfo->drawOffset);
            drawable->setColor([vecInfo.color asRGBAColor]);
            drawable->setDrawPriority(vecInfo->priority);
            drawable->setVisibleRange(vecInfo->minVis,vecInfo->maxVis);
        }
        drawMbr.addPoints(pts);
            
        Point3f prevPt,prevNorm,firstPt,firstNorm;
        for (unsigned int jj=0;jj<pts.size();jj++)
        {
            // Convert to real world coordinates and offset from the globe
            Point2f &geoPt = pts[jj];
            GeoCoord geoCoord = GeoCoord(geoPt.x(),geoPt.y());
            Point3f norm = GeoCoordSystem::LocalToGeocentricish(geoCoord);
            Point3f pt = norm;
            
            // Add to drawable
            // Depending on the type, we do this differently
            if (primType == GL_POINTS)
            {
                drawable->addPoint(pt);
                drawable->addNormal(norm);
            } else {
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
        }
        
        // Close the loop
        if (closed && primType == GL_LINES)
        {
            drawable->addPoint(prevPt);
            drawable->addPoint(firstPt);
            drawable->addNormal(prevNorm);
            drawable->addNormal(firstNorm);
        }
    }
    
    void flush()
    {
        if (drawable)
        {            
            if (drawable->getNumPoints() > 0)
            {
                drawable->setLocalMbr(drawMbr);
                sceneRep->drawIDs.insert(drawable->getId());

                // Save to the cache
                if (cacheWriter)
                    cacheWriter->addDrawable(drawable);

                if (vecInfo.fade > 0.0)
                {
                    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
                    drawable->setFade(curTime,curTime+vecInfo.fade);
                }
                scene->addChangeRequest(new AddDrawableReq(drawable));
            } else
                delete drawable;
            drawable = NULL;
        }
    }
    
protected:   
    Scene *scene;
    VectorSceneRep *sceneRep;
    Mbr drawMbr;
    BasicDrawable *drawable;
    VectorInfo *vecInfo;
    RenderCacheWriter *cacheWriter;
    GLenum primType;
};

}

@implementation WhirlyKitVectorLayer

- (void)clear
{
    for (VectorSceneRepMap::iterator it = vectorReps.begin();
         it != vectorReps.end(); ++it)
        delete it->second;
    vectorReps.clear();    
    
    scene = NULL;
}

- (void)dealloc
{
    [self clear];
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
	scene = inScene;
    layerThread = inLayerThread;
}

- (void)shutdown
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    std::vector<ChangeRequest *> changeRequests;
    
    for (VectorSceneRepMap::iterator it = vectorReps.begin();
         it != vectorReps.end(); ++it)
    {
        VectorSceneRep *sceneRep = it->second;
        for (SimpleIDSet::iterator sit = sceneRep->drawIDs.begin();
             sit != sceneRep->drawIDs.end(); ++sit)
            changeRequests.push_back(new RemDrawableReq(*sit));
    }
    scene->addChangeRequests(changeRequests);
    
    [self clear];
}

// Generate drawables.  We'll stack areas into as few drawables
//  as possible
- (void)runAddVector:(VectorInfo *)vecInfo
{
    VectorSceneRep *sceneRep = new VectorSceneRep(vecInfo->shapes);
    sceneRep->fade = vecInfo.fade;
    sceneRep->setId(vecInfo->sceneRepId);
    vectorReps[sceneRep->getId()] = sceneRep;
    
    // If we're writing out to a cache, set that up as well
    RenderCacheWriter *renderCacheWriter=NULL;
    if (vecInfo.cacheName)
        renderCacheWriter = new RenderCacheWriter(vecInfo.cacheName);
        
    // All the shape types should be the same
    ShapeSet::iterator first = vecInfo->shapes.begin();
    if (first == vecInfo->shapes.end())
        return;
    VectorPointsRef thePoints = boost::dynamic_pointer_cast<VectorPoints>(*first);
    bool linesOrPoints = (thePoints.get() ? false : true);
    
    // Used to toss out drawables as we go
    // Its destructor will flush out the last drawable
    DrawableBuilder drawBuild(scene,sceneRep,vecInfo,linesOrPoints,renderCacheWriter);
    
    for (ShapeSet::iterator it = vecInfo->shapes.begin();
         it != vecInfo->shapes.end(); ++it)
    {
        VectorArealRef theAreal = boost::dynamic_pointer_cast<VectorAreal>(*it);        
        if (theAreal.get())
        {
            // Work through the loops
            for (unsigned int ri=0;ri<theAreal->loops.size();ri++)
            {
                VectorRing &ring = theAreal->loops[ri];					

                drawBuild.addPoints(ring,true);
            }
        } else {
            VectorLinearRef theLinear = boost::dynamic_pointer_cast<VectorLinear>(*it);
            if (theLinear.get())
            {
                drawBuild.addPoints(theLinear->pts,false);
            } else {
                VectorPointsRef thePoints = boost::dynamic_pointer_cast<VectorPoints>(*it);
                if (thePoints.get())
                {
                    drawBuild.addPoints(thePoints->pts,false);
                }
            }
        }
    }    
    
    drawBuild.flush();
    if (renderCacheWriter)
        delete renderCacheWriter;
}

// Load the vector drawables from the cache
- (void)runAddVectorsFromCache:(VectorInfo *)vecInfo
{
    RenderCacheReader *renderCacheReader = new RenderCacheReader(vecInfo.cacheName);
    
    // Load in the textures and drawables
    // We'll hand them to the scene as we get them    
    SimpleIDSet texIDs,drawIDs;
    if (!renderCacheReader->getDrawablesAndTexturesAddToScene(scene,texIDs,drawIDs,vecInfo.fade))
        NSLog(@"VectorLayer failed to load from cache: %@",vecInfo.cacheName);
    else {
        VectorSceneRep *sceneRep = new VectorSceneRep(vecInfo->shapes);
        sceneRep->setId(vecInfo->sceneRepId);
        sceneRep->drawIDs = drawIDs;
        vectorReps[sceneRep->getId()] = sceneRep;        
    }
    
    delete renderCacheReader;
}

// Change a vector representation according to the request
// We'll change color, visible distance, or on/off
// Note: Changing them all, which is dumb
- (void)runChangeVector:(VectorInfo *)vecInfo
{
    VectorSceneRepMap::iterator it = vectorReps.find(vecInfo->sceneRepId);
    
    if (it != vectorReps.end())
    {    
        VectorSceneRep *sceneRep = it->second;
        
        for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
             idIt != sceneRep->drawIDs.end(); ++idIt)
        {
            // Turned it on or off
            scene->addChangeRequest(new OnOffChangeRequest(*idIt, vecInfo->enable));
    
            // Changed color
            RGBAColor newColor = [vecInfo.color asRGBAColor];
            scene->addChangeRequest(new ColorChangeRequest(*idIt, newColor));
            
            // Changed visibility
            scene->addChangeRequest(new VisibilityChangeRequest(*idIt, vecInfo->minVis, vecInfo->maxVis));
        }
    }
}

// Remove the vector (in the layer thread here)
- (void)runRemoveVector:(NSNumber *)num
{
    VectorSceneRepMap::iterator it = vectorReps.find((SimpleIdentity)[num intValue]);
    
    if (it != vectorReps.end())
    {
        VectorSceneRep *sceneRep = it->second;
    
        if (sceneRep->fade > 0.0)
        {
            NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
            for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
                 idIt != sceneRep->drawIDs.end(); ++idIt)
                scene->addChangeRequest(new FadeChangeRequest(*idIt,curTime,curTime+sceneRep->fade));                
            
            // Reset the fade and try to delete again later
            [self performSelector:@selector(runRemoveVector:) withObject:num afterDelay:sceneRep->fade];
            sceneRep->fade = 0.0;            
        } else {
            for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
                 idIt != sceneRep->drawIDs.end(); ++idIt)
                scene->addChangeRequest(new RemDrawableReq(*idIt));
            vectorReps.erase(it);
            
            delete sceneRep;
        }
    }    
}

// Add a vector
// We make up an ID for it before it's actually created
- (SimpleIdentity)addVector:(VectorShapeRef)shape desc:(NSDictionary *)dict
{
    ShapeSet shapes;
    shapes.insert(shape);
    return [self addVectors:&shapes desc:dict];
}

// Add a group of vectors and cache it to the given file, which might be on disk
- (SimpleIdentity)addVectors:(ShapeSet *)shapes desc:(NSDictionary *)desc cacheName:(NSString *)cacheName
{
    VectorInfo *vecInfo = [[VectorInfo alloc] initWithShapes:shapes desc:desc];
    vecInfo.cacheName = cacheName;
    vecInfo->sceneRepId = Identifiable::genId();
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runAddVector:vecInfo];
    else
        [self performSelector:@selector(runAddVector:) onThread:layerThread withObject:vecInfo waitUntilDone:NO];
    
    return vecInfo->sceneRepId;
}

// Add a group of vectors.  These will all be referred to by the same ID.
- (SimpleIdentity)addVectors:(ShapeSet *)shapes desc:(NSDictionary *)desc
{
    return [self addVectors:shapes desc:desc cacheName:nil];
}

// Load the drawables in from a cache
- (SimpleIdentity)addVectorsFromCache:(NSString *)cacheName
{
    VectorInfo *vecInfo = [[VectorInfo alloc] init];
    vecInfo.cacheName = cacheName;
    vecInfo->sceneRepId = Identifiable::genId();
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runAddVectorsFromCache:vecInfo];
    else
        [self performSelector:@selector(runAddVectorsFromCache:) onThread:layerThread withObject:vecInfo waitUntilDone:NO];
    
    return vecInfo->sceneRepId;
}

// Change how the vector is represented
- (void)changeVector:(SimpleIdentity)vecID desc:(NSDictionary *)dict
{
    VectorInfo *vecInfo = [[VectorInfo alloc] initWithSceneRepId:vecID desc:dict];
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runChangeVector:vecInfo];
    else
        [self performSelector:@selector(runChangeVector:) onThread:layerThread withObject:vecInfo waitUntilDone:NO];
}

// Remove the vector
- (void)removeVector:(SimpleIdentity)vecID
{
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runRemoveVector:[NSNumber numberWithInt:vecID]];
    else
        [self performSelector:@selector(runRemoveVector:) onThread:layerThread withObject:[NSNumber numberWithInt:vecID] waitUntilDone:NO];
}

// Return the cost of the given vector represenation
// Can only do this if the vectors(s) have been created, so only from the layer thread
- (WhirlyKitDrawCost *)getCost:(SimpleIdentity)vecId
{
    WhirlyKitDrawCost *cost = [[WhirlyKitDrawCost alloc] init];
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
    {
        VectorSceneRepMap::iterator it = vectorReps.find(vecId);
        
        if (it != vectorReps.end())
        {    
            VectorSceneRep *sceneRep = it->second;        
            // These were all created for this group of labels
            cost.numDrawables = sceneRep->drawIDs.size();
        }
    }
    
    return cost;
}

@end
