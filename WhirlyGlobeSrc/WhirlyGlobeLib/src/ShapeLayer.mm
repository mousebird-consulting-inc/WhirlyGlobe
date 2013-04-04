/*
 *  ShapeLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/28/11.
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

#import "ShapeLayer.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"
#import "GlobeMath.h"
#import "VectorData.h"

using namespace WhirlyKit;

// Used to pass shape information between threads
@interface ShapeInfo : NSObject
{
    NSArray         *shapes;  // Individual shape objects
    UIColor         *color;
    int             drawOffset;
    float           minVis,maxVis;
    int             drawPriority;
    float           fade;
    float           lineWidth;
    SimpleIdentity  shapeId;
}

@property (nonatomic) NSArray *shapes;
@property (nonatomic) UIColor *color;
@property (nonatomic,assign) int drawOffset;
@property (nonatomic,assign) float minVis,maxVis;
@property (nonatomic,assign) int drawPriority;
@property (nonatomic,assign) float fade;
@property (nonatomic,assign) float lineWidth;
@property (nonatomic,assign) SimpleIdentity shapeId;

- (id)initWithShapes:(NSArray *)shapes desc:(NSDictionary *)desc;

- (void)parseDesc:(NSDictionary *)desc;

@end

namespace WhirlyKit
{

ShapeSceneRep::ShapeSceneRep()
{
}
    
ShapeSceneRep::ShapeSceneRep(SimpleIdentity inId)
: Identifiable(inId)
{    
}
    
ShapeSceneRep::~ShapeSceneRep()
{
}
    
void ShapeSceneRep::clearContents(WhirlyKitSelectionLayer *selectLayer,std::vector<ChangeRequest *> &changeRequests)
{
    for (SimpleIDSet::iterator idIt = drawIDs.begin();
         idIt != drawIDs.end(); ++idIt)
        changeRequests.push_back(new RemDrawableReq(*idIt));
    if (selectLayer && selectID != EmptyIdentity)
        [selectLayer removeSelectable:selectID];
}
    
/* Drawable Builder
   Used to construct drawables with multiple shapes in them.
   Eventually, we'll move this out to be a more generic object.
 */
class ShapeDrawableBuilder
{
public:
    ShapeDrawableBuilder(Scene *scene,std::vector<ChangeRequest *> &changeRequests,ShapeSceneRep *sceneRep,
                    ShapeInfo *shapeInfo,bool linesOrPoints)
    : changeRequests(changeRequests), scene(scene), sceneRep(sceneRep), shapeInfo(shapeInfo), drawable(NULL)
    {
        primType = (linesOrPoints ? GL_LINES : GL_POINTS);
    }
    
    ~ShapeDrawableBuilder()
    {
        flush();
    }
    
    void addPoints(std::vector<Point3f> &pts,RGBAColor color,Mbr mbr,float lineWidth,bool closed)
    {
        CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        
        // Decide if we'll appending to an existing drawable or
        //  create a new one
        int ptCount = 2*(pts.size()+1);
        if (!drawable || (drawable->getNumPoints()+ptCount > MaxDrawablePoints) || (drawable->getLineWidth() != lineWidth))
        {
            // We're done with it, toss it to the scene
            if (drawable)
                flush();
            
            drawable = new BasicDrawable();
            drawMbr.reset();
            drawable->setType(primType);
            // Adjust according to the vector info
            drawable->setDrawOffset(shapeInfo.drawOffset);
//            drawable->setColor([shapeInfo.color asRGBAColor]);
            drawable->setLineWidth(lineWidth);
            drawable->setDrawPriority(shapeInfo.drawPriority);
            drawable->setVisibleRange(shapeInfo.minVis,shapeInfo.maxVis);
            drawable->setForceZBufferOn(true);
        }
        drawMbr.expand(mbr);
        
        Point3f prevPt,prevNorm,firstPt,firstNorm;
        for (unsigned int jj=0;jj<pts.size();jj++)
        {
            // The point is already in display coordinates, so we have to project back
            Point3f pt = pts[jj];
            Point3f localPt = coordAdapter->displayToLocal(pt);
            Point3f norm = coordAdapter->normalForLocal(localPt);
            
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
                    drawable->addNormal(prevNorm);
                    drawable->addColor(color);
                    drawable->addPoint(pt);
                    drawable->addNormal(norm);
                    drawable->addColor(color);
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
            drawable->addNormal(prevNorm);
            drawable->addColor(color);
            drawable->addPoint(firstPt);
            drawable->addNormal(firstNorm);
            drawable->addColor(color);
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
                                
                if (shapeInfo.fade > 0.0)
                {
                    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
                    drawable->setFade(curTime,curTime+shapeInfo.fade);
                }
                changeRequests.push_back(new AddDrawableReq(drawable));
            } else
                delete drawable;
            drawable = NULL;
        }
    }
    
