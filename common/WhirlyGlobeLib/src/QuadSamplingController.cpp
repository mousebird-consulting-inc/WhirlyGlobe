/*  QuadSamplingController.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/15/19.
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

#import "QuadSamplingController.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{

void QuadSamplingController::start(const SamplingParams &inParams,Scene *inScene,SceneRenderer *inRenderer)
{
    params = inParams;
    scene = inScene;
    renderer = inRenderer;
    
    builder = std::make_shared<QuadTileBuilder>(params.coordSys,this);
    builder->setBuildGeom(params.generateGeom);
    builder->setCoverPoles(params.coverPoles);
    builder->setEdgeMatching(params.edgeMatching);
    builder->setSingleLevel(params.singleLevel);
    
    displayControl = std::make_shared<QuadDisplayControllerNew>(this,builder.get(),renderer);
    displayControl->setSingleLevel(params.singleLevel);
    displayControl->setKeepMinLevel(params.forceMinLevel,params.forceMinLevelHeight);
    displayControl->setLevelLoads(params.levelLoads);
    std::vector<double> importance(params.maxZoom+1);
    for (int ii=0;ii<=params.maxZoom;ii++) {
        double import = params.minImportance;
        if (ii < params.minZoom)
            import = MAXFLOAT;
        else if (ii == params.minZoom && params.minImportanceTop != MAXFLOAT) {
            import = params.minImportanceTop;
        } else if (ii < params.importancePerLevel.size() && params.importancePerLevel[ii] > -2.0) {
            import = params.importancePerLevel[ii];
        }
        importance[ii] = import;
    }
    if (params.minImportanceTop != params.minImportance && params.minImportanceTop > 0.0)
        importance[params.minZoom] = params.minImportanceTop;
    displayControl->setMinImportancePerLevel(importance);
    displayControl->setMaxTiles(params.maxTiles);
}

void QuadSamplingController::stop()
{
    builder = nullptr;
    displayControl = nullptr;
    builderDelegates.clear();
}

bool QuadSamplingController::addBuilderDelegate(PlatformThreadInfo *,QuadTileBuilderDelegateRef delegate)
{
    std::lock_guard<std::mutex> guardLock(lock);
    builderDelegates.emplace_back(std::move(delegate));
    return builderStarted;
}

void QuadSamplingController::notifyDelegateStartup(PlatformThreadInfo *threadInfo,SimpleIdentity delegateID,ChangeSet &changes)
{
    QuadTileBuilderDelegateRef delegate;
    {
        std::lock_guard<std::mutex> guardLock(lock);
        
        for (const auto& thisDelegate : builderDelegates)
            if (thisDelegate->getId() == delegateID) {
                delegate = thisDelegate;
                break;
            }
        if (!delegate)
            return;
    }
    
    delegate->setBuilder(builder.get(), displayControl.get());
    
    // Pretend we just loaded everything (to the delegate)
    WhirlyKit::TileBuilderDelegateInfo updates = builder->getLoadingState();
    delegate->builderLoad(threadInfo,builder.get(), updates, changes);
}

void QuadSamplingController::removeBuilderDelegate(PlatformThreadInfo *threadInfo,
                                                   const QuadTileBuilderDelegateRef &delegate)
{
    ChangeSet changes;
    
    std::lock_guard<std::mutex> guardLock(lock);
    const auto it = std::find(builderDelegates.begin(), builderDelegates.end(), delegate);
    if (it != builderDelegates.end())
    {
        (*it)->builderShutdown(threadInfo,builder.get(), changes);
        builderDelegates.erase(it);
    }
    
    scene->addChangeRequests(changes);
}

bool QuadSamplingController::builderIsLoading() const
{
    std::lock_guard<std::mutex> guardLock(lock);
    for (const auto& delegate : builderDelegates)
    {
        if (delegate->builderIsLoading())
        {
            return true;
        }
    }

    return false;
}

/// **** QuadDataStructure methods ****

Mbr QuadSamplingController::getValidExtents() const
{
    return params.clipBounds.valid() ? Mbr(params.clipBounds) : getTotalExtents();
}

double QuadSamplingController::importanceForTile(const QuadTreeIdentifier &ident,
                                                 const Mbr &mbr,
                                                 const ViewStateRef &viewState,
                                                 const Point2f &frameSize)
{
    const auto coordAdapter = scene->getCoordAdapter();
    // World spanning level 0 nodes sometimes have problems evaluating
    if (!coordAdapter || (params.minImportanceTop == 0.0 && ident.level == 0))
    {
        return MAXFLOAT;
    }
    
    DisplaySolidRef dispSolid;
    return ScreenImportance(viewState.get(), frameSize, viewState->eyeVec, 1,
                 params.coordSys.get(), coordAdapter, mbr, ident, dispSolid);
}

void QuadSamplingController::newViewState(ViewStateRef viewState)
{
}

bool QuadSamplingController::visibilityForTile(const QuadTreeIdentifier &ident,
                               const Mbr &mbr,
                               const ViewStateRef &viewState,
                               const Point2f &frameSize)
{
    if (ident.level == 0)
        return true;
    
    DisplaySolidRef dispSolid;
    return TileIsOnScreen(viewState.get(), frameSize,  params.coordSys.get(),
                          scene->getCoordAdapter(), mbr, ident, dispSolid);
}
    
/// **** QuadTileBuilderDelegate methods ****

void QuadSamplingController::setBuilder(QuadTileBuilder *inBuilder, QuadDisplayControllerNew *control)
{
    std::vector<QuadTileBuilderDelegateRef> delegates;
    {
        std::lock_guard<std::mutex> guardLock(lock);
        builderStarted = true;
        delegates = builderDelegates;
    }
    
    for (const auto& delegate : delegates) {
        delegate->setBuilder(inBuilder, displayControl.get());
    }

}
    
QuadTreeNew::NodeSet QuadSamplingController::builderUnloadCheck(QuadTileBuilder *inBuilder,
                                        const WhirlyKit::QuadTreeNew::ImportantNodeSet &loadTiles,
                                        const WhirlyKit::QuadTreeNew::NodeSet &unloadTiles,
                                        int targetLevel)
{
    QuadTreeNew::NodeSet toKeep;
    for (const auto& delegate : builderDelegates)
    {
        auto thisToKeep = delegate->builderUnloadCheck(inBuilder, loadTiles, unloadTiles, targetLevel);
        toKeep.insert(thisToKeep.begin(),thisToKeep.end());
    }
    
    return toKeep;
}
    
void QuadSamplingController::builderLoad(PlatformThreadInfo *threadInfo,
                                         QuadTileBuilder *inBuilder,
                                         const WhirlyKit::TileBuilderDelegateInfo &updates,
                                         ChangeSet &changes)
{
    std::vector<QuadTileBuilderDelegateRef> delegates;
    {
        std::lock_guard<std::mutex> guardLock(lock);
        delegates = builderDelegates;
    }

    // Disable the tiles.  The delegates will instance them.
    for (const auto& tile : updates.loadTiles) {
        for (auto di : tile->drawInfo) {
            changes.push_back(new OnOffChangeRequest(di.drawID,false));
        }
    }
    
    for (const auto& delegate : delegates) {
        delegate->builderLoad(threadInfo, inBuilder, updates, changes);
    }
    
    if (debugMode) {
        wkLogLevel(Debug,"SamplingLayer quadBuilder:update changes = %d",(int)changes.size());
    }
}
    
void QuadSamplingController::builderPreSceneFlush(QuadTileBuilder *inBuilder, ChangeSet &changes)
{
    std::vector<QuadTileBuilderDelegateRef> delegates;
    {
        std::lock_guard<std::mutex> guardLock(lock);
        delegates = builderDelegates;
    }

    for (const auto& delegate : delegates) {
        delegate->builderPreSceneFlush(inBuilder, changes);
    }
}
    
void QuadSamplingController::builderShutdown(PlatformThreadInfo *threadInfo, QuadTileBuilder *inBuilder, ChangeSet &changes)
{
    inBuilder = nullptr;
}

}
