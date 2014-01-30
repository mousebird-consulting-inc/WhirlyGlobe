/*
 *  QuadDisplayLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/17/12.
 *  Copyright 2011-2013 mousebird consulting
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

#import "QuadDisplayController.h"
#import "GlobeMath.h"
#import "FlatMath.h"
#import "VectorData.h"

using namespace Eigen;

namespace WhirlyKit
{
        
QuadDisplayController::QuadDisplayController(QuadDataStructure *dataStructure,QuadLoader *loader,QuadDisplayControllerAdapter *adapter)
: adapter(adapter), dataStructure(dataStructure), loader(loader), quadtree(NULL),
    scene(NULL), renderer(NULL), coordSys(dataStructure->getCoordSystem()), mbr(dataStructure->getValidExtents()),
    minImportance(1.0), maxTiles(128), minZoom(dataStructure->getMinZoom()), maxZoom(dataStructure->getMaxZoom()),
    greedyMode(false), meteredMode(true), waitForLocalLoads(false),fullLoad(false), fullLoadTimeout(4.0), viewUpdatePeriod(0.1),
    minUpdateDist(0.0), lineMode(false), debugMode(false), lastFlush(0.0), somethingHappened(false), firstUpdate(true)
{
}

QuadDisplayController::~QuadDisplayController()
{
    if (quadtree)
        delete quadtree;
}
    
void QuadDisplayController::init(Scene *inScene,SceneRendererES *inRenderer)
{
    scene = inScene;
    renderer = inRenderer;

    quadtree = new Quadtree(dataStructure->getTotalExtents(),minZoom,maxZoom,maxTiles,minImportance,this);
    
    if (meteredMode)
        loader->startUpdates();
}
    
void QuadDisplayController::frameEnd()
{
    TimeInterval now = TimeGetCurrent();
    
    // We'll hold off for local loads...up to a point
    bool forcedFlush = false;
    if (now - lastFlush < fullLoadTimeout)
    {
        if (waitingForLocalLoads())
            return;
    } else
        forcedFlush = true;
    
    // Flush out the updates and immediately start new ones
    loader->endUpdates();
    loader->startUpdates();
    // If we forced out a flush, we can wait for more local loads
    if (!forcedFlush)
        waitForLocalLoads = false;
    lastFlush = now;
   
    somethingHappened = false;
}
    
void QuadDisplayController::viewUpdate(ViewState *inViewState)
{
    // Check if we should even be doing an update
    if (!loader->shouldUpdate(inViewState,firstUpdate))
        return;
    firstUpdate = false;
    
    dataStructure->newViewState(inViewState);
    
    viewState = *inViewState;
    nodesForEval.clear();
    quadtree->reevaluateNodes();
    
    // Add everything at the minLevel back in
    for (int ix=0;ix<1<<minZoom;ix++)
        for (int iy=0;iy<1<<minZoom;iy++)
        {
            Quadtree::NodeInfo thisNode = quadtree->generateNode(Quadtree::Identifier(ix,iy,minZoom));
            nodesForEval.insert(thisNode);
        }

    if (fullLoad)
        waitForLocalLoads = true;
}

// Dump out info about what we've got loaded in
void QuadDisplayController::dumpInfo()
{
//    NSLog(@"***Loaded Tiles***");
//    for (LoadedTileSet::iterator it = tileSet.begin();
//         it != tileSet.end(); ++it)
//    {
//        (*it)->Print(quadtree);
//    }
//    NSLog(@"******");
    
//    _quadtree->Print();
}

// Check if we're waiting for local (e.g. fast) loads to finish
bool QuadDisplayController::waitingForLocalLoads()
{
    if (!waitForLocalLoads)
        return false;
    
    // Check for local fetches ongoing
    bool localActivity = !nodesForEval.empty();
    if (!localActivity)
        localActivity = loader->localFetches() != 0;
        
        if (!localActivity)
            return false;
    
    // Note: Cut short if there's network activity
    
    return true;
}

// Run the evaluation step for outstanding nodes
bool QuadDisplayController::evalStep(TimeInterval frameStart,TimeInterval frameInterval,float availableFrame)
{
    bool didSomething = false;
    
    // If the loader isn't ready, it's up to it to wake us up when it is
    if (!loader->isReady())
        return false;
    
    if (!meteredMode)
        loader->startUpdates();
    
    // Look for nodes to remove
    Quadtree::NodeInfo remNodeInfo;
    while (quadtree->leastImportantNode(remNodeInfo))
    {
        quadtree->removeTile(remNodeInfo.ident);
        loader->unloadTile(remNodeInfo);
        
        didSomething = true;
    }
    
    if (!nodesForEval.empty())
    {
        // Let the loader know we're about to do some updates        
        while (!nodesForEval.empty())
        {
            if (!loader->isReady())
            	break;

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
                                
                    loader->loadTile(nodeInfo);
                                    
                    // Remove the old tiles
                    for (unsigned int ii=0;ii<tilesToRemove.size();ii++)
                    {
                        Quadtree::Identifier &thisIdent = tilesToRemove[ii];
//                    NSLog(@"Quad tree removed (%d,%d,%d)",thisIdent.x,thisIdent.y,thisIdent.level);
                        
                        Quadtree::NodeInfo remNodeInfo = quadtree->generateNode(thisIdent);
                        loader->unloadTile(remNodeInfo);
                    }
//            NSLog(@"Quad loaded node (%d,%d,%d) = %.4f",nodeInfo.ident.x,nodeInfo.ident.y,nodeInfo.ident.level,nodeInfo.importance);            
                } else {
                    // It is loaded (as far as we're concerned), so we need to know if we can traverse below that
                    if (nodeInfo.ident.level < maxZoom && loader->canLoadChildrenOfTile(nodeInfo))
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

            // If we're not in greedy mode, we're only doing this for a certain time period, then we'll hand off
            TimeInterval now = TimeGetCurrent();
            if (!greedyMode && meteredMode)
            {
                if (now-frameStart > availableFrame*frameInterval || !loader->isReady())
                    break;
            }
        }
        
        didSomething = true;
    }

    // Let the loader know we're done with this eval step
    if (meteredMode || waitingForLocalLoads() || didSomething)
    {
        loader->updateWithoutFlush();
    } else
        loader->endUpdates();

    if (debugMode)
        dumpInfo();

    if (!didSomething)
    {
        // If we're not waiting for local reloads, we may be done
        if (!meteredMode && !waitingForLocalLoads())
        {
            loader->endUpdates();
            somethingHappened = false;
        }
        
        // We're done waiting for local fetches.  Let the next frame boundary catch it
        if (waitForLocalLoads && !!waitingForLocalLoads())
        {
            waitForLocalLoads = false;
        }
    }
    
    somethingHappened |= didSomething;
    
    return somethingHappened;
}

// This is called by the loader when it finished loading a tile
// Once loaded we can try the children
void QuadDisplayController::tileDidLoad(const WhirlyKit::Quadtree::Identifier &tileIdent)
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
    }
    
    somethingHappened = true;

    adapter->adapterTileDidLoad(tileIdent);
}

// Tile failed to load.
// At the moment we don't care, but we won't look at the children
void QuadDisplayController::tileDidNotLoad(const Quadtree::Identifier &tileIdent)
{
    somethingHappened = true;

    adapter->adapterTileDidNotLoad(tileIdent);
}

// Clear out all the existing tiles and start over
void QuadDisplayController::refresh()
{
    nodesForEval.clear();

    // Remove nodes until we run out
    Quadtree::NodeInfo remNodeInfo;
    loader->startUpdates();
    while (quadtree->leastImportantNode(remNodeInfo,true))
    {
        
        quadtree->removeTile(remNodeInfo.ident);
        loader->unloadTile(remNodeInfo);
    }
    waitForLocalLoads = true;
    
    // Add everything at the minLevel back in
    for (int ix=0;ix<1<<minZoom;ix++)
        for (int iy=0;iy<1<<minZoom;iy++)
        {
            Quadtree::NodeInfo thisNode = quadtree->generateNode(Quadtree::Identifier(ix,iy,minZoom));
            nodesForEval.insert(thisNode);
        }
    
    loader->startUpdates();

    somethingHappened = true;
}
    
void QuadDisplayController::shutdown()
{
    loader->endUpdates();
    
    dataStructure->shutdown();
    loader->shutdownLayer();
}

void QuadDisplayController::wakeUp()
{
    somethingHappened = true;
}

// Importance callback for quad tree
double QuadDisplayController::importanceForTile(const Quadtree::Identifier &ident,const Mbr &theMbr,Quadtree *tree,Dictionary *attrs)
{
    return dataStructure->importanceForTile(ident,theMbr,&viewState,renderer->getFramebufferSize(),attrs);
}

}