public:
    Scene *scene;
    std::vector<ChangeRequest *> &changeRequests;
    ShapeSceneRep *sceneRep;
    Mbr drawMbr;
    BasicDrawable *drawable;
    ShapeInfo *shapeInfo;
    GLenum primType;
};
    
/* Drawable Builder (Triangle version)
 Used to construct drawables with multiple shapes in them.
 Eventually, we'll move this out to be a more generic object.
 */
class ShapeDrawableBuilderTri
{
public:
    ShapeDrawableBuilderTri(Scene *scene,std::vector<ChangeRequest *> &changeRequests,ShapeSceneRep *sceneRep,
                       ShapeInfo *shapeInfo)
    : changeRequests(changeRequests), scene(scene), sceneRep(sceneRep), shapeInfo(shapeInfo), drawable(NULL)
    {
    }
    
    ~ShapeDrawableBuilderTri()
    {
        flush();
    }
    
    void setupNewDrawable()
    {
        
        drawable = new BasicDrawable();
        drawMbr.reset();
        drawable->setType(GL_TRIANGLES);
        // Adjust according to the vector info
        drawable->setDrawOffset(shapeInfo.drawOffset);
//        drawable->setColor([shapeInfo.color asRGBAColor]);
        drawable->setDrawPriority(shapeInfo.drawPriority);
        drawable->setVisibleRange(shapeInfo.minVis,shapeInfo.maxVis);
        drawable->setForceZBufferOn(true);
    }
    
    // Add a triangle with normals
    void addTriangle(Point3f p0,Point3f n0,RGBAColor c0,Point3f p1,Point3f n1,RGBAColor c1,Point3f p2,Point3f n2,RGBAColor c2,Mbr shapeMbr)
    {
        if (!drawable ||
            (drawable->getNumPoints()+3 > MaxDrawablePoints) ||
            (drawable->getNumTris()+1 > MaxDrawableTriangles))
        {
            // We're done with it, toss it to the scene
            if (drawable)
                flush();
            
            setupNewDrawable();
        }
        Mbr mbr = drawable->getLocalMbr();
        mbr.expand(shapeMbr);
        drawable->setLocalMbr(mbr);
        int baseVert = drawable->getNumPoints();
        drawable->addPoint(p0);
        drawable->addNormal(n0);
        drawable->addColor(c0);
        drawable->addPoint(p1);
        drawable->addNormal(n1);
        drawable->addColor(c1);
        drawable->addPoint(p2);
        drawable->addNormal(n2);
        drawable->addColor(c2);
        
        drawable->addTriangle(BasicDrawable::Triangle(0+baseVert,2+baseVert,1+baseVert));
        drawMbr.expand(shapeMbr);
    }
    
    // Add a group of pre-build triangles
    void addTriangles(std::vector<Point3f> &pts,std::vector<Point3f> &norms,std::vector<RGBAColor> &colors,std::vector<BasicDrawable::Triangle> &tris)
    {
        if (!drawable ||
            (drawable->getNumPoints()+pts.size() > MaxDrawablePoints) ||
            (drawable->getNumTris()+tris.size() > MaxDrawableTriangles))
        {
            if (drawable)
                flush();
            
            setupNewDrawable();
        }

        int baseVert = drawable->getNumPoints();
        for (unsigned int ii=0;ii<pts.size();ii++)
        {
            drawable->addPoint(pts[ii]);
            drawable->addNormal(norms[ii]);
            drawable->addColor(colors[ii]);
        }
        for (unsigned int ii=0;ii<tris.size();ii++)
        {
            BasicDrawable::Triangle tri = tris[ii];
            for (unsigned int jj=0;jj<3;jj++)
                tri.verts[jj] += baseVert;
            drawable->addTriangle(tri);
        }
    }
    
