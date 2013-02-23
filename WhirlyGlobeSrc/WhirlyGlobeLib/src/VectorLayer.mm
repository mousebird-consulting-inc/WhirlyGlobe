/*
 *  VectorLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/26/11.
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

#import "WhirlyGeometry.h"
#import "VectorLayer.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"
#import "Tesselator.h"

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
    float                         drawOffset;
    UIColor                     *color;
    int                         priority;
    float                       minVis,maxVis;
    float                       fade;
    float                       lineWidth;
    BOOL                        filled;
    float                       sample;
    SimpleIdentity              replaceVecID;
}

@property (nonatomic) UIColor *color;
@property (nonatomic,assign) float fade;
@property (nonatomic,assign) float lineWidth;
@property (nonatomic,assign) SimpleIdentity replaceVecID;

- (void)parseDict:(NSDictionary *)dict;

@end

@implementation VectorInfo

@synthesize color;
@synthesize fade;
@synthesize lineWidth;
@synthesize replaceVecID;

- (id)initWithShapes:(ShapeSet *)inShapes desc:(NSDictionary *)dict
{
    if ((self = [super init]))
    {
        if (inShapes)
            shapes = *inShapes;
        replaceVecID = EmptyIdentity;
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
    drawOffset = [dict floatForKey:@"drawOffset" default:1];
    self.color = [dict objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]];
    priority = [dict intForKey:@"drawPriority" default:0];
    // This looks like an old bug
    priority = [dict intForKey:@"priority" default:priority];
    minVis = [dict floatForKey:@"minVis" default:DrawVisibleInvalid];
    maxVis = [dict floatForKey:@"maxVis" default:DrawVisibleInvalid];
    fade = [dict floatForKey:@"fade" default:0.0];
    lineWidth = [dict floatForKey:@"width" default:1.0];
    filled = [dict boolForKey:@"filled" default:false];
    sample = [dict floatForKey:@"sample" default:false];
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
    DrawableBuilder(Scene *scene,std::vector<ChangeRequest *> &changeRequests,VectorSceneRep *sceneRep,
                    VectorInfo *vecInfo,bool linesOrPoints)
    : changeRequests(changeRequests), scene(scene), sceneRep(sceneRep), vecInfo(vecInfo), drawable(NULL)
    {
        primType = (linesOrPoints ? GL_LINES : GL_POINTS);
    }
    
    ~DrawableBuilder()
    {
        flush();
    }
        
    void addPoints(VectorRing &pts,bool closed)
    {
        CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        
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
            drawable->setLineWidth(vecInfo.lineWidth);
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
            Point3f localPt = coordAdapter->getCoordSystem()->geographicToLocal(geoCoord);
            Point3f norm = coordAdapter->normalForLocal(localPt);
            Point3f pt = coordAdapter->localToDisplay(localPt);
            
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

                if (vecInfo.fade > 0.0)
                {
                    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
                    drawable->setFade(curTime,curTime+vecInfo.fade);
                }
                changeRequests.push_back(new AddDrawableReq(drawable));
            } else
                delete drawable;
            drawable = NULL;
        }
    }
    
protected:   
    Scene *scene;
    std::vector<ChangeRequest *> &changeRequests;
    VectorSceneRep *sceneRep;
    Mbr drawMbr;
    BasicDrawable *drawable;
    VectorInfo *vecInfo;
    GLenum primType;
};

/* Drawable Builder (Triangle version)
 Used to construct drawables with multiple shapes in them.
 Eventually, we'll move this out to be a more generic object.
 */
class DrawableBuilderTri
{
public:
    DrawableBuilderTri(Scene *scene,std::vector<ChangeRequest *> &changeRequests,VectorSceneRep *sceneRep,
                       VectorInfo *vecInfo)
    : changeRequests(changeRequests), scene(scene), sceneRep(sceneRep), vecInfo(vecInfo), drawable(NULL)
    {
    }
    
    ~DrawableBuilderTri()
    {
        flush();
    }
    
    void addPoints(VectorRing &inRing)
    {
        if (inRing.size() < 3)
            return;
        
        CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        
        std::vector<VectorRing> rings;
        TesselateRing(inRing,rings);
        
        for (unsigned int ir=0;ir<rings.size();ir++)
        {
            VectorRing &pts = rings[ir];
            // Decide if we'll appending to an existing drawable or
            //  create a new one
            int ptCount = pts.size();
            int triCount = pts.size()-2;
            if (!drawable || 
                (drawable->getNumPoints()+ptCount > MaxDrawablePoints) ||
                (drawable->getNumTris()+triCount > MaxDrawableTriangles))
            {
                // We're done with it, toss it to the scene
                if (drawable)
                    flush();
                
                drawable = new BasicDrawable();
                drawMbr.reset();
                drawable->setType(GL_TRIANGLES);
                // Adjust according to the vector info
                drawable->setOnOff(vecInfo->enable);
                drawable->setDrawOffset(vecInfo->drawOffset);
                drawable->setColor([vecInfo.color asRGBAColor]);
                drawable->setDrawPriority(vecInfo->priority);
                drawable->setVisibleRange(vecInfo->minVis,vecInfo->maxVis);
            }
            int baseVert = drawable->getNumPoints();
            drawMbr.addPoints(pts);
            
            // Add the points
            for (unsigned int jj=0;jj<pts.size();jj++)
            {
                // Convert to real world coordinates and offset from the globe
                Point2f &geoPt = pts[jj];
                GeoCoord geoCoord = GeoCoord(geoPt.x(),geoPt.y());
                Point3f localPt = coordAdapter->getCoordSystem()->geographicToLocal(geoCoord);
                Point3f norm = coordAdapter->normalForLocal(localPt);
                Point3f pt = coordAdapter->localToDisplay(localPt);
                
                drawable->addPoint(pt);
                drawable->addNormal(norm);
            }
            
            // Add the triangles
            // Note: Should be reusing vertex indices
            if (pts.size() == 3)
                drawable->addTriangle(BasicDrawable::Triangle(0+baseVert,2+baseVert,1+baseVert));
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
                
                if (vecInfo.fade > 0.0)
                {
                    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
                    drawable->setFade(curTime,curTime+vecInfo.fade);
                }
                changeRequests.push_back(new AddDrawableReq(drawable));
            } else
                delete drawable;
            drawable = NULL;
        }
    }
    
