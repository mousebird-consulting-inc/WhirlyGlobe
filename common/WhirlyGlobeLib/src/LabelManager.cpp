/*
 *  LabelManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/7/11.
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

#import "LabelRenderer.h"
#import "WhirlyGeometry.h"
#import "GlobeMath.h"
#import "ScreenSpaceBuilder.h"
#import "FontTextureManager.h"

#import "LabelManager.h"

using namespace Eigen;

namespace WhirlyKit
{

SingleLabel::SingleLabel()
    : isSelectable(true), selectID(EmptyIdentity), loc(0,0), rotation(0), iconTexture(EmptyIdentity),
iconSize(0,0), screenOffset(0,0), layoutSize(-1.0,-1.0), layoutEngine(false), layoutImportance(MAXFLOAT), layoutPlacement(0)
{
}

LabelManager::LabelManager()
    : textureAtlasSize(LabelTextureAtlasSizeDefault)
{
}
    
LabelManager::~LabelManager()
{
}

SimpleIdentity LabelManager::addLabels(PlatformThreadInfo *threadInfo,std::vector<SingleLabelRef> &labels,const LabelInfo &desc,ChangeSet &changes)
{
    std::vector<SingleLabel *> unwrapLabels;
    
    for (auto label: labels)
        unwrapLabels.push_back(label.get());
    
    return addLabels(threadInfo,unwrapLabels, desc, changes);
}
    
SimpleIdentity LabelManager::addLabels(PlatformThreadInfo *threadInfo,std::vector<SingleLabel *> &labels,const LabelInfo &labelInfo,ChangeSet &changes)
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();

    // Set up the representation (but then hand it off)
    LabelSceneRep *labelRep = new LabelSceneRep();
    if (labelInfo.fadeOut > 0.0 && labelInfo.fadeOutTime != 0.0)
        labelRep->fadeOut = labelInfo.fadeOut;
    else
        labelRep->fadeOut = 0.0;

    FontTextureManager *fontTexManager = scene->getFontTextureManager();
    
    // Set up the label renderer
    LabelRenderer labelRenderer(scene,fontTexManager,&labelInfo);
    labelRenderer.textureAtlasSize = textureAtlasSize;
    labelRenderer.coordAdapter = scene->getCoordAdapter();
    labelRenderer.labelRep = labelRep;
    labelRenderer.scene = scene;
    labelRenderer.fontTexManager = (labelInfo.screenObject ? fontTexManager : NULL);
    labelRenderer.scale = renderer->getScale();
   
    labelRenderer.render(threadInfo, labels, changes);
    
    changes.insert(changes.end(),labelRenderer.changeRequests.begin(), labelRenderer.changeRequests.end());

    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    
    // Create screen shapes
    if (!labelRenderer.screenObjects.empty())
    {
        ScreenSpaceBuilder ssBuild(renderer,coordAdapter,renderer->getScale());
        for (unsigned int ii=0;ii<labelRenderer.screenObjects.size();ii++)
            ssBuild.addScreenObject(labelRenderer.screenObjects[ii]);
        ssBuild.flushChanges(changes, labelRep->drawIDs);
    }
    
    // Hand over some to the layout manager
    if (layoutManager && !labelRenderer.layoutObjects.empty())
    {
        for (unsigned int ii=0;ii<labelRenderer.layoutObjects.size();ii++)
            labelRep->layoutIDs.insert(labelRenderer.layoutObjects[ii].getId());
        layoutManager->addLayoutObjects(labelRenderer.layoutObjects);
    }
    
    // Pass on selection data
    if (selectManager)
    {
        for (unsigned int ii=0;ii<labelRenderer.selectables2D.size();ii++)
        {
            std::vector<WhirlyKit::RectSelectable2D> &selectables2D = labelRenderer.selectables2D;
            RectSelectable2D &sel = selectables2D[ii];
            selectManager->addSelectableScreenRect(sel.selectID,sel.center,sel.pts,sel.minVis,sel.maxVis,sel.enable);
            labelRep->selectIDs.insert(sel.selectID);
        }
        for (unsigned int ii=0;ii<labelRenderer.movingSelectables2D.size();ii++)
        {
            std::vector<WhirlyKit::MovingRectSelectable2D> &movingSelectables2D = labelRenderer.movingSelectables2D;
            MovingRectSelectable2D &sel = movingSelectables2D[ii];
            selectManager->addSelectableMovingScreenRect(sel.selectID,sel.center,sel.endCenter,sel.startTime,sel.endTime,sel.pts,sel.minVis,sel.maxVis,sel.enable);
            labelRep->selectIDs.insert(sel.selectID);
        }
        for (unsigned int ii=0;ii<labelRenderer.selectables3D.size();ii++)
        {
            std::vector<WhirlyKit::RectSelectable3D> &selectables3D = labelRenderer.selectables3D;
            RectSelectable3D &sel = selectables3D[ii];
            selectManager->addSelectableRect(sel.selectID,sel.pts,sel.minVis,sel.maxVis,sel.enable);
            labelRep->selectIDs.insert(sel.selectID);
        }
    }

    SimpleIdentity labelID = labelRep->getId();
    {
        std::lock_guard<std::mutex> guardLock(labelLock);
        labelReps.insert(labelRep);
    }
    
    return labelID;
}

void LabelManager::changeLabel(PlatformThreadInfo *threadInfo,SimpleIdentity labelID,const LabelInfo &labelInfo,ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(labelLock);

    LabelSceneRep dummyRep(labelID);
    LabelSceneRepSet::iterator it = labelReps.find(&dummyRep);
    
    if (it != labelReps.end())
    {
        LabelSceneRep *sceneRep = *it;
        
        for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
             idIt != sceneRep->drawIDs.end(); ++idIt)
        {
            // Changed visibility
            changes.push_back(new VisibilityChangeRequest(*idIt, labelInfo.minVis, labelInfo.maxVis));
        }
    }
}
    
void LabelManager::enableLabels(SimpleIDSet labelIDs,bool enable,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    
    std::lock_guard<std::mutex> guardLock(labelLock);

    for (SimpleIDSet::iterator lit = labelIDs.begin(); lit != labelIDs.end(); ++lit)
    {
        LabelSceneRep dummyRep(*lit);
        LabelSceneRepSet::iterator it = labelReps.find(&dummyRep);
        if (it != labelReps.end())
        {
            LabelSceneRep *sceneRep = *it;
            for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
                 idIt != sceneRep->drawIDs.end(); ++idIt)
                changes.push_back(new OnOffChangeRequest(*idIt,enable));
            if (!sceneRep->selectIDs.empty() && selectManager)
                selectManager->enableSelectables(sceneRep->selectIDs, enable);
            if (!sceneRep->layoutIDs.empty() && layoutManager)
                layoutManager->enableLayoutObjects(sceneRep->layoutIDs,enable);
        }
    }
}


void LabelManager::removeLabels(PlatformThreadInfo *threadInfo,SimpleIDSet &labelIDs,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    FontTextureManager *fontTexManager = scene->getFontTextureManager();
    
    std::lock_guard<std::mutex> guardLock(labelLock);

    TimeInterval curTime = scene->getCurrentTime();
    for (SimpleIDSet::iterator lit = labelIDs.begin(); lit != labelIDs.end(); ++lit)
    {
        LabelSceneRep dummyRep(*lit);
        LabelSceneRepSet::iterator it = labelReps.find(&dummyRep);
        if (it != labelReps.end())
        {
            LabelSceneRep *labelRep = *it;
            
            TimeInterval removeTime = 0.0;
            // We need to fade them out, then delete
            if (labelRep->fadeOut > 0.0)
            {
                for (SimpleIDSet::iterator idIt = labelRep->drawIDs.begin();
                     idIt != labelRep->drawIDs.end(); ++idIt)
                    changes.push_back(new FadeChangeRequest(*idIt,curTime,curTime+labelRep->fadeOut));
                
                removeTime = curTime+labelRep->fadeOut;
            }
            
            for (SimpleIDSet::iterator idIt = labelRep->drawIDs.begin();
                 idIt != labelRep->drawIDs.end(); ++idIt)
                changes.push_back(new RemDrawableReq(*idIt,removeTime));
            for (SimpleIDSet::iterator idIt = labelRep->texIDs.begin();
                 idIt != labelRep->texIDs.end(); ++idIt)
                changes.push_back(new RemTextureReq(*idIt,removeTime));
            for (SimpleIDSet::iterator idIt = labelRep->drawStrIDs.begin();
                 idIt != labelRep->drawStrIDs.end(); ++idIt)
            {
                // Give the layout manager a little extra time so we don't pull the
                // textures out from underneath it
                TimeInterval fontRemoveTime = removeTime;
                if (layoutManager && !labelRep->layoutIDs.empty())
                    fontRemoveTime = curTime+2.0;
                if (fontTexManager)
                    fontTexManager->removeString(*idIt, changes, fontRemoveTime);
            }
            
            if (selectManager && !labelRep->selectIDs.empty())
                selectManager->removeSelectables(labelRep->selectIDs);

            // Note: Screenspace  Doesn't handle fade
            if (layoutManager && !labelRep->layoutIDs.empty())
                layoutManager->removeLayoutObjects(labelRep->layoutIDs);
            
            labelReps.erase(it);
            delete labelRep;
        }
    }
}

}