    // Add a convex outline, triangulated
    void addConvexOutline(std::vector<Point3f> &pts,Point3f norm,RGBAColor color,Mbr shapeMbr)
    {
        // It's convex, so we'll just triangulate it dumb style
        for (unsigned int ii = 2;ii<pts.size();ii++)
            addTriangle(pts[0], norm, color, pts[ii-1], norm, color, pts[ii], norm, color, shapeMbr);
    }
    
    void flush()
    {
        if (drawable)
        {
            if (drawable->getNumPoints() > 0)
            {
                drawable->setLocalMbr(drawMbr);
                sceneRep->drawIDs.insert(drawable->getId());
                
                if (shapeInfo.fade > 0.0)
                {
                    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
                    drawable->setFade(curTime,curTime+shapeInfo.fade);
                }
                changeRequests.push_back(new AddDrawableReq(drawable));
            } else
                delete drawable;
            drawable = NULL;
        }
    }
    
public:
    Scene *scene;
    std::vector<ChangeRequest *> &changeRequests;
    ShapeSceneRep *sceneRep;
    Mbr drawMbr;
    BasicDrawable *drawable;
    ShapeInfo *shapeInfo;
};

}

@interface WhirlyKitShape()


@end

@interface WhirlyKitShape()

- (void)makeGeometryWithBuilder:(WhirlyKit::ShapeDrawableBuilder *)regBuilder triBuilder:(WhirlyKit::ShapeDrawableBuilderTri *)triBuilder scene:(WhirlyKit::Scene *)scene;

@end

@implementation WhirlyKitShape

@synthesize isSelectable;
@synthesize selectID;
@synthesize useColor;
@synthesize color;

// Base shape doesn't make anything
- (void)makeGeometryWithBuilder:(WhirlyKit::ShapeDrawableBuilder *)regBuilder triBuilder:(WhirlyKit::ShapeDrawableBuilderTri *)triBuilder scene:(WhirlyKit::Scene *)scene;
{
}

@end

// Number of samples for a circle.
// Note: Make this a parameter
static int CircleSamples = 20;

@implementation WhirlyKitCircle

@synthesize loc;
@synthesize radius;
@synthesize height;

// Build the geometry for a circle in display space
- (void)makeGeometryWithBuilder:(WhirlyKit::ShapeDrawableBuilder *)regBuilder triBuilder:(WhirlyKit::ShapeDrawableBuilderTri *)triBuilder scene:(WhirlyKit::Scene *)scene;
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    
    RGBAColor theColor = (useColor ? color : [regBuilder->shapeInfo.color asRGBAColor]);
    
    Point3f localPt = coordAdapter->getCoordSystem()->geographicToLocal(loc);
    Point3f dispPt = coordAdapter->localToDisplay(localPt);
    dispPt += coordAdapter->normalForLocal(localPt) * height;
    Point3f norm = coordAdapter->normalForLocal(localPt);
    
    // Construct a set of axes to build the circle around
    Point3f up = norm;
    Point3f xAxis,yAxis;
    if (coordAdapter->isFlat())
    {
        xAxis = Point3f(1,0,0);
        yAxis = Point3f(0,1,0);
    } else {
        Point3f north(0,0,1);
        // Note: Also check if we're at a pole
        xAxis = north.cross(up);  xAxis.normalize();
        yAxis = up.cross(xAxis);  yAxis.normalize();
    }
        
    // Calculate the locations, using the axis from the center
    std::vector<Point3f> samples;
    samples.resize(CircleSamples);
    for (unsigned int ii=0;ii<CircleSamples;ii++)
        samples[ii] =  xAxis * radius * sinf(2*M_PI*ii/(float)(CircleSamples-1)) + radius * yAxis * cosf(2*M_PI*ii/(float)(CircleSamples-1)) + dispPt;
    
    // We need the bounding box in the local coordinate system
    Mbr shapeMbr;
    for (unsigned int ii=0;ii<samples.size();ii++)
    {
        Point3f thisLocalPt = coordAdapter->displayToLocal(samples[ii]);
        // Note: If this shape has height, this is insufficient
        shapeMbr.addPoint(Point2f(thisLocalPt.x(),thisLocalPt.y()));
    }
    
    triBuilder->addConvexOutline(samples,norm,theColor,shapeMbr);
}