protected:   
    Scene *scene;
    std::vector<ChangeRequest *> &changeRequests;
    VectorSceneRep *sceneRep;
    Mbr drawMbr;
    BasicDrawable *drawable;
    VectorInfo *vecInfo;
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
            
    // All the shape types should be the same
    ShapeSet::iterator first = vecInfo->shapes.begin();
    if (first == vecInfo->shapes.end())
        return;
    VectorPointsRef thePoints = boost::dynamic_pointer_cast<VectorPoints>(*first);
    bool linesOrPoints = (thePoints.get() ? false : true);
    
    // Used to toss out drawables as we go
    // Its destructor will flush out the last drawable
    std::vector<ChangeRequest *> changeRequests;
    DrawableBuilder drawBuild(scene,changeRequests,sceneRep,vecInfo,linesOrPoints);
    DrawableBuilderTri drawBuildTri(scene,changeRequests,sceneRep,vecInfo);
    
    // Note: This is a duplicate of the runRemoveVector logic
    if (vecInfo.replaceVecID)
    {
        VectorSceneRepMap::iterator it = vectorReps.find(vecInfo.replaceVecID);
        
        if (it != vectorReps.end())
        {
            VectorSceneRep *sceneRep = it->second;
            
            for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
                 idIt != sceneRep->drawIDs.end(); ++idIt)
                changeRequests.push_back(new RemDrawableReq(*idIt));
            vectorReps.erase(it);
                
            delete sceneRep;
        }
    }
    
    for (ShapeSet::iterator it = vecInfo->shapes.begin();
         it != vecInfo->shapes.end(); ++it)
    {
        VectorArealRef theAreal = boost::dynamic_pointer_cast<VectorAreal>(*it);        
        if (theAreal.get())
        {
            if (vecInfo->filled)
            {
                // Triangulate the outside
                drawBuildTri.addPoints(theAreal->loops[0]);
            } else {
                // Work through the loops
                for (unsigned int ri=0;ri<theAreal->loops.size();ri++)
                {
                    VectorRing &ring = theAreal->loops[ri];

                    // Break the edges around the globe (presumably)
                    if (vecInfo->sample > 0.0)
                    {
                        VectorRing newPts;
                        SubdivideEdges(ring, newPts, false, vecInfo->sample);
                        drawBuild.addPoints(newPts,true);
                    } else
                        drawBuild.addPoints(ring,true);
                }
            }
        } else {
            VectorLinearRef theLinear = boost::dynamic_pointer_cast<VectorLinear>(*it);
            if (vecInfo->filled)
            {
                // Triangulate the outside
                drawBuildTri.addPoints(theLinear->pts);
            } else {
                if (theLinear.get())
                {
                    if (vecInfo->sample > 0.0)
                    {
                        VectorRing newPts;
                        SubdivideEdges(theLinear->pts, newPts, false, vecInfo->sample);
                        drawBuild.addPoints(newPts,false);
                    } else
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
    }    
    
    drawBuild.flush();
    drawBuildTri.flush();
    
    scene->addChangeRequests(changeRequests);    
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
            
            // Changed line width
            scene->addChangeRequest(new LineWidthChangeRequest(*idIt, vecInfo->lineWidth));
            
            // Changed draw priority
            scene->addChangeRequest(new DrawPriorityChangeRequest(*idIt, vecInfo->priority));
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

// Replace the given set of vectors wit the new one
- (WhirlyKit::SimpleIdentity)replaceVector:(WhirlyKit::SimpleIdentity)oldVecID withVectors:(WhirlyKit::ShapeSet *)shapes desc:(NSDictionary *)dict
{
    if (!layerThread || !scene)
    {
        NSLog(@"WhirlyGlobe Vector layer has not been initialized, yet you're calling replaceVector.  Dropping data on floor.");
        return EmptyIdentity;
    }
    
    VectorInfo *vecInfo = [[VectorInfo alloc] initWithShapes:shapes desc:dict];
    vecInfo.replaceVecID = oldVecID;
    vecInfo->sceneRepId = Identifiable::genId();
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runAddVector:vecInfo];
    else
        [self performSelector:@selector(runAddVector:) onThread:layerThread withObject:vecInfo waitUntilDone:NO];
    
    return vecInfo->sceneRepId;    
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
- (SimpleIdentity)addVectors:(ShapeSet *)shapes desc:(NSDictionary *)desc
{
    if (!layerThread || !scene)
    {
        NSLog(@"WhirlyGlobe Vector layer has not been initialized, yet you're calling addVectors.  Dropping data on floor.");
        return EmptyIdentity;
    }
    
    VectorInfo *vecInfo = [[VectorInfo alloc] initWithShapes:shapes desc:desc];
    vecInfo->sceneRepId = Identifiable::genId();
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runAddVector:vecInfo];
    else
        [self performSelector:@selector(runAddVector:) onThread:layerThread withObject:vecInfo waitUntilDone:NO];
    
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
