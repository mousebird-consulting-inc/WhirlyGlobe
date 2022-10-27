/*  QuadDisplayControllerNew.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/13/19.
 *  Copyright 2011-2022 mousebird consulting
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
    QuadTreeNew(dataStructure->getTotalExtents(),
                dataStructure->getMinZoom(),
                dataStructure->getMaxZoom()),
    dataStructure(dataStructure),
    loader(loader),
    renderer(renderer)
{
    mbr = dataStructure->getValidExtents();
    coordSys = dataStructure->getCoordSystem();
    minZoom = dataStructure->getMinZoom();
    maxZoom = dataStructure->getMaxZoom();
    reportedMaxZoom = dataStructure->getReportedMaxZoom();
    scene = renderer->getScene();
    if (scene)
    {
        zoomSlot = scene->retainZoomSlot();
    }
}

Scene *QuadDisplayControllerNew::getScene() const
{
    return scene;
}
    
SceneRenderer *QuadDisplayControllerNew::getRenderer() const
{
    return renderer;
}

QuadTreeNew *QuadDisplayControllerNew::getQuadTree()
{
    return this;
}
    
CoordSystem *QuadDisplayControllerNew::getCoordSys() const
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

void QuadDisplayControllerNew::setMBRScaling(double newScale)
{
    mbrScaling = newScale;
}

void QuadDisplayControllerNew::setLevelLoads(const std::vector<int> &newLoads)
{
    levelLoads = newLoads;
}
    
std::vector<double> QuadDisplayControllerNew::getMinImportancePerLevel() const
{
    return minImportancePerLevel;
}

void QuadDisplayControllerNew::setMinImportancePerLevel(std::vector<double> imports)
{
    minImportancePerLevel = std::move(imports);
    
    // Version we'll use for reaching deeper down
    if (reportedMaxZoom > maxZoom)
    {
        reportedMinImportancePerLevel.resize(reportedMaxZoom + 1);
        for (unsigned int ii=0;ii<reportedMinImportancePerLevel.size();ii++)
        {
            const auto idx = (ii >= minImportancePerLevel.size()) ? (minImportancePerLevel.size() - 1) : ii;
            reportedMinImportancePerLevel[ii] = minImportancePerLevel[idx];
        }
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
    running = true;
}

void QuadDisplayControllerNew::stop(PlatformThreadInfo *threadInfo,ChangeSet &changes)
{
    running = false;
    scene->releaseZoomSlot(zoomSlot);
    loader->quadLoaderShutdown(threadInfo,changes);
    dataStructure = nullptr;
    loader = nullptr;
    scene = nullptr;
    coordSys = nullptr;
}

bool QuadDisplayControllerNew::viewUpdate(PlatformThreadInfo *threadInfo,const ViewStateRef &inViewState,ChangeSet &changes)
{
    // Just put ourselves on hold for a while
    if (!running || !scene || !inViewState)
    {
        return true;
    }

    viewState = inViewState;
    dataStructure->newViewState(viewState);

    // View state update can be slow, check again
    if (!running)
    {
        return true;
    }

    // We may want to force the min level in, always
    // Or we may vary that by height
    bool localKeepMinLevel = keepMinLevel;
    if (keepMinLevelHeight != 0.0 && localKeepMinLevel)
    {
        if (const auto globeViewState = dynamic_cast<WhirlyGlobe::GlobeViewState*>(inViewState.get()))
        {
            localKeepMinLevel = globeViewState->heightAboveGlobe > keepMinLevelHeight;
        }
    }

    // Nodes to load are different for single level vs regular loading
    QuadTreeNew::ImportantNodeSet newNodes;
    int targetLevel = -1;
    std::vector<double> maxRejectedImport(std::max(reportedMaxZoom, maxLevel) + 1,0.0);
    if (singleLevel)
    {
        std::tie(targetLevel,newNodes) = calcCoverageVisible(minImportancePerLevel, maxTiles, levelLoads, localKeepMinLevel, maxRejectedImport);
    }
    else
    {
        newNodes = calcCoverageImportance(minImportancePerLevel,maxTiles,true, maxRejectedImport);

        // Just take the highest level as target
        for (const auto &node : newNodes)
        {
            targetLevel = std::max(targetLevel,node.level);
        }
    }

    if (!running)
    {
        return false;
    }

    double maxRatio = (targetLevel >= maxLevel) ? 0.0 : maxRejectedImport[targetLevel+1];

//    wkLogLevel(Debug,"Selected level %d for %d nodes",targetLevel,(int)newNodes.size());
//    for (auto node: newNodes) {
//        wkLogLevel(Debug," %d: (%d,%d), import = %f",node.level,node.x,node.y,node.importance);
//    }
    
    QuadTreeNew::ImportantNodeSet toAdd,toUpdate;
    QuadTreeNew::NodeSet toRemove;
    
    // Need a version of new and old that has no importance values, since those change
    QuadTreeNew::NodeSet testNewNodes;
    for (const auto &node : newNodes)
    {
        testNewNodes.insert(node);
    }

    QuadTreeNew::NodeSet testCurrentNodes;
    for (const auto &node : currentNodes)
    {
        testCurrentNodes.insert(node);
    }
    
    // Nodes to remove
    for (const auto &node : currentNodes)
    {
        if (testNewNodes.find(node) == testNewNodes.end())
        {
            toRemove.insert(node);
        }
    }

    // Nodes to add and nodes to update importance for
    for (const auto &node : newNodes)
    {
        if (testCurrentNodes.find(node) == testCurrentNodes.end())
        {
            toAdd.insert(node);
        }
        else    // todo: test whether the importance value actually changed?
        {
            toUpdate.insert(node);
        }
    }
    
    const QuadTreeNew::NodeSet removesToKeep =
        loader->quadLoaderUpdate(threadInfo, toAdd, toRemove, toUpdate, targetLevel, changes);

    const bool needsDelayCheck = !removesToKeep.empty();
    
    currentNodes = newNodes;
    for (const auto &node : removesToKeep)
    {
        currentNodes.emplace(node,0.0);
    }

    // If we're at the max level, we may want to reach beyond
    int testTargetLevel = targetLevel;
    if (reportedMaxZoom > maxZoom && targetLevel == maxZoom)
    {
        const int oldMaxLevel = maxLevel;
        maxLevel = reportedMaxZoom;
        QuadTreeNew::ImportantNodeSet testNodes;
        std::vector<double> maxRejectedImportLocal(reportedMaxZoom + 1, 0.0);
        std::tie(testTargetLevel,testNodes) = calcCoverageVisible(reportedMinImportancePerLevel, maxTiles, levelLoads, localKeepMinLevel, maxRejectedImportLocal);
        maxLevel = oldMaxLevel;
        maxRatio = (testTargetLevel + 1 < maxRejectedImportLocal.size()) ? maxRejectedImportLocal[testTargetLevel + 1] : 0.0;
    }

    // We take the largest importance for a tile beyond the one we're loading and use
    //  that as a proxy for fractional zoom level
    double testTargetDecimal = maxRatio;
    // This will rarely be less than 0.25 (quad tree), so we scale accordingly
    testTargetDecimal = (testTargetDecimal-0.25)/0.75;
    testTargetDecimal = std::max(std::min(testTargetDecimal,0.99999999999),0.0);

    // If the level changed (at least 1/100) then update it
    if (testTargetLevel != (int)lastTargetLevel ||
        std::abs(testTargetDecimal-lastTargetDecimal) > 0.01)
    {
        lastTargetLevel = (float)testTargetLevel;
        lastTargetDecimal = (float)testTargetDecimal;
        if (zoomSlot > -1)
        {
            const float newZoom = lastTargetLevel+lastTargetDecimal;
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
    MbrD nodeMbrD = generateMbrForNode(node);

    // Scale the bounding box, possibly
    if (mbrScaling != 1.0)
        nodeMbrD.expandByFraction(mbrScaling-1.0);

    const Mbr nodeMbr(nodeMbrD);
    // Is this a valid tile?
    if (!nodeMbr.inside(nodeMbr.mid())) {
        return -1.0;
    }
    
    // Note: Add back the mutable attributes?
    const QuadTreeIdentifier ident(node.x, node.y, node.level);
    return dataStructure->importanceForTile(ident, nodeMbr, viewState, renderer->getFramebufferSize());
}

// Pure visibility check
bool QuadDisplayControllerNew::visible(const Node &node) {
    MbrD nodeMbrD = generateMbrForNode(node);

    // Scale the bounding box, possibly
    if (mbrScaling != 1.0)
        nodeMbrD.expandByFraction(mbrScaling-1.0);

    const Mbr nodeMbr(nodeMbrD);
    // Is this a valid tile?
    if (!nodeMbr.inside(nodeMbr.mid())) {
        return false;
    }
    
    // Note: Add back the mutable attributes?
    const QuadTreeIdentifier ident(node.x, node.y, node.level);
    return dataStructure->visibilityForTile(ident, nodeMbr, viewState, renderer->getFramebufferSize());
}

    
}