@end

@implementation WhirlyKitSphere

@synthesize loc;
@synthesize height;
@synthesize radius;

// Note: We could make these parameters
static const float SphereTessX = 10;
static const float SphereTessY = 10;

- (void)makeGeometryWithBuilder:(WhirlyKit::ShapeDrawableBuilder *)regBuilder triBuilder:(WhirlyKit::ShapeDrawableBuilderTri *)triBuilder scene:(WhirlyKit::Scene *)scene
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();

    RGBAColor theColor = (useColor ? color : [regBuilder->shapeInfo.color asRGBAColor]);

    // Get the location in display coordinates
    Point3f localPt = coordAdapter->getCoordSystem()->geographicToLocal(loc);
    Point3f dispPt = coordAdapter->localToDisplay(localPt);
    Point3f norm = coordAdapter->normalForLocal(localPt);
    
    // Run it up a bit by the height
    dispPt = dispPt + norm*height;
    
    // It's lame, but we'll use lat/lon coordinates to tesselate the sphere
    // Note: Replace this with something less lame
    std::vector<Point3f> locs,norms;
    locs.reserve((SphereTessX+1)*(SphereTessX+1));
    norms.reserve((SphereTessX+1)*(SphereTessY+1));
    std::vector<RGBAColor> colors;
    colors.reserve((SphereTessX+1)*(SphereTessX+1));
    Point2f geoIncr(2*M_PI/SphereTessX,M_PI/SphereTessY);
    for (unsigned int iy=0;iy<SphereTessY+1;iy++)
        for (unsigned int ix=0;ix<SphereTessX+1;ix++)
        {
            GeoCoord geoLoc(-M_PI+ix*geoIncr.x(),-M_PI/2.0 + iy*geoIncr.y());
			if (geoLoc.x() < -M_PI)  geoLoc.x() = -M_PI;
			if (geoLoc.x() > M_PI) geoLoc.x() = M_PI;
			if (geoLoc.y() < -M_PI/2.0)  geoLoc.y() = -M_PI/2.0;
			if (geoLoc.y() > M_PI/2.0) geoLoc.y() = M_PI/2.0;
            
            Point3f spherePt = FakeGeocentricDisplayAdapter::LocalToDisplay(Point3f(geoLoc.lon(),geoLoc.lat(),0.0));
            Point3f thisPt = dispPt + spherePt * radius;
            
            norms.push_back(spherePt);
            locs.push_back(thisPt);
            colors.push_back(theColor);
        }
    
    // Two triangles per cell
    std::vector<BasicDrawable::Triangle> tris;
    tris.reserve(2*SphereTessX*SphereTessY);
    for (unsigned int iy=0;iy<SphereTessY;iy++)
        for (unsigned int ix=0;ix<SphereTessX;ix++)
        {
			BasicDrawable::Triangle triA,triB;
			triA.verts[0] = iy*(SphereTessX+1)+ix;
			triA.verts[1] = iy*(SphereTessX+1)+(ix+1);
			triA.verts[2] = (iy+1)*(SphereTessX+1)+(ix+1);
			triB.verts[0] = triA.verts[0];
			triB.verts[1] = triA.verts[2];
			triB.verts[2] = (iy+1)*(SphereTessX+1)+ix;
            tris.push_back(triA);
            tris.push_back(triB);
        }
    
    triBuilder->addTriangles(locs,norms,colors,tris);
}

@end

@implementation WhirlyKitCylinder

@synthesize loc;
@synthesize baseHeight;
@synthesize radius;
@synthesize height;

