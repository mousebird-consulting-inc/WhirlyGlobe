/*  LabelManager.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/7/11.
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

#import "LabelRenderer.h"
#import "WhirlyGeometry.h"
#import "GlobeMath.h"
#import "ScreenSpaceBuilder.h"
#import "FontTextureManager.h"
#import "SharedAttributes.h"

#import "LabelManager.h"

using namespace Eigen;

namespace WhirlyKit
{

SingleLabel::SingleLabel() :
    isSelectable(true),
    selectID(EmptyIdentity),
    loc(0,0),
    hasMotion(false),
    endLoc(0,0),
    startTime(0),
    endTime(0),
    rotation(0),
    keepUpright(false),
    iconTexture(EmptyIdentity),
    iconSize(0,0),
    screenOffset(0,0),
    layoutSize(-1.0,-1.0),
    layoutEngine(false),
    layoutImportance(MAXFLOAT),
    layoutPlacement(0),
    maskID(EmptyIdentity),
    maskRenderTargetID(EmptyIdentity)
{
}

LabelManager::LabelManager()
    : textureAtlasSize(LabelTextureAtlasSizeDefault), maskProgID(EmptyIdentity)
{
}

SimpleIdentity LabelManager::addLabels(PlatformThreadInfo *threadInfo,
                                       const std::vector<SingleLabelRef> &labels,
                                       const LabelInfo &desc,ChangeSet &changes)
{
    return addLabels(threadInfo,labels,desc,changes,[](auto){return false;});
}

SimpleIdentity LabelManager::addLabels(PlatformThreadInfo *threadInfo,
                                       const std::vector<SingleLabelRef> &labels,
                                       const LabelInfo &desc,ChangeSet &changes,
                                       const CancelFunction& cancelFn)
{
    std::vector<SingleLabel *> unwrapLabels;
    unwrapLabels.reserve(labels.size());
    for (const auto& label: labels)
    {
        unwrapLabels.push_back(label.get());
    }
    
    return addLabels(threadInfo,unwrapLabels, desc, changes, cancelFn);
}

SimpleIdentity LabelManager::addLabels(PlatformThreadInfo *threadInfo,
                                       const std::vector<SingleLabel *> &labels,
                                       const LabelInfo &labelInfo,ChangeSet &changes)
{
    return addLabels(threadInfo,labels,labelInfo,changes,[](auto){return false;});
}

static constexpr int cancelCheckBatch = 50;

SimpleIdentity LabelManager::addLabels(PlatformThreadInfo *threadInfo,
                                       const std::vector<SingleLabel *> &labels,
                                       const LabelInfo &labelInfo,ChangeSet &changes,
                                       const CancelFunction& cancelFn)
{
    const auto fontTexManager = scene->getFontTextureManager();

    // Set up the representation (but then hand it off)
    auto labelRep = new LabelSceneRep();
    labelRep->fadeOut = (float)((labelInfo.fadeOut > 0 && labelInfo.fadeOutTime != 0) ? labelInfo.fadeOut : 0);
    
    if (maskProgID == EmptyIdentity) {
        Program *prog = scene->findProgramByName(MaplyScreenSpaceMaskShader);
        if (prog)
            maskProgID = prog->getId();
    }

    // Set up the label renderer
    LabelRenderer labelRenderer(scene,renderer,fontTexManager,&labelInfo,maskProgID);
    labelRenderer.textureAtlasSize = (int)textureAtlasSize;
    labelRenderer.coordAdapter = scene->getCoordAdapter();
    labelRenderer.labelRep = labelRep;
    labelRenderer.scene = scene;
    labelRenderer.fontTexManager = (labelInfo.screenObject ? fontTexManager : nullptr);
    labelRenderer.scale = renderer->getScale();
   
    labelRenderer.render(threadInfo, labels, changes, cancelFn);
    
    changes.insert(changes.end(),labelRenderer.changeRequests.begin(), labelRenderer.changeRequests.end());

    // Create screen shapes
    if (!labelRenderer.screenObjects.empty())
    {
        auto coordAdapter = scene->getCoordAdapter();
        ScreenSpaceBuilder ssBuild(renderer, coordAdapter, renderer->getScale());
        int n = 0;
        for (auto & screenObject : labelRenderer.screenObjects)
        {
            if (((++n) % cancelCheckBatch) == 0 && cancelFn(threadInfo))
            {
                return EmptyIdentity;
            }
            ssBuild.addScreenObject(screenObject,screenObject.getWorldLoc(),screenObject.getGeometry());
        }
        ssBuild.flushChanges(changes, labelRep->drawIDs);
    }

    // Hand over some to the layout manager
    if (const auto layoutManager = scene->getManager<LayoutManager>(kWKLayoutManager))
    {
        if (!labelRenderer.layoutObjects.empty())
        {
            for (const auto &layoutObject : labelRenderer.layoutObjects)
            {
                labelRep->layoutIDs.insert(layoutObject.getId());
            }
            layoutManager->addLayoutObjects(labelRenderer.layoutObjects);
        }
    }

    // Pass on selection data
    if (const auto selectManager = scene->getManager<SelectionManager>(kWKSelectionManager))
    {
        int n = 0;
        for (unsigned int ii=0;ii<labelRenderer.selectables2D.size();ii++)
        {
            if (((++n) % cancelCheckBatch) == 0 && cancelFn(threadInfo))
            {
                return EmptyIdentity;
            }
            auto &selectables2D = labelRenderer.selectables2D;
            RectSelectable2D &sel = selectables2D[ii];
            selectManager->addSelectableScreenRect(sel.selectID,sel.center,sel.pts,sel.minVis,sel.maxVis,sel.enable);
            labelRep->selectIDs.insert(sel.selectID);
        }
        for (unsigned int ii=0;ii<labelRenderer.movingSelectables2D.size();ii++)
        {
            if (((++n) % cancelCheckBatch) == 0 && cancelFn(threadInfo))
            {
                return EmptyIdentity;
            }
            auto &movingSelectables2D = labelRenderer.movingSelectables2D;
            auto &sel = movingSelectables2D[ii];
            selectManager->addSelectableMovingScreenRect(sel.selectID,sel.center,sel.endCenter,sel.startTime,sel.endTime,sel.pts,sel.minVis,sel.maxVis,sel.enable);
            labelRep->selectIDs.insert(sel.selectID);
        }
        for (unsigned int ii=0;ii<labelRenderer.selectables3D.size();ii++)
        {
            if (((++n) % cancelCheckBatch) == 0 && cancelFn(threadInfo))
            {
                return EmptyIdentity;
            }
            auto &selectables3D = labelRenderer.selectables3D;
            auto &sel = selectables3D[ii];
            selectManager->addSelectableRect(sel.selectID,sel.pts,sel.minVis,sel.maxVis,sel.enable);
            labelRep->selectIDs.insert(sel.selectID);
        }
    }

    SimpleIdentity labelID = labelRep->getId();
    {
        std::lock_guard<std::mutex> guardLock(lock);
        labelReps.insert(labelRep);
    }
    
    return labelID;
}

void LabelManager::changeLabel(PlatformThreadInfo *,SimpleIdentity labelID,const LabelInfo &labelInfo,ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(lock);

    LabelSceneRep dummyRep(labelID);
    const auto it = labelReps.find(&dummyRep);
    if (it != labelReps.end())
    {
        const LabelSceneRep *sceneRep = *it;
        
        for (const auto drawID : sceneRep->drawIDs)
        {
            // Changed visibility
            changes.push_back(new VisibilityChangeRequest(drawID, (float)labelInfo.minVis, (float)labelInfo.maxVis));
        }
    }
}
    
void LabelManager::enableLabels(const SimpleIDSet &labelIDs,bool enable,ChangeSet &changes)
{
    auto selectManager = scene->getManager<SelectionManager>(kWKSelectionManager);
    auto layoutManager = scene->getManager<LayoutManager>(kWKLayoutManager);

    std::lock_guard<std::mutex> guardLock(lock);

    for (const auto &labelID : labelIDs)
    {
        LabelSceneRep dummyRep(labelID);
        const auto it = labelReps.find(&dummyRep);
        if (it != labelReps.end())
        {
            LabelSceneRep *sceneRep = *it;
            for (const auto &drawID : sceneRep->drawIDs)
                changes.push_back(new OnOffChangeRequest(drawID,enable));
            if (!sceneRep->selectIDs.empty() && selectManager)
                selectManager->enableSelectables(sceneRep->selectIDs, enable);
            if (!sceneRep->layoutIDs.empty() && layoutManager)
                layoutManager->enableLayoutObjects(sceneRep->layoutIDs,enable);
        }
    }
}


void LabelManager::removeLabels(PlatformThreadInfo *inst,const SimpleIDSet &labelIDs,ChangeSet &changes)
{
    auto selectManager = scene->getManager<SelectionManager>(kWKSelectionManager);
    auto layoutManager = scene->getManager<LayoutManager>(kWKLayoutManager);
    auto fontTexManager = scene->getFontTextureManager();
    
    std::lock_guard<std::mutex> guardLock(lock);

    TimeInterval curTime = scene->getCurrentTime();
    for (const auto &lbl : labelIDs)
    {
        LabelSceneRep dummyRep(lbl);
        const auto it = labelReps.find(&dummyRep);
        if (it != labelReps.end())
        {
            LabelSceneRep *labelRep = *it;
            
            TimeInterval removeTime = 0.0;
            // We need to fade them out, then delete
            if (labelRep->fadeOut > 0.0)
            {
                for (auto id : labelRep->drawIDs)
                {
                    changes.push_back(new FadeChangeRequest(id,curTime,curTime+labelRep->fadeOut));
                }

                removeTime = curTime+labelRep->fadeOut;
            }
            
            for (auto drawID : labelRep->drawIDs)
                changes.push_back(new RemDrawableReq(drawID,removeTime));
            for (auto texID : labelRep->texIDs)
                changes.push_back(new RemTextureReq(texID,removeTime));

            if (fontTexManager)
            {
                for (auto id : labelRep->drawStrIDs)
                {
                    // Give the layout manager a little extra time so we don't pull the
                    // textures out from underneath it
                    TimeInterval fontRemoveTime = removeTime;
                    if (layoutManager && !labelRep->layoutIDs.empty())
                        fontRemoveTime = curTime + 2.0;
                    fontTexManager->removeString(inst, id, changes, fontRemoveTime);
                }
            }
            
            if (selectManager && !labelRep->selectIDs.empty())
                selectManager->removeSelectables(labelRep->selectIDs);

            // Note: Screen-space doesn't handle fade
            if (layoutManager && !labelRep->layoutIDs.empty())
                layoutManager->removeLayoutObjects(labelRep->layoutIDs);
            
            labelReps.erase(it);
            delete labelRep;
        }
    }
}

}
