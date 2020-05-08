/*
 *  QuadDisplayControllerNew.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/13/19.
 *  Copyright 2011-2019 mousebird consulting
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

#import "QuadDisplayControllerNew.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{
    
QuadDataStructure::QuadDataStructure()
{
}

QuadDataStructure::~QuadDataStructure()
{
}

QuadDisplayControllerNew::QuadDisplayControllerNew(QuadDataStructure *dataStructure,QuadLoaderNew *loader,SceneRenderer *renderer)
    : dataStructure(dataStructure), loader(loader), renderer(renderer), QuadTreeNew(MbrD(dataStructure->getTotalExtents()),dataStructure->getMinZoom(),dataStructure->getMaxZoom())
{
    coordSys = dataStructure->getCoordSystem();
    mbr = dataStructure->getValidExtents();
    minZoom = dataStructure->getMinZoom();
    maxZoom = dataStructure->getMaxZoom();
    maxTiles = 128;
    // TODO: Set this to 0.2 for older devices
    viewUpdatePeriod = 0.1;
    singleLevel = false;
    keepMinLevel = true;
    keepMinLevelHeight = 0.0;
    scene = renderer->getScene();
}
    
QuadDisplayControllerNew::~QuadDisplayControllerNew()
{
}
    
Scene *QuadDisplayControllerNew::getScene()
{
    return scene;
}
    
SceneRenderer *QuadDisplayControllerNew::getRenderer()
{
    return renderer;
}

QuadTreeNew *QuadDisplayControllerNew::getQuadTree()
{
    return this;
}
    
CoordSystem *QuadDisplayControllerNew::getCoordSys()
{
    return coordSys;
}

int QuadDisplayControllerNew::getMaxTiles()
{
    return maxTiles;
}

void QuadDisplayControllerNew::setMaxTiles(int newMaxTiles)
{
    maxTiles = newMaxTiles;
}
    
TimeInterval QuadDisplayControllerNew::getViewUpdatePeriod()
{
    return viewUpdatePeriod;
}

void QuadDisplayControllerNew::setViewUpdatePeriod(TimeInterval newVal)
{
    viewUpdatePeriod = newVal;
}
    
bool QuadDisplayControllerNew::getSingleLevel()
{
    return singleLevel;
}

void QuadDisplayControllerNew::setSingleLevel(bool newSingleLevel)
{
    singleLevel = newSingleLevel;
}
    
void QuadDisplayControllerNew::setKeepMinLevel(bool newVal,double height)
{
    keepMinLevel = newVal;
    keepMinLevelHeight = height;
}

std::vector<int> QuadDisplayControllerNew::getLevelLoads()
{
    return levelLoads;
}

void QuadDisplayControllerNew::setLevelLoads(const std::vector<int> &newLoads)
{
    levelLoads = newLoads;
}
    
std::vector<double> QuadDisplayControllerNew::getMinImportancePerLevel()
{
    return minImportancePerLevel;
}

void QuadDisplayControllerNew::setMinImportancePerLevel(const std::vector<double> &imports)
{
    minImportancePerLevel = imports;
}
    
QuadDataStructure *QuadDisplayControllerNew::getDataStructure()
{
    return dataStructure;
}

ViewStateRef QuadDisplayControllerNew::getViewState()
{
    return viewState;
}
    
// Called on the LayerThread
void QuadDisplayControllerNew::start()
{
    loader->setController(this);
}
    
void QuadDisplayControllerNew::stop(PlatformThreadInfo *threadInfo,ChangeSet &changes)
{
    loader->quadLoaderShutdown(threadInfo,changes);
    dataStructure = NULL;
    loader = NULL;
    
    scene = NULL;
}
    
bool QuadDisplayControllerNew::viewUpdate(PlatformThreadInfo *threadInfo,ViewStateRef inViewState,ChangeSet &changes)
{
    if (!scene)
        return true;
    
    // Just put ourselves on hold for a while
    if (!inViewState)
        return true;
    
    viewState = inViewState;
    dataStructure->newViewState(viewState);
    
    // We may want to force the min level in, always
    // Or we may vary that by height
    bool localKeepMinLevel = keepMinLevel;
    if (keepMinLevelHeight != 0.0 && localKeepMinLevel) {
        WhirlyGlobe::GlobeViewStateRef globeViewState = std::dynamic_pointer_cast<WhirlyGlobe::GlobeViewState>(inViewState);
        if (globeViewState)
            localKeepMinLevel = globeViewState->heightAboveGlobe > keepMinLevelHeight;
    }
    
    // Nodes to load are different for single level vs regular loading
    QuadTreeNew::ImportantNodeSet newNodes;
    int targetLevel = -1;
    if (singleLevel) {
        std::tie(targetLevel,newNodes) = calcCoverageVisible(minImportancePerLevel, maxTiles, levelLoads, localKeepMinLevel);
    } else {
        newNodes = calcCoverageImportance(minImportancePerLevel,maxTiles,true);
        // Just take the highest level as target
        for (auto node : newNodes)
            targetLevel = std::max(targetLevel,node.level);
    }
    
//    wkLogLevel(Debug,"Selected level %d for %d nodes",targetLevel,(int)newNodes.size());
//    for (auto node: newNodes) {
//        wkLogLevel(Debug," %d: (%d,%d), import = %f",node.level,node.x,node.y,node.importance);
//    }
    
    QuadTreeNew::ImportantNodeSet toAdd,toUpdate;
    QuadTreeNew::NodeSet toRemove;
    
    // Need a version of new and old that has no importance values, since those change
    QuadTreeNew::NodeSet testNewNodes;
    for (auto node : newNodes)
        testNewNodes.insert(node);
    QuadTreeNew::NodeSet testCurrentNodes;
    for (auto node : currentNodes)
        testCurrentNodes.insert(node);
    
    // Nodes to remove
    for (auto node : currentNodes)
        if (testNewNodes.find(node) == testNewNodes.end())
            toRemove.insert(node);
    
    // Nodes to add and nodes to update importance for
    for (auto node : newNodes)
        if (testCurrentNodes.find(node) == testCurrentNodes.end())
            toAdd.insert(node);
        else
            toUpdate.insert(node);
    
    QuadTreeNew::NodeSet removesToKeep;
    removesToKeep = loader->quadLoaderUpdate(threadInfo, toAdd, toRemove, toUpdate, targetLevel,changes);
    
    bool needsDelayCheck = !removesToKeep.empty();
    
    currentNodes = newNodes;
    for (auto node : removesToKeep) {
        currentNodes.insert(QuadTreeNew::ImportantNode(node,0.0));
    }
    
    return needsDelayCheck;
}
    
void QuadDisplayControllerNew::preSceneFlush(ChangeSet &changes)
{
    loader->quadLoaderPreSceenFlush(changes);
}
    
// MARK: QuadTreeNew methods
    
// Calculate importance for a given node
double QuadDisplayControllerNew::importance(const Node &node)
{
    QuadTreeIdentifier ident;
    ident.level = node.level;  ident.x = node.x;  ident.y = node.y;
    Point2d ll,ur;
    MbrD mbrD = generateMbrForNode(node);
    Mbr mbr(mbrD);
    
    // Is this a valid tile?
    if (!mbr.inside(mbr.mid())) {
        return -1.0;
    }
    
    // Note: Add back the mutable attributes?
    return dataStructure->importanceForTile(ident, mbr, viewState, renderer->getFramebufferSize());
}

// Pure visibility check
bool QuadDisplayControllerNew::visible(const Node &node) {
    QuadTreeIdentifier ident;
    ident.level = node.level;  ident.x = node.x;  ident.y = node.y;
    Point2d ll,ur;
    MbrD mbrD = generateMbrForNode(node);
    Mbr mbr(mbrD);
    
    // Is this a valid tile?
    if (!mbr.inside(mbr.mid())) {
        return 0.0;
    }
    
    // Note: Add back the mutable attributes?
    return dataStructure->visibilityForTile(ident, mbr, viewState, renderer->getFramebufferSize());
}

    
}
