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
    minUpdateDist(0.0), lineMode(false), debugMode(false), lastFlush(0.0), somethingHappened(false), firstUpdate(true), targetLevel(-1)
{
    // Note: Debugging
    greedyMode = true;
    meteredMode = false;
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
    loader->init(this,scene);
    
    if (meteredMode)
    {
        ChangeSet changes;
        loader->startUpdates(changes);
        // Note: We'll never get any changes here
    }
}
    
void QuadDisplayController::frameEnd(ChangeSet &changes)
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
    loader->endUpdates(changes);
    loader->startUpdates(changes);
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
    toPhantom.clear();
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
        localActivity = loader->numLocalFetches() != 0;
        
        if (!localActivity)
            return false;
    
    // Note: Cut short if there's network activity
    
    return true;
}

// Run the evaluation step for outstanding nodes
bool QuadDisplayController::evalStep(TimeInterval frameStart,TimeInterval frameInterval,float availableFrame,ChangeSet &changes)
{
    bool didSomething = false;
    somethingHappened = false;
    
    // If the loader isn't ready, it's up to it to wake us up when it is
    if (!loader->isReady())
        return false;
    
    if (!meteredMode)
        loader->startUpdates(changes);
    
    // Look for nodes to remove
    if (targetLevel == -1)
    {
      Quadtree::NodeInfo remNodeInfo;
      while (quadtree->leastImportantNode(remNodeInfo,false,-1))
      {
          quadtree->removeTile(remNodeInfo.ident);
          if (!remNodeInfo.phantom)
              loader->unloadTile(remNodeInfo);
        
          // Take it out of the phantom list
          QuadNodeInfoSet::iterator it = toPhantom.find(remNodeInfo);
          if (it != toPhantom.end())
              toPhantom.erase(it);
        
          didSomething = true;
      }
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
                    std::vector<Quadtree::Identifier> tilesToRemove;
                    // This is a phantom node, so just fake the loading
                    if (targetLevel != -1 && nodeInfo.ident.level < targetLevel)
                    {
                        nodeInfo.phantom = true;
//                        NSLog(@"Loading phantom tile: %d: (%d,%d)",nodeInfo.ident.level,nodeInfo.ident.x,nodeInfo.ident.y);
                        quadtree->addTile(nodeInfo, tilesToRemove);

                        // It's not "loaded" so go look at the children
                        if (nodeInfo.ident.level < maxZoom)
                        {
                            std::vector<Quadtree::NodeInfo> childNodes;
                            quadtree->generateChildren(nodeInfo.ident, childNodes);
                            nodesForEval.insert(childNodes.begin(),childNodes.end());
                        }
                    } else {
//                        NSLog(@"Loading real tile: %d: (%d,%d) import = %f",nodeInfo.ident.level,nodeInfo.ident.x,nodeInfo.ident.y,nodeInfo.importance);
                        // Tell the quad tree what we're up to
                        nodeInfo.phantom = false;
                        quadtree->addTile(nodeInfo, tilesToRemove);
                        
                        loader->loadTile(nodeInfo);
                    }
                                                        
                    // Remove the old tiles
                    for (unsigned int ii=0;ii<tilesToRemove.size();ii++)
                    {
                        Quadtree::Identifier &thisIdent = tilesToRemove[ii];
//                    NSLog(@"Quad tree removed (%d,%d,%d)",thisIdent.x,thisIdent.y,thisIdent.level);
                        
                        Quadtree::NodeInfo remNodeInfo = quadtree->generateNode(thisIdent);
                        if (!remNodeInfo.phantom)
                            loader->unloadTile(remNodeInfo);
                            
                        // Take it out of the phantom list
                        QuadNodeInfoSet::iterator it = toPhantom.find(nodeInfo);
                        if (it != toPhantom.end())
                            toPhantom.erase(it);
                    }
//            NSLog(@"Quad loaded node (%d,%d,%d) = %.4f",nodeInfo.ident.x,nodeInfo.ident.y,nodeInfo.ident.level,nodeInfo.importance);            
                } else {
                    bool isPhantom = quadtree->isPhantom(nodeInfo.ident);
                    if (targetLevel != -1)
                    {
                        // This is a phantom node that now must be loaded
                        if (isPhantom && nodeInfo.ident.level == targetLevel)
                        {
//                            NSLog(@"Reloading phantom tile: %d: (%d,%d)",nodeInfo.ident.level,nodeInfo.ident.x,nodeInfo.ident.y);
                            loader->loadTile(nodeInfo);
                            quadtree->setPhantom(nodeInfo.ident, false);

                            // Take it out of the phantom list
                            QuadNodeInfoSet::iterator it = toPhantom.find(nodeInfo);
                            if (it != toPhantom.end())
                                toPhantom.erase(it);
                        } else if (!isPhantom && nodeInfo.ident.level < targetLevel)
                        {
                            // This one needs to *be* a phantom tile
//                            NSLog(@"Unloading phantom tile: %d: (%d,%d)",nodeInfo.ident.level,nodeInfo.ident.x,nodeInfo.ident.y);
                            toPhantom.insert(nodeInfo);
                        }
                    }

                
                    // It is loaded (as far as we're concerned), so we need to know if we can traverse below that
                    if (nodeInfo.ident.level < maxZoom && (isPhantom || loader->canLoadChildrenOfTile(nodeInfo)))
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

    // Look for nodes to remove
    if (!didSomething && targetLevel != -1)
    {
        Quadtree::NodeInfo remNodeInfo;
        while (quadtree->leastImportantNode(remNodeInfo,false,-1))
        {
    //        NSLog(@"Unload tile: %d: (%d,%d) phantom = %@, import = %f",remNodeInfo.ident.level,remNodeInfo.ident.x,remNodeInfo.ident.y,(remNodeInfo.phantom ? @"YES" : @"NO"), remNodeInfo.importance);
            quadtree->removeTile(remNodeInfo.ident);
            if (!remNodeInfo.phantom)
                loader->unloadTile(remNodeInfo);

            didSomething = true;
        }
    }
    
    // Clear out the phantoms we've collected up
    if (!didSomething && targetLevel != -1)
    {
        // Is the loader doing anything?
        int activityLevel = 0;
        activityLevel += loader->numLocalFetches();
        activityLevel += loader->numNetworkFetches();
        
        if (activityLevel <= 0)
        {
            for (std::set<Quadtree::NodeInfo>::iterator it = toPhantom.begin();it != toPhantom.end(); ++it)
            {
                Quadtree::NodeInfo nodeInfo = *it;
                loader->unloadTile(nodeInfo);
                quadtree->setPhantom(nodeInfo.ident, true);
            }
        }
    }

    if (debugMode)
        dumpInfo();

    if (meteredMode)
    {
		// Let the loader know we're done with this eval step
		if (waitingForLocalLoads() || didSomething)
		{
			loader->updateWithoutFlush();
		}

		if (!didSomething)
	    {
	        // If we're not waiting for local reloads, we may be done
	        if (!meteredMode && !waitingForLocalLoads())
	        {
	            loader->endUpdates(changes);
	            somethingHappened = false;
	        }

	        // We're done waiting for local fetches.  Let the next frame boundary catch it
	        if (waitForLocalLoads && !!waitingForLocalLoads())
	        {
	            waitForLocalLoads = false;
	        }
	    }
    } else
    	loader->endUpdates(changes);

    
    somethingHappened |= didSomething;
    
        // If we're in metered mode, make sure we've got a flush here
//    if (_meteredMode)
//    {
//        TimeInterval howLong = frameEndTime-CFAbsoluteTimeGetCurrent();
//        if (howLong < 0.0)
//            howLong = 0.0;
//        [self performSelector:@selector(frameEndThread) withObject:nil afterDelay:howLong];
//    }
    
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
        {
            bool phantom = quadtree->isPhantom(tileIdent);
            if (phantom)
                fprintf(stderr,"Loaded phantom tile");
            return;
        }
        
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
void QuadDisplayController::refresh(ChangeSet &changes)
{
    nodesForEval.clear();

    // Remove nodes until we run out
    Quadtree::NodeInfo remNodeInfo;
    loader->startUpdates(changes);
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
    
    loader->startUpdates(changes);

    somethingHappened = true;
}
    
void QuadDisplayController::shutdown(ChangeSet &changes)
{
    loader->endUpdates(changes);
    
    dataStructure->shutdown();
    loader->shutdownLayer(changes);
}

void QuadDisplayController::reset(ChangeSet &changes)
{    
    waitForLocalLoads = false;
    
    // Clean out anything we might be currently evaluating
    nodesForEval.clear();
    
    // Remove nodes until we run out
    Quadtree::NodeInfo remNodeInfo;
    loader->startUpdates(changes);
    while (quadtree->leastImportantNode(remNodeInfo,true))
    {
        quadtree->removeTile(remNodeInfo.ident);
    }
    
    // Tell the tile loader to reset
    loader->reset(changes);
    
    // Now start loading again
    poke();
}

void QuadDisplayController::poke()
{
    viewUpdate(&viewState);
}


void QuadDisplayController::wakeUp()
{
    somethingHappened = true;
    adapter->adapterWakeUp();
}

// Importance callback for quad tree
double QuadDisplayController::importanceForTile(const Quadtree::Identifier &ident,const Mbr &theMbr,Quadtree *tree,Dictionary *attrs)
{
    return dataStructure->importanceForTile(ident,theMbr,&viewState,renderer->getFramebufferSize(),attrs);
}

}

