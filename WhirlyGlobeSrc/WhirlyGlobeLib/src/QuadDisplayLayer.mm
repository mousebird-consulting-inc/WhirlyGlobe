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

+ (WhirlyKitDisplaySolid *)displaySolidWithNodeIdent:(WhirlyKit::Quadtree::Identifier &)nodeIdent mbr:(WhirlyKit::Mbr)nodeMbr minZ:(float)inMinZ maxZ:(float)inMaxZ srcSystem:(WhirlyKit::CoordSystem *)srcSystem adapter:(WhirlyKit::CoordSystemDisplayAdapter *)coordAdapter;
{
    WhirlyKitDisplaySolid *dispSolid = [[WhirlyKitDisplaySolid alloc] init];
    
    // Start with the outline in the source coordinate system
    WhirlyKit::CoordSystem *displaySystem = coordAdapter->getCoordSystem();
    std::vector<Point3d> srcBounds;
    srcBounds.push_back(Point3d(nodeMbr.ll().x(),nodeMbr.ll().y(),inMinZ));
    srcBounds.push_back(Point3d(nodeMbr.ur().x(),nodeMbr.ll().y(),inMinZ));
    srcBounds.push_back(Point3d(nodeMbr.ur().x(),nodeMbr.ur().y(),inMinZ));
    srcBounds.push_back(Point3d(nodeMbr.ll().x(),nodeMbr.ur().y(),inMinZ));
    srcBounds.push_back(Point3d((nodeMbr.ll().x()+nodeMbr.ur().x())/2.0,(nodeMbr.ll().y()+nodeMbr.ur().y())/2.0,inMinZ));
    if (inMinZ != inMaxZ)
    {
        srcBounds.push_back(Point3d(nodeMbr.ll().x(),nodeMbr.ll().y(),inMaxZ));
        srcBounds.push_back(Point3d(nodeMbr.ur().x(),nodeMbr.ll().y(),inMaxZ));
        srcBounds.push_back(Point3d(nodeMbr.ur().x(),nodeMbr.ur().y(),inMaxZ));
        srcBounds.push_back(Point3d(nodeMbr.ll().x(),nodeMbr.ur().y(),inMaxZ));
        srcBounds.push_back(Point3d((nodeMbr.ll().x()+nodeMbr.ur().x())/2.0,(nodeMbr.ll().y()+nodeMbr.ur().y())/2.0,inMaxZ));
    }
    
    // Figure out where the bounds drop in display space
    std::vector<Point3d> dispBounds;
    dispBounds.reserve(srcBounds.size());
    std::vector<Point3d> srcPts;
    srcPts.reserve(srcBounds.size());
    for (unsigned int ii=0;ii<srcBounds.size();ii++)
    {
        Point3d localPt = CoordSystemConvert3d(srcSystem, displaySystem, srcBounds[ii]);
        Point3d dispPt = coordAdapter->localToDisplay(localPt);
        Point3d dispNorm = coordAdapter->normalForLocal(localPt);
        dispSolid->surfNormals.push_back(dispNorm);
        // If the previous one is too close, ditch this one
        if (ii > 0)
        {
            double dist2 = (dispBounds[dispBounds.size()-1] - dispPt).squaredNorm();
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
    Point3d localMidPt = CoordSystemConvert3d(srcSystem, displaySystem, (srcBounds[0]+srcBounds[2])/2.0);
    Point3d dispMidPt = coordAdapter->localToDisplay(localMidPt);
    Point3d zAxis = coordAdapter->normalForLocal(localMidPt);  zAxis.normalize();
    Point3d xAxis = dispBounds[1] - dispBounds[0];  xAxis.normalize();
    Point3d yAxis = zAxis.cross(xAxis); yAxis.normalize();
    Point3d org = dispMidPt;
    
    // Project the corner points onto the plane
    // We'll collect height at the same time
    std::vector<Point2d> planePts;
    planePts.reserve(dispBounds.size());
    double minZ=MAXFLOAT,maxZ =-MAXFLOAT;
    Point2d minPt,maxPt;
    for (unsigned int ii=0;ii<dispBounds.size();ii++)
    {
        Point3d dir = dispBounds[ii]-org;
        Point3d planePt(dir.dot(xAxis),dir.dot(yAxis),dir.dot(zAxis));
        minZ = std::min(minZ,planePt.z());
        maxZ = std::max(maxZ,planePt.z());
        planePts.push_back(Point2d(planePt.x(),planePt.y()));
        if (ii == 0)
        {
            minPt = maxPt = Point2d(planePt.x(),planePt.y());
        } else {
            minPt.x() = std::min(minPt.x(),planePt.x());
            minPt.y() = std::min(minPt.y(),planePt.y());
            maxPt.x() = std::max(maxPt.x(),planePt.x());
            maxPt.y() = std::max(maxPt.y(),planePt.y());
        }
    }

    // Now sample the edges back in the source coordinate system
    //  and see where they land in here
    
    // Surface normals only make sense if there is a surface, but there are other important
    //  calculations below
    if (inMinZ == inMaxZ)
    dispSolid->surfNormals.reserve(srcPts.size());

    for (unsigned int ii=0;ii<srcPts.size();ii++)
    {
//        Point2f &planePt0 = planePts[ii], &planePt1 = planePts[(ii+1)%planePts.size()];
        // Project the test point all the way into our plane
        Point3d edgeSrcPt = (srcPts[ii]+srcPts[(ii+1)%srcPts.size()])/2.0;
        Point3d localPt = CoordSystemConvert3d(srcSystem, displaySystem, edgeSrcPt);
        Point3d edgeDispPt = coordAdapter->localToDisplay(localPt);
        Point3d dir = edgeDispPt-org;
        Point3d planePt(dir.dot(xAxis),dir.dot(yAxis),dir.dot(zAxis));
        // Update the min and max
        minZ = std::min(minZ,planePt.z());
        maxZ = std::max(maxZ,planePt.z());
        // Note: Trying MBR
        minPt.x() = std::min(minPt.x(),planePt.x());
        minPt.y() = std::min(minPt.y(),planePt.y());
        maxPt.x() = std::max(maxPt.x(),planePt.x());
        maxPt.y() = std::max(maxPt.y(),planePt.y());

        // And throw in another normal for the biggest tiles
        Point3d dispNorm = coordAdapter->normalForLocal(localPt);
        if (inMinZ == inMaxZ)
           dispSolid->surfNormals.push_back(Vector3d(dispNorm.x(),dispNorm.y(),dispNorm.z()));

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
    std::vector<WhirlyKit::Point3d> botCorners;
    std::vector<WhirlyKit::Point3d> topCorners;
    std::vector<WhirlyKit::Point2d> planeMbrPts;  planeMbrPts.reserve(4);
    planeMbrPts.push_back(minPt);
    planeMbrPts.push_back(Point2d(maxPt.x(),minPt.y()));
    planeMbrPts.push_back(maxPt);
    planeMbrPts.push_back(Point2d(minPt.x(),maxPt.y()));
    botCorners.reserve(planeMbrPts.size());
    topCorners.reserve(planeMbrPts.size());
//    for (unsigned int ii=0;ii<planePts.size();ii++)
    for (unsigned int ii=0;ii<planeMbrPts.size();ii++)
    {
        Point2d planePt = planeMbrPts[ii];
        Point3d dispPt0 = xAxis * planePt.x() + yAxis * planePt.y() + zAxis * minZ + org;
        Point3d dispPt1 = xAxis * planePt.x() + yAxis * planePt.y() + zAxis * maxZ + org;
        botCorners.push_back(Point3d(dispPt0.x(),dispPt0.y(),dispPt0.z()));
        topCorners.push_back(Point3d(dispPt1.x(),dispPt1.y(),dispPt1.z()));
    }
    
    // Now let's go ahead and form the polygons for the planes
    // First the ones around the outside
    dispSolid->polys.reserve(planeMbrPts.size()+2);
    for (unsigned int ii=0;ii<planeMbrPts.size();ii++)
    {
        int thisPt = ii;
        int nextPt = (ii+1)%planeMbrPts.size();
        std::vector<Point3d> poly;  poly.reserve(4);
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
    dispSolid->normals.reserve(dispSolid->polys.size());
    for (unsigned int ii=0;ii<dispSolid->polys.size();ii++)
    {
        if (coordAdapter->isFlat())
            dispSolid->normals.push_back(Vector3d(0,0,1));
        else {
            std::vector<Point3d> &poly = dispSolid->polys[ii];
            Point3d &p0 = poly[0];
            Point3d &p1 = poly[1];
            Point3d &p2 = poly[poly.size()-1];
            Vector3d norm = (p1-p0).cross(p2-p0);
            norm.normalize();
            dispSolid->normals.push_back(norm);
        }
    }
        
    return dispSolid;
}

float PolyImportance(const std::vector<Point3d> &poly,const Point3d &norm,WhirlyKitViewState *viewState,WhirlyKit::Point2f frameSize)
{
    float origArea = PolygonArea(poly,norm);
    origArea = std::abs(origArea);
    
    std::vector<Eigen::Vector4d> pts;
    pts.reserve(poly.size());
    for (unsigned int ii=0;ii<poly.size();ii++)
    {
        const Point3d &pt = poly[ii];
        // Run through the model transform
        Vector4d modPt = viewState->fullMatrix * Vector4d(pt.x(),pt.y(),pt.z(),1.0);
        // And then the projection matrix.  Now we're in clip space
        Vector4d projPt = viewState->projMatrix * modPt;
        pts.push_back(projPt);
    }
    
    // The points are in clip space, so clip!
    std::vector<Eigen::Vector4d> clipSpacePts;
    clipSpacePts.reserve(2*pts.size());
    ClipHomogeneousPolygon(pts,clipSpacePts);
    
    // Outside the viewing frustum, so ignore it
    if (clipSpacePts.empty())
        return 0.0;
    
    // Project to the screen
    std::vector<Point2d> screenPts;
    screenPts.reserve(clipSpacePts.size());
    Point2d halfFrameSize(frameSize.x()/2.0,frameSize.y()/2.0);
    for (unsigned int ii=0;ii<clipSpacePts.size();ii++)
    {
        Vector4d &outPt = clipSpacePts[ii];
        Point2d screenPt(outPt.x()/outPt.w() * halfFrameSize.x()+halfFrameSize.x(),outPt.y()/outPt.w() * halfFrameSize.y()+halfFrameSize.y());
        screenPts.push_back(screenPt);
    }
    
    float screenArea = CalcLoopArea(screenPts);
    screenArea = std::abs(screenArea);
    if (boost::math::isnan(screenArea))
        screenArea = 0.0;
    
    // Now project the screen points back into model space
    std::vector<Point3d> backPts;
    backPts.reserve(screenPts.size());
    for (unsigned int ii=0;ii<screenPts.size();ii++)
    {
        Vector4d modelPt = viewState->invProjMatrix * clipSpacePts[ii];
        Vector4d backPt = viewState->invFullMatrix * modelPt;
        backPts.push_back(Point3d(backPt.x(),backPt.y(),backPt.z()));
    }
    // Then calculate the area
    float backArea = PolygonArea(backPts,norm);
    backArea = std::abs(backArea);
    
    // Now we know how much of the original polygon made it out to the screen
    // We can scale its importance accordingly.
    // This gets rid of small slices of big tiles not getting loaded
    float scale = (backArea == 0.0) ? 1.0 : origArea / backArea;
    
    // Note: Turned off for the moment
    return std::abs(screenArea) * scale;
}

- (bool)isInside:(WhirlyKit::Point3d)pt
{
    // We should be on the inside of each plane
    for (unsigned int ii=0;ii<polys.size();ii++)
    {
        Point3d org = (polys[ii])[0];
        if ((pt-org).dot(normals[ii]) > 0.0)
            return false;
    }
    
    return true;
}

- (float)importanceForViewState:(WhirlyKitViewState *)viewState frameSize:(WhirlyKit::Point2f)frameSize;
{
    Point3d eyePos = viewState.eyePos;
    
    if (!viewState->coordAdapter->isFlat())
    {
        // If the viewer is inside the bounds, the node is maximimally important (duh)
        if ([self isInside:eyePos])
            return MAXFLOAT;

        // Make sure that we're pointed toward the eye, even a bit
        if (!surfNormals.empty())
        {
            bool isFacing = false;
            for (unsigned int ii=0;ii<surfNormals.size();ii++)
            {
                const Vector3d &surfNorm = surfNormals[ii];
                if ((isFacing |= (surfNorm.dot(eyePos) >= 0.0)))
                    break;
            }
            if (!isFacing)
                return 0.0;
        }
    }
    
    // Now work through the polygons and project each to the screen
    float totalImport = 0.0;
    for (unsigned int ii=0;ii<polys.size();ii++)
    {
        float import = PolyImportance(polys[ii], normals[ii], viewState, frameSize);
        totalImport += import;
    }
    
    return totalImport/2.0;
}

@end

namespace WhirlyKit
{

// Calculate the max pixel size for a tile
float ScreenImportance(WhirlyKitViewState *viewState,WhirlyKit::Point2f frameSize,const Point3d &notUsed,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,Mbr nodeMbr,WhirlyKit::Quadtree::Identifier &nodeIdent,NSMutableDictionary *attrs)
{
    WhirlyKitDisplaySolid *dispSolid = attrs[@"DisplaySolid"];
    if (!dispSolid)
    {
        dispSolid = [WhirlyKitDisplaySolid displaySolidWithNodeIdent:nodeIdent mbr:nodeMbr minZ:0.0 maxZ:0.0 srcSystem:srcSystem adapter:coordAdapter];
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

// This version is for volumes with height
float ScreenImportance(WhirlyKitViewState *viewState,WhirlyKit::Point2f frameSize,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,Mbr nodeMbr,double minZ,double maxZ,WhirlyKit::Quadtree::Identifier &nodeIdent,NSMutableDictionary *attrs)
{
    WhirlyKitDisplaySolid *dispSolid = attrs[@"DisplaySolid"];
    if (!dispSolid)
    {
        dispSolid = [WhirlyKitDisplaySolid displaySolidWithNodeIdent:nodeIdent mbr:nodeMbr minZ:minZ maxZ:maxZ srcSystem:srcSystem adapter:coordAdapter];
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
        NSLog(@"GlobeQuadDisplayLayer: Called viewUpdate: after being shutdown.");
        return;
    }
    
    // Just put ourselves on hold for a while
    if (!inViewState)
        return;

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
    if (renderer.framebufferWidth == 0 || renderer.framebufferHeight == 0 || viewState == nil)
    {
        [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.1];
        return;
    }    

    // If the loader isn't ready, it's up to it to wake us up when it is
    if (![loader isReady])
    {
        return;
    }

    [loader quadDisplayLayerStartUpdates:self];

    // Look for nodes to remove
    Quadtree::NodeInfo remNodeInfo;
    while (quadtree->leastImportantNode(remNodeInfo))
    {
        quadtree->removeTile(remNodeInfo.ident);
        [loader quadDisplayLayer:self unloadTile:remNodeInfo];

        didSomething = true;
    }
    
    if (!nodesForEval.empty())
    {
        // Let the loader know we're about to do some updates        
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
        
        didSomething = true;
    }

    // Let the loader know we're done with this eval step
    [loader quadDisplayLayerEndUpdates:self];

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

    // Make sure we actually evaluate them
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
}

// Tile failed to load.
// At the moment we don't care, but we won't look at the children
- (void)loader:(NSObject<WhirlyKitQuadLoader> *)loader tileDidNotLoad:(WhirlyKit::Quadtree::Identifier)tileIdent
{
    // Might get stuck here
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];    
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

- (void)wakeUp
{
    if ([NSThread currentThread] != layerThread)
    {
        [self performSelector:@selector(wakeUp) onThread:layerThread withObject:nil waitUntilDone:NO];
        return;
    }

    // Note: Might be better to check if an eval is scheduled, rather than cancel it
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
}

#pragma mark - Quad Tree Importance Delegate

- (float)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(Mbr)theMbr tree:(WhirlyKit::Quadtree *)tree attrs:(NSMutableDictionary *)attrs
{
    return [dataStructure importanceForTile:ident mbr:theMbr viewInfo:viewState frameSize:Point2f(renderer.framebufferWidth,renderer.framebufferHeight) attrs:attrs];
}

@end