// Build the geometry for a circle in display space
- (void)makeGeometryWithBuilder:(WhirlyKit::ShapeDrawableBuilder *)regBuilder triBuilder:(WhirlyKit::ShapeDrawableBuilderTri *)triBuilder scene:(WhirlyKit::Scene *)scene;
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    
    RGBAColor theColor = (useColor ? color : [regBuilder->shapeInfo.color asRGBAColor]);

    Point3f localPt = coordAdapter->getCoordSystem()->geographicToLocal(loc);
    Point3f dispPt = coordAdapter->localToDisplay(localPt);
    Point3f norm = coordAdapter->normalForLocal(localPt);
    
    // Move up by baseHeight
    dispPt += norm * baseHeight;
    
    // Construct a set of axes to build the circle around
    Point3f up = norm;
    Point3f xAxis,yAxis;
    if (coordAdapter->isFlat())
    {
        xAxis = Point3f(1,0,0);
        yAxis = Point3f(0,1,0);
    } else {
        Point3f north(0,0,1);
        // Note: Also check if we're at a pole
        xAxis = north.cross(up);  xAxis.normalize();
        yAxis = up.cross(xAxis);  yAxis.normalize();
    }

    // Calculate samples around the bottom
    std::vector<Point3f> samples;
    samples.resize(CircleSamples);
    for (unsigned int ii=0;ii<CircleSamples;ii++)
        samples[ii] =  xAxis * radius * sinf(2*M_PI*ii/(float)(CircleSamples-1)) + radius * yAxis * cosf(2*M_PI*ii/(float)(CircleSamples-1)) + dispPt;
    
    // We need the bounding box in the local coordinate system
    // Note: This is not handling height correctly
    Mbr shapeMbr;
    for (unsigned int ii=0;ii<samples.size();ii++)
    {
        Point3f thisLocalPt = coordAdapter->displayToLocal(samples[ii]);
        // Note: If this shape has height, this is insufficient
        shapeMbr.addPoint(Point2f(thisLocalPt.x(),thisLocalPt.y()));
    }
    
    // For the top we just offset
    std::vector<Point3f> top = samples;
    for (unsigned int ii=0;ii<top.size();ii++)
    {
        Point3f &pt = top[ii];
        pt = pt + height * norm;
    }
    triBuilder->addConvexOutline(top,norm,theColor,shapeMbr);
    
    // For the sides we'll just run things bottom to top
    for (unsigned int ii=0;ii<CircleSamples;ii++)
    {
        std::vector<Point3f> pts(4);
        pts[0] = samples[ii];
        pts[1] = samples[(ii+1)%samples.size()];
        pts[2] = top[(ii+1)%top.size()];
        pts[3] = top[ii];
        Point3f thisNorm = (pts[0]-pts[1]).cross(pts[2]-pts[1]);
        thisNorm.normalize();
        triBuilder->addConvexOutline(pts, thisNorm, theColor, shapeMbr);
    }
}

@end

@implementation WhirlyKitShapeLinear

@synthesize pts;
@synthesize mbr;
@synthesize lineWidth;

- (void)makeGeometryWithBuilder:(WhirlyKit::ShapeDrawableBuilder *)regBuilder triBuilder:(WhirlyKit::ShapeDrawableBuilderTri *)triBuilder scene:(WhirlyKit::Scene *)scene;
{
    RGBAColor theColor = (useColor ? color : [regBuilder->shapeInfo.color asRGBAColor]);

    regBuilder->addPoints(pts, theColor, mbr, lineWidth, false);
}

@end

@implementation ShapeInfo

@synthesize shapes;
@synthesize color;
@synthesize drawOffset;
@synthesize minVis,maxVis;
@synthesize drawPriority;
@synthesize fade;
@synthesize lineWidth;
@synthesize shapeId;

// Initialize with an array of shapes and parse out parameters
- (id)initWithShapes:(NSArray *)inShapes desc:(NSDictionary *)desc;
{
    self = [super init];
    
    if (self)
    {
        self.shapes = inShapes;
        [self parseDesc:desc];
        
        shapeId = Identifiable::genId();
    }
    
    return self;
}


- (void)parseDesc:(NSDictionary *)desc
{
    self.color = [desc objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]];
    drawOffset = [desc intForKey:@"drawOffset" default:0];
    minVis = [desc floatForKey:@"minVis" default:DrawVisibleInvalid];
    maxVis = [desc floatForKey:@"maxVis" default:DrawVisibleInvalid];
    drawPriority = [desc intForKey:@"drawPriority" default:ShapeDrawPriority];
    lineWidth = [desc floatForKey:@"width" default:1.0];
    fade = [desc floatForKey:@"fade" default:0.0];
}

