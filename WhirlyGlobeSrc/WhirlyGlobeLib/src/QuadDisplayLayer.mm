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
#import <boost/math/special_functions/fpclassify.hpp>

using namespace Eigen;
using namespace WhirlyKit;

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
float ScreenImportance(WhirlyKitViewState *viewState,WhirlyKit::Point2f frameSize,WhirlyKit::Point3f eyeVec,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,Mbr nodeMbr)
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

// Calculate the size of a rectangle projected onto the screen
float RectangleImportance(Point3f *rect,WhirlyKitViewState *viewState,WhirlyKit::Point2f frameSize,WhirlyKit::Point3f eyePos)
{
    // Project the points onto the screen
    Point2f screenPts[4];
    Mbr mbrOnScreen;
    for (unsigned int ii=0;ii<4;ii++)
    {
        CGPoint screenPt = [viewState pointOnScreenFromDisplay:rect[ii] transform:&viewState->fullMatrix frameSize:frameSize];
        mbrOnScreen.addPoint(Point2f(screenPt.x,screenPt.y));
        screenPts[ii] = Point2f(screenPt.x,screenPt.y);
    }
    Mbr frameMbr(Point2f(0,0),Point2f(frameSize.x(),frameSize.y()));
    
    // If there's no overlap, we don't care about it
    if (!mbrOnScreen.overlaps(frameMbr))
        return 0.0;
    
    Point2f ac = screenPts[2]-screenPts[0];
    Point2f bd = screenPts[3]-screenPts[1];
    float area = 0.5 * (ac.x()*bd.y()-bd.x()*ac.y());
    
    if (boost::math::isnan(area))
        area = 0.0;
        
    return std::abs(area);
}

// Cube face indices
static int FaceIndices[][4] = {{0,3,2,1},{0,1,5,4},{1,2,6,5},{3,7,6,2},{4,7,3,0},{4,5,6,7}};
    
// Calculate the max pixel size for a tile
float ScreenImportanceNew(WhirlyKitViewState *viewState,WhirlyKit::Point2f frameSize,WhirlyKit::Point3f eyeVec,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,Mbr nodeMbr)
{
    Vector3f eyePos = [viewState eyePos];

    // Start with the outline in the source coordinate system
    WhirlyKit::CoordSystem *displaySystem = coordAdapter->getCoordSystem();
    Point3f srcBounds[9];
    srcBounds[0] = Point3f(nodeMbr.ll().x(),nodeMbr.ll().y(),0.0);
    srcBounds[1] = Point3f(nodeMbr.ur().x(),nodeMbr.ll().y(),0.0);
    srcBounds[2] = Point3f(nodeMbr.ur().x(),nodeMbr.ur().y(),0.0);
    srcBounds[3] = Point3f(nodeMbr.ll().x(),nodeMbr.ur().y(),0.0);
    // Throw in the middle point, and a few edge points to account for curves (sort of)
    srcBounds[4] = Point3f((srcBounds[0].x()+srcBounds[2].x())/2.0,(srcBounds[0].y()+srcBounds[2].y())/2.0,0.0);
    for (unsigned int ii=0;ii<4;ii++)
    {
        int nextii = (ii+1)%4;
        srcBounds[ii+5] = (srcBounds[ii]+srcBounds[nextii])/2.0;
    }
    Point3f minPt(MAXFLOAT,MAXFLOAT,MAXFLOAT),maxPt(-MAXFLOAT,-MAXFLOAT,-MAXFLOAT);
    bool frontFacing = false;
    for (unsigned int ii=0;ii<9;ii++)
    {
        Point3f localPt = CoordSystemConvert(srcSystem, displaySystem, srcBounds[ii]);
        Point3f dispPt = coordAdapter->localToDisplay(localPt);
        Point3f norm = coordAdapter->normalForLocal(localPt);
        frontFacing |= (eyePos - dispPt).dot(norm) > 0.0;
        minPt.x() = std::min(minPt.x(),dispPt.x());
        minPt.y() = std::min(minPt.y(),dispPt.y());
        minPt.z() = std::min(minPt.z(),dispPt.z());
        maxPt.x() = std::max(maxPt.x(),dispPt.x());
        maxPt.y() = std::max(maxPt.y(),dispPt.y());
        maxPt.z() = std::max(maxPt.z(),dispPt.z());
    }
    // Expand things out a bit
//    Point3f diff = maxPt - minPt;
//    minPt -= diff*0.1;
//    maxPt += diff*0.1;
    
    // Now we'll form a rectilinear solid in display space
    Point3f dispBounds[8];
    dispBounds[0] = Point3f(minPt.x(),minPt.y(),minPt.z());
    dispBounds[1] = Point3f(maxPt.x(),minPt.y(),minPt.z());
    dispBounds[2] = Point3f(maxPt.x(),maxPt.y(),minPt.z());
    dispBounds[3] = Point3f(minPt.x(),maxPt.y(),minPt.z());
    dispBounds[4] = Point3f(minPt.x(),minPt.y(),maxPt.z());
    dispBounds[5] = Point3f(maxPt.x(),minPt.y(),maxPt.z());
    dispBounds[6] = Point3f(maxPt.x(),maxPt.y(),maxPt.z());
    dispBounds[7] = Point3f(minPt.x(),maxPt.y(),maxPt.z());
    
    
    // If the viewer is inside the bounds, the node is maximimally important (duh)
    if (minPt.x() <= eyePos.x() && eyePos.x() <= maxPt.x() &&
        minPt.y() <= eyePos.y() && eyePos.y() <= maxPt.y() &&
        minPt.z() <= eyePos.z() && eyePos.z() <= maxPt.z())
        return MAXFLOAT;

    // If the chunk isn't front facing, from our perspective, ditch it
    if (!frontFacing)
        return 0.0;

    // Now work through the six faces
    float totalImport = 0.0;
    for (unsigned int ii=0;ii<6;ii++)
    {
        Point3f rect[4],mid(0,0,0);
        for (unsigned int jj=0;jj<4;jj++)
        {
            rect[jj] = dispBounds[FaceIndices[ii][jj]];
            mid += rect[jj];
        }
        mid /= 4;

        // Check if the retangle is pointed away
        Vector3f norm = (rect[1]-rect[0]).cross(rect[3]-rect[0]);
        if ((eyePos - mid).dot(norm) < 0.0)
            continue;
        
        float import = RectangleImportance(rect,viewState,frameSize,eyeVec);
        if (import == MAXFLOAT)
            return import;
        // We'll scale this by the nominal pixels, since that's what the system is expecting
//        totalImport += import;
        totalImport = std::max(totalImport,import / (pixelsSquare * pixelsSquare));
//        totalImport += import / (pixelsSquare * pixelsSquare);
    }
    
//    NSLog(@"nodeMbr: (%f,%f)->(%f,%f), totalImport = %f",nodeMbr.ll().x(),nodeMbr.ll().y(),nodeMbr.ur().x(),nodeMbr.ur().y(),totalImport);
    
    return totalImport;    
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
        // Note: Debugging
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
//    NSLog(@"View state: (%f,%f,%f), height = %f",inViewState.eyePos.x(),inViewState.eyePos.y(),inViewState.eyePos.z(),inViewState->heightAboveGlobe);
    
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

#pragma mark - Quad Tree Importance Delegate

- (float)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(Mbr)theMbr tree:(WhirlyKit::Quadtree *)tree
{
    return [dataStructure importanceForTile:ident mbr:theMbr viewInfo:viewState frameSize:Point2f(renderer.framebufferWidth,renderer.framebufferHeight)];
}

@end

