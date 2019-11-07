/*
 *  QuadSamplingController.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/15/19.
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

#import "QuadSamplingController.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{

QuadSamplingController::QuadSamplingController()
{
    debugMode = false;
    builderStarted = false;
    valid = true;
}
QuadSamplingController::~QuadSamplingController()
{
}

void QuadSamplingController::start(const SamplingParams &inParams,Scene *inScene,SceneRenderer *inRenderer)
{
    params = inParams;
    scene = inScene;
    renderer = inRenderer;
    
    builder = QuadTileBuilderRef(new QuadTileBuilder(params.coordSys,this));
    builder->setBuildGeom(params.generateGeom);
    builder->setCoverPoles(params.coverPoles);
    builder->setEdgeMatching(params.edgeMatching);
    builder->setSingleLevel(params.singleLevel);
    
    displayControl = QuadDisplayControllerNewRef(new QuadDisplayControllerNew(this,builder.get(),renderer));
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
    builder = NULL;
    displayControl = NULL;
    builderDelegates.clear();
}

int QuadSamplingController::getNumClients()
{
    return builderDelegates.size();
}

QuadDisplayControllerNewRef QuadSamplingController::getDisplayControl()
{
    return displayControl;
}

bool QuadSamplingController::addBuilderDelegate(QuadTileBuilderDelegateRef delegate)
{
    std::lock_guard<std::mutex> guardLock(lock);
    
    builderDelegates.push_back(delegate);
    
    return builderStarted;
}

void QuadSamplingController::notifyDelegateStartup(SimpleIdentity delegateID,ChangeSet &changes)
{
    QuadTileBuilderDelegateRef delegate;
    {
        std::lock_guard<std::mutex> guardLock(lock);
        
        for (auto thisDelegate : builderDelegates)
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
    delegate->builderLoad(builder.get(), updates, changes);
}

void QuadSamplingController::removeBuilderDelegate(QuadTileBuilderDelegateRef delegate)
{
    ChangeSet changes;
    
    std::lock_guard<std::mutex> guardLock(lock);
    auto it = std::find(builderDelegates.begin(), builderDelegates.end(), delegate);
    if (it != builderDelegates.end()) {
        (*it)->builderShutdown(builder.get(), changes);
        builderDelegates.erase(it);
    }
    
    scene->addChangeRequests(changes);
}

bool QuadSamplingController::builderIsLoading()
{
    std::lock_guard<std::mutex> guardLock(lock);
    for (auto delegate : builderDelegates) {
        if (delegate->builderIsLoading())
            return true;
    }

    return false;
}

/// **** QuadDataStructure methods ****

CoordSystem *QuadSamplingController::getCoordSystem()
{
    return params.coordSys.get();
}

Mbr QuadSamplingController::getTotalExtents()
{
    return Mbr(params.coordBounds);
}

Mbr QuadSamplingController::getValidExtents()
{
    if (params.clipBounds.valid()) {
        return Mbr(params.clipBounds);
    } else
        return getTotalExtents();
}

int QuadSamplingController::getMinZoom()
{
    return params.minZoom;
}

int QuadSamplingController::getMaxZoom()
{
    return params.maxZoom;
}

double QuadSamplingController::importanceForTile(const QuadTreeIdentifier &ident,
                                 const Mbr &mbr,
                                 ViewStateRef viewState,
                                 const Point2f &frameSize)
{
    // World spanning level 0 nodes sometimes have problems evaluating
    if (params.minImportanceTop == 0.0 && ident.level == 0)
        return MAXFLOAT;
    
    DisplaySolidRef dispSolid;
    double import = ScreenImportance(viewState.get(), frameSize, viewState->eyeVec, 1, params.coordSys.get(), scene->getCoordAdapter(), mbr, ident, dispSolid);
    
    return import;
}

void QuadSamplingController::newViewState(ViewStateRef viewState)
{
}

bool QuadSamplingController::visibilityForTile(const QuadTreeIdentifier &ident,
                               const Mbr &mbr,
                               ViewStateRef viewState,
                               const Point2f &frameSize)
{
    if (ident.level == 0)
        return true;
    
    DisplaySolidRef dispSolid;
    return TileIsOnScreen(viewState.get(), frameSize,  params.coordSys.get(), scene->getCoordAdapter(), mbr, ident, dispSolid);
}
    
/// **** QuadTileBuilderDelegate methods ****

void QuadSamplingController::setBuilder(QuadTileBuilder *builder,QuadDisplayControllerNew *control)
{
    std::vector<QuadTileBuilderDelegateRef> delegates;
    {
        std::lock_guard<std::mutex> guardLock(lock);
        builderStarted = true;
        delegates = builderDelegates;
    }
    
    for (auto delegate : delegates) {
        delegate->setBuilder(builder, displayControl.get());
    }

}
    
QuadTreeNew::NodeSet QuadSamplingController::builderUnloadCheck(QuadTileBuilder *builder,
                                        const WhirlyKit::QuadTreeNew::ImportantNodeSet &loadTiles,
                                        const WhirlyKit::QuadTreeNew::NodeSet &unloadTiles,
                                        int targetLevel)
{
    QuadTreeNew::NodeSet toKeep;
    for (auto delegate : builderDelegates) {
        auto thisToKeep = delegate->builderUnloadCheck(builder, loadTiles, unloadTiles, targetLevel);
        toKeep.insert(thisToKeep.begin(),thisToKeep.end());
    }
    
    return toKeep;
}
    
void QuadSamplingController::builderLoad(QuadTileBuilder *builder,
                const WhirlyKit::TileBuilderDelegateInfo &updates,
                ChangeSet &changes)
{
    std::vector<QuadTileBuilderDelegateRef> delegates;
    {
        std::lock_guard<std::mutex> guardLock(lock);
        delegates = builderDelegates;
    }

    // Disable the tiles.  The delegates will instance them.
    for (auto tile : updates.loadTiles) {
        for (auto di : tile->drawInfo) {
            changes.push_back(new OnOffChangeRequest(di.drawID,false));
        }
    }
    
    for (auto delegate : delegates) {
        delegate->builderLoad(builder, updates, changes);
    }
    
    if (debugMode) {
        wkLogLevel(Debug,"SamplingLayer quadBuilder:update changes = %d",(int)changes.size());
    }
}
    
void QuadSamplingController::builderPreSceneFlush(QuadTileBuilder *builder,ChangeSet &changes)
{
    std::vector<QuadTileBuilderDelegateRef> delegates;
    {
        std::lock_guard<std::mutex> guardLock(lock);
        delegates = builderDelegates;
    }

    for (auto delegate : delegates) {
        delegate->builderPreSceneFlush(builder, changes);
    }
}
    
void QuadSamplingController::builderShutdown(QuadTileBuilder *builder,ChangeSet &changes)
{
    builder = NULL;
}

}