@end

@implementation WhirlyKitShapeLayer

@synthesize selectLayer;

- (void)clear
{
    for (ShapeSceneRepSet::iterator it = shapeReps.begin();
         it != shapeReps.end(); ++it)
        delete *it;
    shapeReps.clear();    
}

- (void)dealloc
{
    [self clear];
}

// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(Scene *)inScene
{
    layerThread = inLayerThread;
    scene = inScene;
}

- (void)shutdown
{
    std::vector<ChangeRequest *> changeRequests;
    
    for (ShapeSceneRepSet::iterator it = shapeReps.begin();
         it != shapeReps.end(); ++it)
        (*it)->clearContents(selectLayer,changeRequests);
    
    scene->addChangeRequests(changeRequests);
    
    [self clear];
}

// Add a single shape
- (SimpleIdentity) addShape:(WhirlyKitShape *)shape desc:(NSDictionary *)desc
{
    return [self addShapes:[NSArray arrayWithObject:shape] desc:desc];
}

// Do the work for adding shapes
- (void)runAddShapes:(ShapeInfo *)shapeInfo
{
    ShapeSceneRep *sceneRep = new ShapeSceneRep(shapeInfo.shapeId);
    sceneRep->fade = shapeInfo.fade;

    std::vector<ChangeRequest *> changeRequests;
    ShapeDrawableBuilderTri drawBuildTri(scene,changeRequests,sceneRep,shapeInfo);
    ShapeDrawableBuilder drawBuildReg(scene,changeRequests,sceneRep,shapeInfo,true);

    // Work through the shapes
    for (WhirlyKitShape *shape in shapeInfo.shapes)
        [shape makeGeometryWithBuilder:&drawBuildReg triBuilder:&drawBuildTri scene:scene];
    
    // Flush out remaining geometry
    drawBuildReg.flush();
    drawBuildTri.flush();
    
    scene->addChangeRequests(changeRequests);
    
    shapeReps.insert(sceneRep);
}

// Do the work for removing shapes
- (void)runRemoveShapes:(NSNumber *)num
{
    SimpleIdentity shapeId = [num unsignedIntValue];
    
    ShapeSceneRep dummyRep(shapeId);
    ShapeSceneRepSet::iterator it = shapeReps.find(&dummyRep);
    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
    if (it != shapeReps.end())
    {
        ShapeSceneRep *shapeRep = *it;
        
        std::vector<ChangeRequest *> changeRequests;
        if (shapeRep->fade > 0.0)
        {
            for (SimpleIDSet::iterator idIt = shapeRep->drawIDs.begin();
                 idIt != shapeRep->drawIDs.end(); ++idIt)
                changeRequests.push_back(new FadeChangeRequest(*idIt, curTime, curTime+shapeRep->fade));
            [self performSelector:@selector(runRemoveShapes:) withObject:num afterDelay:shapeRep->fade];
            shapeRep->fade = 0.0;
        } else {
            shapeRep->clearContents(selectLayer, changeRequests);
            shapeReps.erase(it);
            delete shapeRep;
        }
        
        scene->addChangeRequests(changeRequests);
    }
}

// Add a whole bunch of shapes
- (SimpleIdentity) addShapes:(NSArray *)shapes desc:(NSDictionary *)desc
{
    ShapeInfo *shapeInfo = [[ShapeInfo alloc] initWithShapes:shapes desc:desc];
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runAddShapes:shapeInfo];
    else
        [self performSelector:@selector(runAddShapes:) onThread:layerThread withObject:shapeInfo waitUntilDone:NO];
    
    return shapeInfo.shapeId;
}

// Remove a group of shapes
- (void) removeShapes:(WhirlyKit::SimpleIdentity)shapeID
{
    NSNumber *num = [NSNumber numberWithUnsignedInt:shapeID];
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runRemoveShapes:num];
    else
        [self performSelector:@selector(runRemoveShapes:) onThread:layerThread withObject:num waitUntilDone:NO];
}

@end

