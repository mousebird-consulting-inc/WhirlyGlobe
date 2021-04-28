/*  QuadDisplayControllerNew.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/13/19.
 *  Copyright 2011-2021 mousebird consulting
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
 */

#import "QuadDisplayControllerNew.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{
    
QuadDisplayControllerNew::QuadDisplayControllerNew(QuadDataStructure *dataStructure,QuadLoaderNew *loader,SceneRenderer *renderer) :
    dataStructure(dataStructure), loader(loader),
    renderer(renderer), zoomSlot(-1),
    QuadTreeNew(MbrD(dataStructure->getTotalExtents()),
    dataStructure->getMinZoom(),dataStructure->getMaxZoom())
{
    coordSys = dataStructure->getCoordSystem();
    mbr = dataStructure->getValidExtents();
    minZoom = dataStructure->getMinZoom();
    maxZoom = dataStructure->getMaxZoom();
    reportedMaxZoom = dataStructure->getReportedMaxZoom();
    maxTiles = 128;
    // TODO: Set this to 0.2 for older devices
    viewUpdatePeriod = 0.1;
    singleLevel = false;
    keepMinLevel = true;
    keepMinLevelHeight = 0.0;
    scene = renderer->getScene();
    zoomSlot = scene->retainZoomSlot();
    lastTargetLevel = -1.0;
    lastTargetDecimal = -1.0;
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

int QuadDisplayControllerNew::getMaxTiles() const
{
    return maxTiles;
}

void QuadDisplayControllerNew::setMaxTiles(int newMaxTiles)
{
    maxTiles = newMaxTiles;
}
    
TimeInterval QuadDisplayControllerNew::getViewUpdatePeriod() const
{
    return viewUpdatePeriod;
}

void QuadDisplayControllerNew::setViewUpdatePeriod(TimeInterval newVal)
{
    viewUpdatePeriod = newVal;
}
    
bool QuadDisplayControllerNew::getSingleLevel() const
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

std::vector<int> QuadDisplayControllerNew::getLevelLoads() const
{
    return levelLoads;
}

void QuadDisplayControllerNew::setLevelLoads(const std::vector<int> &newLoads)
{
    levelLoads = newLoads;
}
    
std::vector<double> QuadDisplayControllerNew::getMinImportancePerLevel() const
{
    return minImportancePerLevel;
}

void QuadDisplayControllerNew::setMinImportancePerLevel(const std::vector<double> &imports)
{
    minImportancePerLevel = imports;
    
    // Version we'll use for reaching deeper down
    if (reportedMaxZoom > maxZoom) {
        reportedMinImportancePerLevel.resize(reportedMaxZoom);
        for (unsigned int ii=0;ii<reportedMinImportancePerLevel.size();ii++)
            reportedMinImportancePerLevel[ii] = ii >= minImportancePerLevel.size() ? minImportancePerLevel[minImportancePerLevel.size()-1] : minImportancePerLevel[ii];
    }
}
    
QuadDataStructure *QuadDisplayControllerNew::getDataStructure()
{
    return dataStructure;
}

ViewStateRef QuadDisplayControllerNew::getViewState() const
{
    return viewState;
}

int QuadDisplayControllerNew::getZoomSlot() const
{
    return zoomSlot;
}
    
// Called on the LayerThread
void QuadDisplayControllerNew::start()
{
    loader->setController(this);
}
    
void QuadDisplayControllerNew::stop(PlatformThreadInfo *threadInfo,ChangeSet &changes)
{
    scene->releaseZoomSlot(zoomSlot);
    loader->quadLoaderShutdown(threadInfo,changes);
    dataStructure = nullptr;
    loader = nullptr;
    
    scene = nullptr;
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
    std::vector<double> maxRejectedImport((reportedMaxZoom > 0 ? reportedMaxZoom : maxLevel) +1,0.0);
    if (singleLevel) {
        std::tie(targetLevel,newNodes) = calcCoverageVisible(minImportancePerLevel, maxTiles, levelLoads, localKeepMinLevel, maxRejectedImport);
    } else {
        newNodes = calcCoverageImportance(minImportancePerLevel,maxTiles,true, maxRejectedImport);
        // Just take the highest level as target
        for (const auto &node : newNodes)
            targetLevel = std::max(targetLevel,node.level);
    }
    double maxRatio = targetLevel >= maxLevel ? 0.0 : maxRejectedImport[targetLevel+1];

//    wkLogLevel(Debug,"Selected level %d for %d nodes",targetLevel,(int)newNodes.size());
//    for (auto node: newNodes) {
//        wkLogLevel(Debug," %d: (%d,%d), import = %f",node.level,node.x,node.y,node.importance);
//    }
    
    QuadTreeNew::ImportantNodeSet toAdd,toUpdate;
    QuadTreeNew::NodeSet toRemove;
    
    // Need a version of new and old that has no importance values, since those change
    QuadTreeNew::NodeSet testNewNodes;
    for (const auto &node : newNodes)
        testNewNodes.insert(node);
    QuadTreeNew::NodeSet testCurrentNodes;
    for (const auto &node : currentNodes)
        testCurrentNodes.insert(node);
    
    // Nodes to remove
    for (const auto &node : currentNodes)
        if (testNewNodes.find(node) == testNewNodes.end())
            toRemove.insert(node);
    
    // Nodes to add and nodes to update importance for
    for (const auto &node : newNodes)
        if (testCurrentNodes.find(node) == testCurrentNodes.end())
            toAdd.insert(node);
        else
            toUpdate.insert(node);
    
    QuadTreeNew::NodeSet removesToKeep;
    removesToKeep = loader->quadLoaderUpdate(threadInfo, toAdd, toRemove, toUpdate, targetLevel, changes);
    
    const bool needsDelayCheck = !removesToKeep.empty();
    
    currentNodes = newNodes;
    for (const auto &node : removesToKeep) {
        currentNodes.insert(QuadTreeNew::ImportantNode(node,0.0));
    }
    
    // If we're at the max level, we may want to reach beyond
    int testTargetLevel = targetLevel;
    if (reportedMaxZoom > maxZoom && targetLevel == maxZoom) {
        int oldMaxLevel = maxLevel;
        maxLevel = reportedMaxZoom;
        QuadTreeNew::ImportantNodeSet testNodes;
        std::vector<double> maxRejectedImportLocal(reportedMaxZoom + 1, 0.0);
        std::tie(testTargetLevel,testNodes) = calcCoverageVisible(reportedMinImportancePerLevel, maxTiles, levelLoads, localKeepMinLevel, maxRejectedImportLocal);
        maxLevel = oldMaxLevel;
        maxRatio = (testTargetLevel >= maxRejectedImportLocal.size()) ? 0.0 : maxRejectedImportLocal[testTargetLevel + 1];
    }

    // We take the largest importance for a tile beyond the one we're loading and use
    //  that as a proxy for fractional zoom level
    double testTargetDecimal = maxRatio;
    // This will rarely be less than 0.25 (quad tree), so we scale accordingly
    testTargetDecimal = (testTargetDecimal-0.25)/0.75;
    testTargetDecimal = std::max(std::min(testTargetDecimal,0.99999999999),0.0);

    // If the level changed (at least 1/100) then update it
    if (testTargetLevel != lastTargetLevel ||
        std::abs(testTargetDecimal-lastTargetDecimal) > 0.01) {
        lastTargetLevel = testTargetLevel;
        lastTargetDecimal = testTargetDecimal;
        float newZoom = lastTargetLevel+lastTargetDecimal;
        if (zoomSlot > -1) {
//            wkLogLevel(Debug, "zoomSlot = %d, zoom = %f",zoomSlot,newZoom);
            changes.push_back(new SetZoomSlotReq(zoomSlot,newZoom));
        }
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
    const MbrD nodeMbrD = generateMbrForNode(node);
    const Mbr nodeMbr(nodeMbrD);
    
    // Is this a valid tile?
    if (!nodeMbr.inside(nodeMbr.mid())) {
        return -1.0;
    }
    
    // Note: Add back the mutable attributes?
    return dataStructure->importanceForTile(ident, nodeMbr, viewState, renderer->getFramebufferSize());
}

// Pure visibility check
bool QuadDisplayControllerNew::visible(const Node &node) {
    QuadTreeIdentifier ident;
    ident.level = node.level;  ident.x = node.x;  ident.y = node.y;
    Point2d ll,ur;
    const MbrD nodeMbrD = generateMbrForNode(node);
    const Mbr nodeMbr(nodeMbrD);
    
    // Is this a valid tile?
    if (!nodeMbr.inside(nodeMbr.mid())) {
        return 0.0;
    }
    
    // Note: Add back the mutable attributes?
    return dataStructure->visibilityForTile(ident, nodeMbr, viewState, renderer->getFramebufferSize());
}

    
}
