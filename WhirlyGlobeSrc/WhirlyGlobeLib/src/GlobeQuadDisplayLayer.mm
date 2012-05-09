/*
 *  GlobeQuadDisplayLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/17/12.
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

#import "GlobeQuadDisplayLayer.h"
#import "GlobeMath.h"
#import "GlobeLayerViewWatcher.h"
#import "UIImage+Stuff.h"
#import <boost/math/special_functions/fpclassify.hpp>

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

namespace WhirlyGlobe
{
// Calculate the importance for the given texel
static float calcImportance(WhirlyGlobeViewState *viewState,Point3f eyeVec,Point3f pt,Point2f pixSize,Point2f frameSize,WhirlyKit::CoordSystem *coordSys)
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
        pts3d[ii] = coordSys->localToGeocentricish(pts[ii]);
        
        // Check the normal (point in this case) against the eye vec
        if (pts3d[ii].dot(eyeVec) > 0.0)
            forwardFacing = true;
        
        CGPoint screenPt = [viewState pointOnScreenFromSphere:pts3d[ii] transform:&viewState->modelMatrix frameSize:frameSize];
        screenPts[ii] = Point2f(screenPt.x,screenPt.y);
    }
    
    // Look at area on the screen
    float area = 0.0;
    if (forwardFacing)
    {
        Point2f ac = screenPts[2]-screenPts[0];
        Point2f bd = screenPts[3]-screenPts[1];
        area = 0.5 * (ac.x()*bd.y()-bd.x()*ac.y());
    }
    
    if (boost::math::isnan(area))
        area = 0.0;
    
    //        if (area > 10000.0)
    //        {
    //            NSLog(@"Got one");
    //        }
    
    return std::abs(area);
}


// Calculate the max pixel size for a tile
float ScreenImportance(WhirlyGlobeViewState *viewState,WhirlyKit::Point2f frameSize,WhirlyKit::Point3f eyeVec,int pixelsSquare,WhirlyKit::CoordSystem *coordSys,Mbr nodeMbr)
{
    Point2f pixSize((nodeMbr.ur().x()-nodeMbr.ll().x())/pixelsSquare,(nodeMbr.ur().y()-nodeMbr.ll().y())/pixelsSquare);
    Point3f testPoints[5];
    testPoints[0] = Point3f(nodeMbr.ll().x(),nodeMbr.ll().y(),0.0);
    testPoints[1] = Point3f(nodeMbr.ur().x(),nodeMbr.ll().y(),0.0);
    testPoints[2] = Point3f(nodeMbr.ur().x(),nodeMbr.ur().y(),0.0);
    testPoints[3] = Point3f(nodeMbr.ll().x(),nodeMbr.ur().y(),0.0);
    testPoints[4] = (testPoints[0]+testPoints[2])/2.0;
    
    // Let's make sure we at least overlap the screen
    Mbr mbrOnScreen;
    for (unsigned int ii=0;ii<4;ii++)
    {
        Point3f pt3d = coordSys->localToGeocentricish(testPoints[ii]);
        
        CGPoint screenPt = [viewState pointOnScreenFromSphere:pt3d transform:&viewState->modelMatrix frameSize:frameSize];
        mbrOnScreen.addPoint(Point2f(screenPt.x,screenPt.y));
    }
    Mbr frameMbr(Point2f(0,0),Point2f(frameSize.x(),frameSize.y()));
    if (!mbrOnScreen.overlaps(frameMbr))
        return 0.0;
    
    float maxImport = 0.0;
    for (unsigned int ii=0;ii<5;ii++)
    {
        float thisImport = calcImportance(viewState,eyeVec,testPoints[ii],pixSize,frameSize,coordSys);
        maxImport = std::max(thisImport,maxImport);
    }
    
    return maxImport;
}    
    
}

@implementation WhirlyGlobeQuadDisplayLayer

@synthesize layerThread;
@synthesize scene;
@synthesize quadtree;
@synthesize coordSys;
@synthesize mbr;
@synthesize maxTiles;
@synthesize minTileArea;
@synthesize lineMode;
@synthesize drawEmpty;
@synthesize debugMode;
@synthesize dataSource;
@synthesize viewUpdatePeriod;

- (id)initWithDataSource:(NSObject<WhirlyGlobeQuadDataSource> *)inDataSource renderer:(WhirlyKitSceneRendererES1 *)inRenderer;
{
    self = [super init];
    if (self)
    {
        dataSource = inDataSource;
        coordSys = [dataSource coordSystem];
        mbr = [dataSource validExtents];
        minZoom = [dataSource minZoom];
        maxZoom = [dataSource maxZoom];
        maxTiles = 100;
        minTileArea = 1.0;
        viewUpdatePeriod = 1.0;
        quadtree = new Quadtree([dataSource totalExtents],minZoom,maxZoom,maxTiles,minTileArea,self);
        renderer = inRenderer;
        lineMode = false;
        drawEmpty = false;
        debugMode = false;
        loaders = [NSMutableArray array];
    }
    
    return self;
}

- (void)dealloc
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];

    if (quadtree)
        delete quadtree;
    if (layerThread.viewWatcher)
        [(WhirlyGlobeLayerViewWatcher *)layerThread.viewWatcher removeWatcherTarget:self selector:@selector(viewUpdate:)];
}

/// Add the given loader to the list
- (void)addLoader:(NSObject<WhirlyGlobeQuadLoader> *)loader
{
    if (![loaders containsObject:loader])
        [loaders addObject:loader];
}

/// Remove the given loader from the list
- (void)removeLoader:(NSObject<WhirlyGlobeQuadLoader> *)loader
{
    [loaders removeObject:loader];
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(Scene *)inScene
{
    layerThread = inLayerThread;
	scene = (GlobeScene *)inScene;
        
    // We want view updates, but only 1s in frequency
    if (layerThread.viewWatcher)
        [(WhirlyGlobeLayerViewWatcher *)layerThread.viewWatcher addWatcherTarget:self selector:@selector(viewUpdate:) minTime:viewUpdatePeriod];
}

- (void)shutdown
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    
    if (layerThread.viewWatcher)
        [(WhirlyGlobeLayerViewWatcher *)layerThread.viewWatcher removeWatcherTarget:self selector:@selector(viewUpdate:)];
    
    [dataSource shutdown];
    
    for (NSObject<WhirlyGlobeQuadLoader> *loader in loaders)
        [loader shutdownLayer:self scene:scene];
    
    scene = NULL;
}

// Called every so often by the view watcher
// It's here that we evaluate what to load
- (void)viewUpdate:(WhirlyGlobeViewState *)inViewState
{
    if (!scene)
    {
        NSLog(@"GlobeQuadDisplayLayer: Called viewUpdate: after begin shutdown.");
        return;
    }
    
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
    if (nodesForEval.empty())
        return;
    
    // Let the loaders know we're about to do some updates
    for (NSObject<WhirlyGlobeQuadLoader> *loader in loaders)
        [loader quadDisplayLayerStartUpdates:self];
    
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
                        
            for (NSObject<WhirlyGlobeQuadLoader> *loader in loaders)
                [loader quadDisplayLayer:self loadedTile:nodeInfo ];
                            
            // Remove the old tiles
            for (unsigned int ii=0;ii<tilesToRemove.size();ii++)
            {
                Quadtree::Identifier &thisIdent = tilesToRemove[ii];
                //                    NSLog(@"Quad tree removed (%d,%d,%d)",thisIdent.x,thisIdent.y,thisIdent.level);
                
                for (NSObject<WhirlyGlobeQuadLoader> *loader in loaders)
                {
                    Quadtree::NodeInfo remNodeInfo = quadtree->generateNode(thisIdent);
                    [loader quadDisplayLayer:self unloadedTile:remNodeInfo];           
                }
            }
//            NSLog(@"Quad loaded node (%d,%d,%d) = %.4f",nodeInfo.ident.x,nodeInfo.ident.y,nodeInfo.ident.level,nodeInfo.importance);            
        }
        
        if (nodeInfo.ident.level < maxZoom)
        {
            // Now try the children
            std::vector<Quadtree::NodeInfo> childNodes;
            quadtree->generateChildren(nodeInfo.ident, childNodes);
            nodesForEval.insert(childNodes.begin(),childNodes.end());
        }
    } else
    {
        //        NSLog(@"Quad rejecting node (%d,%d,%d) = %.4f",nodeInfo.ident.x,nodeInfo.ident.y,nodeInfo.ident.level,nodeInfo.importance);
    }
    
    // Let the loaders know we're done with this eval step
    for (NSObject<WhirlyGlobeQuadLoader> *loader in loaders)
            [loader quadDisplayLayerEndUpdates:self];    
    
//    if (debugMode)
//        [self dumpInfo];
    
    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
}

#pragma mark - Quad Tree Importance Delegate

- (float)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(Mbr)theMbr tree:(WhirlyKit::Quadtree *)tree
{
    return [dataSource importanceForTile:ident mbr:theMbr viewInfo:viewState frameSize:Point2f(renderer.framebufferWidth,renderer.framebufferHeight)];
}

@end

