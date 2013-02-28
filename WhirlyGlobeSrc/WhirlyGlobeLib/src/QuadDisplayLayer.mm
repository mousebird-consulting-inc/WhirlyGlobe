/*
 *  QuadDisplayLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/17/12.
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

#import "QuadDisplayLayer.h"
#import "GlobeMath.h"
#import "GlobeLayerViewWatcher.h"
#import "MaplyLayerViewWatcher.h"
#import "UIImage+Stuff.h"
#import "FlatMath.h"
#import "VectorData.h"
#import <boost/math/special_functions/fpclassify.hpp>

using namespace Eigen;
using namespace WhirlyKit;

@implementation WhirlyKitDisplaySolid

// Let's not support tiles less than 10m on a side
static float const BoundsEps = 10.0 / EarthRadius;

+ (WhirlyKitDisplaySolid *)displaySolidWithNodeIdent:(WhirlyKit::Quadtree::Identifier &)nodeIdent mbr:(WhirlyKit::Mbr)nodeMbr srcSystem:(WhirlyKit::CoordSystem *)srcSystem adapter:(WhirlyKit::CoordSystemDisplayAdapter *)coordAdapter
{
    WhirlyKitDisplaySolid *dispSolid = [[WhirlyKitDisplaySolid alloc] init];
    
    // Start with the outline in the source coordinate system
    WhirlyKit::CoordSystem *displaySystem = coordAdapter->getCoordSystem();
    Point3f srcBounds[4];
    srcBounds[0] = Point3f(nodeMbr.ll().x(),nodeMbr.ll().y(),0.0);
    srcBounds[1] = Point3f(nodeMbr.ur().x(),nodeMbr.ll().y(),0.0);
    srcBounds[2] = Point3f(nodeMbr.ur().x(),nodeMbr.ur().y(),0.0);
    srcBounds[3] = Point3f(nodeMbr.ll().x(),nodeMbr.ur().y(),0.0);
    
    // Figure out where the bounds drop in display space
    std::vector<Point3f> dispBounds;
    std::vector<Point3f> srcPts;
    for (unsigned int ii=0;ii<4;ii++)
    {
        Point3f localPt = CoordSystemConvert(srcSystem, displaySystem, srcBounds[ii]);
        Point3f dispPt = coordAdapter->localToDisplay(localPt);
        Point3f dispNorm = coordAdapter->normalForLocal(localPt);
        dispSolid->surfNormals.push_back(dispNorm);
        // If the previous one is too close, ditch this one
        if (ii > 0)
        {
            float dist2 = (dispBounds[dispBounds.size()-1] - dispPt).squaredNorm();
            if (dist2 > BoundsEps*BoundsEps)
            {
                dispBounds.push_back(dispPt);
                srcPts.push_back(srcBounds[ii]);
            }
        } else {
            dispBounds.push_back(dispPt);
            srcPts.push_back(srcBounds[ii]);
        }
    }
    
    // If we didn't get enough boundary points, this is degenerate
    if (dispBounds.size() < 3)
        return nil;
    
    // We'll set up a plane and start working in that space
    Point3f localMidPt = CoordSystemConvert(srcSystem, displaySystem, (srcBounds[0]+srcBounds[2])/2.0);
    Point3f dispMidPt = coordAdapter->localToDisplay(localMidPt);
    Point3f zAxis = coordAdapter->normalForLocal(localMidPt);  zAxis.normalize();
    Point3f xAxis = dispBounds[1] - dispBounds[0];  xAxis.normalize();
    Point3f yAxis = zAxis.cross(xAxis); yAxis.normalize();
    Point3f org = dispMidPt;
    
    // Project the corner points onto the plane
    // We'll collect height at the same time
    std::vector<Point2f> planePts;
    float minZ=MAXFLOAT,maxZ =-MAXFLOAT;
    Mbr planeMbr;
    for (unsigned int ii=0;ii<dispBounds.size();ii++)
    {
        Point3f dir = dispBounds[ii]-org;
        Point3f planePt(dir.dot(xAxis),dir.dot(yAxis),dir.dot(zAxis));
        minZ = std::min(minZ,planePt.z());
        maxZ = std::max(maxZ,planePt.z());
        planePts.push_back(Point2f(planePt.x(),planePt.y()));
        planeMbr.addPoint(Point2f(planePt.x(),planePt.y()));
    }

    // Now sample the edges back in the source coordinate system
    //  and see where the land in here
    for (unsigned int ii=0;ii<srcPts.size();ii++)
    {
//        Point2f &planePt0 = planePts[ii], &planePt1 = planePts[(ii+1)%planePts.size()];
        // Project the test point all the way into our plane
        Point3f edgeSrcPt = (srcPts[ii]+srcPts[(ii+1)%srcPts.size()])/2.0;
        Point3f localPt = CoordSystemConvert(srcSystem, displaySystem, edgeSrcPt);
        Point3f edgeDispPt = coordAdapter->localToDisplay(localPt);
        Point3f dir = edgeDispPt-org;
        Point3f planePt(dir.dot(xAxis),dir.dot(yAxis),dir.dot(zAxis));
        // Update the min and max
        minZ = std::min(minZ,planePt.z());
        maxZ = std::max(maxZ,planePt.z());
        // Note: Trying MBR
        planeMbr.addPoint(Point2f(planePt.x(),planePt.y()));

        // And throw in another normal for the biggest tiles
        Point3f dispNorm = coordAdapter->normalForLocal(localPt);
        dispSolid->surfNormals.push_back(dispNorm);

#if 0
        // See if the plane pt is on the right of the two sample points
        if ((planePt1.x()-planePt0.x())*(planePt.y()-planePt0.y()) - (planePt1.y()-planePt0.y())*(planePt.x()-planePt0.x()) < 0.0)
        {
            // If it is, we need to nudge the line outward
            // Need the point previous to 0 and the one after 1
            Point2f &planePt_1 = planePts[(ii-1+srcPts.size())%srcPts.size()];
            Point2f &planePt2 = planePts[(ii+2)%srcPts.size()];
            Point2f dir = planePt1 - planePt0;
            Point2f ePt0 = Point2f(planePt.x(),planePt.y());
            Point2f ePt1 = ePt0 + dir;

            // Find the various intersections
            Point2f new_planePt0,new_planePt1;
            if (IntersectLines(ePt0, ePt1, planePt_1, planePt0, &new_planePt0))
                planePt0 = new_planePt0;
            if (IntersectLines(ePt0, ePt1, planePt1, planePt2, &new_planePt1))
                planePt1 = new_planePt1;
        }
#endif
    }
    
    // Now convert the plane points back into display space for the volume
    std::vector<WhirlyKit::Point3f> botCorners;
    std::vector<WhirlyKit::Point3f> topCorners;
    std::vector<WhirlyKit::Point2f> planeMbrPts;
    planeMbr.asPoints(planeMbrPts);
//    for (unsigned int ii=0;ii<planePts.size();ii++)
    for (unsigned int ii=0;ii<planeMbrPts.size();ii++)
    {
#if 0
        Point2f planePt = planePts[ii];
#endif
        Point2f planePt = planeMbrPts[ii];
        Point3f dispPt0 = xAxis * planePt.x() + yAxis * planePt.y() + zAxis * minZ + org;
        Point3f dispPt1 = xAxis * planePt.x() + yAxis * planePt.y() + zAxis * maxZ + org;
        botCorners.push_back(dispPt0);
        topCorners.push_back(dispPt1);        
    }
    
    // Now let's go ahead and form the polygons for the planes
    // First the ones around the outside
    for (unsigned int ii=0;ii<planePts.size();ii++)
    {
        int thisPt = ii;
        int nextPt = (ii+1)%planePts.size();
        std::vector<Point3f> poly;
        poly.push_back(botCorners[thisPt]);
        poly.push_back(botCorners[nextPt]);
        poly.push_back(topCorners[nextPt]);
        poly.push_back(topCorners[thisPt]);
        dispSolid->polys.push_back(poly);
    }
    // Then top and bottom
    dispSolid->polys.push_back(topCorners);
    std::reverse(botCorners.begin(),botCorners.end());
    dispSolid->polys.push_back(botCorners);
    
    // Now calculate normals for each of those
    for (unsigned int ii=0;ii<dispSolid->polys.size();ii++)
    {
        std::vector<Point3f> &poly = dispSolid->polys[ii];
        Point3f &p0 = poly[0];
        Point3f &p1 = poly[1];
        Point3f &p2 = poly[poly.size()-1];
        Vector3f norm = (p1-p0).cross(p2-p0);
        norm.normalize();
        dispSolid->normals.push_back(norm);
    }
        
    return dispSolid;
}

// Calculate the size of a rectangle projected onto the screen
float PolyImportance(const std::vector<Point3f> &poly,WhirlyKitViewState *viewState,WhirlyKit::Point2f frameSize)
{
    // Project the points onto the screen
    std::vector<Point2f> screenPts;
    Mbr mbrOnScreen;
    for (unsigned int ii=0;ii<poly.size();ii++)
    {
        CGPoint screenPt = [viewState pointOnScreenFromDisplay:poly[ii] transform:&viewState->modelMatrix frameSize:frameSize];
        mbrOnScreen.addPoint(Point2f(screenPt.x,screenPt.y));
        screenPts.push_back(Point2f(screenPt.x,screenPt.y));
    }
    Mbr frameMbr(Point2f(0,0),Point2f(frameSize.x(),frameSize.y()));
    
    // If there's no overlap, we don't care about it
    if (!mbrOnScreen.overlaps(frameMbr))
        return 0.0;

    float area = CalcLoopArea(screenPts);
    
    if (boost::math::isnan(area))
        area = 0.0;
    
    return std::abs(area);
}

- (bool)isInside:(WhirlyKit::Point3f)pt
{
    // We should be on the inside of each plane
    for (unsigned int ii=0;ii<polys.size();ii++)
    {
        const Point3f &org = (polys[ii])[0];
        if ((pt-org).dot(normals[ii]) > 0.0)
            return false;
    }
    
    return true;
}

- (float)importanceForViewState:(WhirlyKitViewState *)viewState frameSize:(WhirlyKit::Point2f)frameSize;
{
    Point3f eyePos = viewState.eyePos;
    
    // If the viewer is inside the bounds, the node is maximimally important (duh)
    if ([self isInside:eyePos])
        return MAXFLOAT;
    
    // Make sure that we're pointed toward the eye, even a bit
    bool isFacing = false;
    for (unsigned int ii=0;ii<surfNormals.size();ii++)
    {
        const Vector3f &surfNorm = surfNormals[ii];
        isFacing |= surfNorm.dot(eyePos) >= 0.0;
    }
    if (!isFacing)
        return 0.0;
    
    // Now work through the polygons and project each to the screen
    float totalImport = 0.0;
    for (unsigned int ii=0;ii<polys.size();ii++)
    {
        const std::vector<Point3f> &poly = polys[ii];
        float import = PolyImportance(poly, viewState, frameSize);
        totalImport += import;
    }
    
    return totalImport;
}

@end

namespace WhirlyKit
{
// Calculate the importance for the given texel
static float calcImportance(WhirlyKitViewState *viewState,Point3f eyeVec,Point3f pt,Point2f pixSize,Point2f frameSize,CoordSystem *srcSystem,CoordSystem *destSystem,CoordSystemDisplayAdapter *coordAdapter)
{
    Point3f pts[4];
    pts[0] = pt + Point3f(-pixSize.x()/2.0,-pixSize.y()/2.0,0.0);
    pts[1] = pt + Point3f(pixSize.x()/2.0,-pixSize.y()/2.0,0.0);
    pts[2] = pt + Point3f(pixSize.x()/2.0,pixSize.y()/2.0,0.0);
    pts[3] = pt + Point3f(-pixSize.x()/2.0,pixSize.y()/2.0,0.0);
    
    // Convert to 3-space
    Point3f pts3d[4];
    Point2f screenPts[4];
    bool forwardFacing = false;
    for (unsigned int ii=0;ii<4;ii++)
    {
        pts3d[ii] = coordAdapter->localToDisplay(CoordSystemConvert(srcSystem, destSystem, pts[ii]));
        
        // Check the normal (point in this case) against the eye vec
        if (!coordAdapter->isFlat())
            if (pts3d[ii].dot(eyeVec) > 0.0)
                forwardFacing = true;
        
        CGPoint screenPt = [viewState pointOnScreenFromDisplay:pts3d[ii] transform:&viewState->fullMatrix frameSize:frameSize];
        screenPts[ii] = Point2f(screenPt.x,screenPt.y);
    }
    
    // Look at area on the screen
    float area = 0.0;
    if (forwardFacing || coordAdapter->isFlat())
    {
        Point2f ac = screenPts[2]-screenPts[0];
        Point2f bd = screenPts[3]-screenPts[1];
        area = 0.5 * (ac.x()*bd.y()-bd.x()*ac.y());
    }
    
    if (boost::math::isnan(area))
        area = 0.0;
        
    return std::abs(area);
}


// Calculate the max pixel size for a tile
float ScreenImportanceOld(WhirlyKitViewState *viewState,WhirlyKit::Point2f frameSize,WhirlyKit::Point3f eyeVec,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,Mbr nodeMbr)
{
    WhirlyKit::CoordSystem *displaySystem = coordAdapter->getCoordSystem();
    Point2f pixSize((nodeMbr.ur().x()-nodeMbr.ll().x())/pixelsSquare,(nodeMbr.ur().y()-nodeMbr.ll().y())/pixelsSquare);
    Point3f testPoints[6];
    int numTestPoints = 0;
    testPoints[0] = Point3f(nodeMbr.ll().x()+pixSize.x(),nodeMbr.ll().y()+pixSize.y(),0.0);
    testPoints[1] = Point3f(nodeMbr.ur().x()-pixSize.x(),nodeMbr.ll().y()+pixSize.y(),0.0);
    testPoints[2] = Point3f(nodeMbr.ur().x()-pixSize.x(),nodeMbr.ur().y()-pixSize.y(),0.0);
    testPoints[3] = Point3f(nodeMbr.ll().x()+pixSize.x(),nodeMbr.ur().y()-pixSize.y(),0.0);
    testPoints[4] = (testPoints[0]+testPoints[2])/2.0;
    numTestPoints = 5;
    
    // Let's make sure we at least overlap the screen
    // Note: Need to fix this for Maply
    Mbr mbrOnScreen;
    for (unsigned int ii=0;ii<4;ii++)
    {
        Point3f pt3d = coordAdapter->localToDisplay(CoordSystemConvert(srcSystem, displaySystem, testPoints[ii]));
        
        CGPoint screenPt = [viewState pointOnScreenFromDisplay:pt3d transform:&viewState->fullMatrix frameSize:frameSize];
        mbrOnScreen.addPoint(Point2f(screenPt.x,screenPt.y));
    }
    Mbr frameMbr(Point2f(0,0),Point2f(frameSize.x(),frameSize.y()));
    if (!mbrOnScreen.overlaps(frameMbr))
        return 0.0;
    
#if 0
    // Figure out the intersection of the projection and the screen MBR
    // We'll take the middle, project that back and toss that in as a test point
    Mbr screenIntersect = mbrOnScreen.intersect(frameMbr);
    Point2f intersectMid = screenIntersect.mid();
    if ([viewState isKindOfClass:[WhirlyGlobeViewState class]])
    {
        WhirlyGlobeViewState *globeViewState = (WhirlyGlobeViewState *)viewState;
        Point3f dispPt;
        if ([globeViewState pointOnSphereFromScreen:CGPointMake(intersectMid.x(), intersectMid.y()) transform:&viewState->fullMatrix frameSize:frameSize hit:&dispPt])
        {
            Point3f localPt = coordAdapter->displayToLocal(dispPt);
            if (nodeMbr.inside(Point2f(localPt.x(),localPt.y())))
            {
                testPoints[numTestPoints++] = coordAdapter->displayToLocal(dispPt);
                numTestPoints++;
            }
        }
    }
#endif
    
    float maxImport = 0.0;
    for (unsigned int ii=0;ii<numTestPoints;ii++)
    {
        float thisImport = calcImportance(viewState,eyeVec,testPoints[ii],pixSize,frameSize,srcSystem,displaySystem,coordAdapter);
        maxImport = std::max(thisImport,maxImport);
    }
    
    return maxImport;
}

// Calculate the max pixel size for a tile
float ScreenImportance(WhirlyKitViewState *viewState,WhirlyKit::Point2f frameSize,WhirlyKit::Point3f eyeVec,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,Mbr nodeMbr,WhirlyKit::Quadtree::Identifier &nodeIdent,NSMutableDictionary *attrs)
{
    WhirlyKitDisplaySolid *dispSolid = attrs[@"DisplaySolid"];
    if (!dispSolid)
    {
        dispSolid = [WhirlyKitDisplaySolid displaySolidWithNodeIdent:nodeIdent mbr:nodeMbr srcSystem:srcSystem adapter:coordAdapter];
        if (dispSolid)
            attrs[@"DisplaySolid"] = dispSolid;
        else
            attrs[@"DisplaySolid"] = [NSNull null];
    }
    
    // This means the tile is degenerate (as far as we're concerned)
    if ([dispSolid isKindOfClass:[NSNull null]])
        return 0.0;

    float import = [dispSolid importanceForViewState:viewState frameSize:frameSize];
        // The system is expecting an estimate of pixel size on screen
    import = import/(pixelsSquare * pixelsSquare);
    
//    NSLog(@"Import: %d: (%d,%d)  %f",nodeIdent.level,nodeIdent.x,nodeIdent.y,import);
    
    return import;
}

    
}

@implementation WhirlyKitQuadDisplayLayer

@synthesize layerThread;
@synthesize scene;
@synthesize quadtree;
@synthesize coordSys;
@synthesize mbr;
@synthesize maxTiles;
@synthesize minImportance;
@synthesize lineMode;
@synthesize drawEmpty;
@synthesize debugMode;
@synthesize dataStructure;
@synthesize loader;
@synthesize viewUpdatePeriod;
@synthesize renderer;

- (id)initWithDataSource:(NSObject<WhirlyKitQuadDataStructure> *)inDataStructure loader:(NSObject<WhirlyKitQuadLoader> *)inLoader renderer:(WhirlyKitSceneRendererES *)inRenderer;
{
    self = [super init];
    if (self)
    {
        dataStructure = inDataStructure;
        loader = inLoader;
        [loader setQuadLayer:self];
        coordSys = [dataStructure coordSystem];
        mbr = [dataStructure validExtents];
        minZoom = [dataStructure minZoom];
        maxZoom = [dataStructure maxZoom];
        maxTiles = 256;
        minImportance = 1.0;
        viewUpdatePeriod = 0.1;
        quadtree = new Quadtree([dataStructure totalExtents],minZoom,maxZoom,maxTiles,minImportance,self);
        renderer = inRenderer;
        lineMode = false;
        drawEmpty = false;
        debugMode = false;
        greedyMode = false;
    }
    
    return self;
}

- (void)dealloc
{
    if (quadtree)
        delete quadtree;
}

- (void)setMaxTiles:(int)newMaxTiles
{
    maxTiles = newMaxTiles;
    quadtree->setMaxNodes(newMaxTiles);
}

- (void)setMinImportance:(float)newMinImportance
{
    minImportance = newMinImportance;
    quadtree->setMinImportance(newMinImportance);
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(Scene *)inScene
{
    layerThread = inLayerThread;
	scene = inScene;
        
    // We want view updates, but only 1s in frequency
    if (layerThread.viewWatcher)
        [(WhirlyGlobeLayerViewWatcher *)layerThread.viewWatcher addWatcherTarget:self selector:@selector(viewUpdate:) minTime:viewUpdatePeriod];
}

- (void)shutdown
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    
    if (layerThread.viewWatcher) {
        [(WhirlyGlobeLayerViewWatcher *)layerThread.viewWatcher removeWatcherTarget:self selector:@selector(viewUpdate:)];
    }
    
    [dataStructure shutdown];
    dataStructure = nil;
    [loader shutdownLayer:self scene:scene];
    loader = nil;
    
    scene = NULL;
}

// Called every so often by the view watcher
// It's here that we evaluate what to load
- (void)viewUpdate:(WhirlyKitViewState *)inViewState
{
    if (!scene)
    {
        NSLog(@"GlobeQuadDisplayLayer: Called viewUpdate: after begin shutdown.");
        return;
    }
    
    // Check if we should even be doing an update
    if ([loader respondsToSelector:@selector(shouldUpdate:initial:)])
        if (![loader shouldUpdate:inViewState initial:(viewState == nil)])
            return;
        
    viewState = inViewState;
    nodesForEval.clear();
    quadtree->reevaluateNodes();
    
    // Add everything at the minLevel back in
    for (int ix=0;ix<1<<minZoom;ix++)
        for (int iy=0;iy<1<<minZoom;iy++)
        {
            Quadtree::NodeInfo thisNode = quadtree->generateNode(Quadtree::Identifier(ix,iy,minZoom));
            nodesForEval.insert(thisNode);
        }
        
//    NSLog(@"View update called: (eye) = (%f,%f,%f), nodesForEval = %lu",eyeVec3.x(),eyeVec3.y(),eyeVec3.z(),nodesForEval.size());
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
}

// Dump out info about what we've got loaded in
- (void)dumpInfo
{
//    NSLog(@"***Loaded Tiles***");
//    for (LoadedTileSet::iterator it = tileSet.begin();
//         it != tileSet.end(); ++it)
//    {
//        (*it)->Print(quadtree);
//    }
//    NSLog(@"******");
    
    quadtree->Print();
}

// Run the evaluation step for outstanding nodes
- (void)evalStep:(id)Sender
{
    bool didSomething = false;
    
    // If the renderer hasn't been set up, punt and try again later
    if (renderer.framebufferWidth == 0 || renderer.framebufferHeight == 0)
    {
        [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
        return;
    }    
    
    // Look for nodes to remove
    Quadtree::NodeInfo remNodeInfo;
    while (quadtree->leastImportantNode(remNodeInfo))
    {
        [loader quadDisplayLayerStartUpdates:self];

        quadtree->removeTile(remNodeInfo.ident);
        [loader quadDisplayLayer:self unloadTile:remNodeInfo];

        [loader quadDisplayLayerEndUpdates:self];
        didSomething = true;
    }
    
    // If the loader isn't ready, try again in a bit
    if (![loader isReady])
    {
        [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];        
        return;
    }
    
    if (!nodesForEval.empty())
    {
        // Let the loader know we're about to do some updates
        [loader quadDisplayLayerStartUpdates:self];
        
        while (!nodesForEval.empty())
        {
        // Grab the first node.
        QuadNodeInfoSet::iterator nodeIt = nodesForEval.end();
        nodeIt--;
        Quadtree::NodeInfo nodeInfo = *nodeIt;
        nodesForEval.erase(nodeIt);
        
        // The quad tree will take this node over an existing one
        bool isLoaded = quadtree->isTileLoaded(nodeInfo.ident);
        if (isLoaded || quadtree->willAcceptTile(nodeInfo))
        {
            if (!isLoaded)
            {
                // Tell the quad tree what we're up to
                std::vector<Quadtree::Identifier> tilesToRemove;
                quadtree->addTile(nodeInfo, tilesToRemove);
                            
                [loader quadDisplayLayer:self loadTile:nodeInfo ];
                                
                // Remove the old tiles
                for (unsigned int ii=0;ii<tilesToRemove.size();ii++)
                {
                    Quadtree::Identifier &thisIdent = tilesToRemove[ii];
                    //                    NSLog(@"Quad tree removed (%d,%d,%d)",thisIdent.x,thisIdent.y,thisIdent.level);
                    
                    Quadtree::NodeInfo remNodeInfo = quadtree->generateNode(thisIdent);
                    [loader quadDisplayLayer:self unloadTile:remNodeInfo];           
                }
    //            NSLog(@"Quad loaded node (%d,%d,%d) = %.4f",nodeInfo.ident.x,nodeInfo.ident.y,nodeInfo.ident.level,nodeInfo.importance);            
            } else {
                // It is loaded (as far as we're concerned), so we need to know if we can traverse below that
                if (nodeInfo.ident.level < maxZoom && [loader quadDisplayLayer:self canLoadChildrenOfTile:nodeInfo])
                {
                    std::vector<Quadtree::NodeInfo> childNodes;
                    quadtree->generateChildren(nodeInfo.ident, childNodes);
                    nodesForEval.insert(childNodes.begin(),childNodes.end());                
                }
            }
        } else
        {
            //        NSLog(@"Quad rejecting node (%d,%d,%d) = %.4f",nodeInfo.ident.x,nodeInfo.ident.y,nodeInfo.ident.level,nodeInfo.importance);
        }
        
            // If we're not in greedy mode, we'll just do this once through
            if (!greedyMode)
                break;
        }
        
        // Let the loader know we're done with this eval step
        [loader quadDisplayLayerEndUpdates:self];
        
        didSomething = true;
    }
    
//    if (debugMode)
//        [self dumpInfo];
    
    if (didSomething)
        [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
}

// This is called by the loader when it finished loading a tile
// Once loaded we can try the children
- (void)loader:(NSObject<WhirlyKitQuadLoader> *)loader tileDidLoad:(WhirlyKit::Quadtree::Identifier)tileIdent
{
    // Note: Need to check that we still care about this load
    
    if (tileIdent.level < maxZoom)
    {
        // Make sure we still want this one
        if (!quadtree->isTileLoaded(tileIdent))
            return;
        
        // Now try the children
        std::vector<Quadtree::NodeInfo> childNodes;
        quadtree->generateChildren(tileIdent, childNodes);
        nodesForEval.insert(childNodes.begin(),childNodes.end());
        
        // Make sure we actually evaluate them
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
        [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
    }
}

// Tile failed to load.
// At the moment we don't care, but we won't look at the children
- (void)loader:(NSObject<WhirlyKitQuadLoader> *)loader tileDidNotLoad:(WhirlyKit::Quadtree::Identifier)tileIdent
{
    
}

// Clear out all the existing tiles and start over
- (void)refresh
{
    if ([NSThread currentThread] != layerThread)
    {
        [self performSelector:@selector(refresh) onThread:layerThread withObject:nil waitUntilDone:NO];
        return;
    }
    
    // Clean out anything we might be currently evaluating
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
    nodesForEval.clear();

    // Remove nodes until we run out
    Quadtree::NodeInfo remNodeInfo;
    [loader quadDisplayLayerStartUpdates:self];
    while (quadtree->leastImportantNode(remNodeInfo,true))
    {
        
        quadtree->removeTile(remNodeInfo.ident);
        [loader quadDisplayLayer:self unloadTile:remNodeInfo];        
    }
    [loader quadDisplayLayerEndUpdates:self];

    // Add everything at the minLevel back in
    for (int ix=0;ix<1<<minZoom;ix++)
        for (int iy=0;iy<1<<minZoom;iy++)
        {
            Quadtree::NodeInfo thisNode = quadtree->generateNode(Quadtree::Identifier(ix,iy,minZoom));
            nodesForEval.insert(thisNode);
        }

    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
}

#pragma mark - Quad Tree Importance Delegate

- (float)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(Mbr)theMbr tree:(WhirlyKit::Quadtree *)tree attrs:(NSMutableDictionary *)attrs
{
    return [dataStructure importanceForTile:ident mbr:theMbr viewInfo:viewState frameSize:Point2f(renderer.framebufferWidth,renderer.framebufferHeight) attrs:attrs];
}

@end

