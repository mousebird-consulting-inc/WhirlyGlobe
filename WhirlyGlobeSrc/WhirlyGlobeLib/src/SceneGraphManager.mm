/*
 *  SceneGraphManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/24/12.
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

#import "SceneGraphManager.h"
#import "SceneRendererES.h"

using namespace Eigen;

namespace WhirlyKit
{

SceneGraphManager::SceneGraphManager(DynamicDrawableAtlas *drawAtlas) : drawAtlas(drawAtlas)
{
}

SceneGraphManager::~SceneGraphManager()
{
    drawAtlas = NULL;
    for (NodeSet::iterator it = allNodes.begin(); it != allNodes.end(); ++it)
        delete *it;
    topNodes.clear();
    allNodes.clear();
    for (std::set<BasicDrawable *,IdentifiableSorter>::iterator it = drawables.begin();
         it != drawables.end(); ++it)
        delete *it;
    drawables.clear();
}
    
void SceneGraphManager::traverseNode(Point3f localPt,SceneGraphNode *node,std::set<SceneGraphNode *> &siblingNodes,SimpleIDSet &toDraw)
{
    SceneGraphGeometry *geom = dynamic_cast<SceneGraphGeometry *>(node);
    if (geom)
    {
        // Add it to the drawables
        // Note: Need to replace the isOn check
        toDraw.insert(geom->drawIDs.begin(), geom->drawIDs.end());
    } else {
        SceneGraphGroup *group = dynamic_cast<SceneGraphGroup *>(node);
        if (group)
        {
            bool doTraverse = true;
            SceneGraphLOD *lod = dynamic_cast<SceneGraphLOD *>(node);
            if (lod)
            {
                // Basic LOD test
                float dist = (localPt-lod->center).norm();
                if (lod->switchOut < dist && dist < lod->switchIn)
                    doTraverse = true;
                else
                    doTraverse = false;
                
                // If we're missing children, don't traverse
                if (lod->nodes.size() < lod->numExpectedChildren)
                    doTraverse = false;
                else {
                    // Only nodes with pageable children are marked with expectedChildren
                    if (lod->numExpectedChildren == 0 && dist < lod->switchOut)
                    {
                        // If a sibling node is missing children, lock this LOD on
                        for (std::set<SceneGraphNode *>::iterator siblingIt = siblingNodes.begin();
                             siblingIt != siblingNodes.end(); ++siblingIt)                            
                        {
                            SceneGraphLOD *siblingLod = dynamic_cast<SceneGraphLOD *>(*siblingIt);
                            if (siblingLod && siblingLod != lod &&
                                (siblingLod->nodes.size() < siblingLod->numExpectedChildren))
                                doTraverse = true;
                        }
                    }
                }
            }
            
            // Traverse the children
            if (doTraverse)
            {
                for (std::set<SceneGraphNode *>::iterator it = group->nodes.begin();
                     it != group->nodes.end(); ++it)
                    traverseNode(localPt, *it, group->nodes, toDraw);
            }
        }
    }
}
    
void SceneGraphManager::traverseAddNodeIDs(SceneGraphNode *node)
{
    SceneGraphGroup *group = dynamic_cast<SceneGraphGroup *>(node);
    
    if (group)
    {
        allNodes.insert(group);
        for (std::set<SceneGraphNode *>::iterator it = group->nodes.begin();
                it != group->nodes.end(); ++it)
            traverseAddNodeIDs(*it);
    }
}
    
void SceneGraphManager::traverseRemNodeIDs(SceneGraphNode *node,ChangeSet &changes)
{
    SceneGraphGroup *group = dynamic_cast<SceneGraphGroup *>(node);
    
    if (group)
    {
        allNodes.erase(group);
        for (std::set<SceneGraphNode *>::iterator it = group->nodes.begin();
             it != group->nodes.end(); ++it)
            traverseRemNodeIDs(*it,changes);
        
    } else {
        // If this is a geometry node, get rid of the drawables too
        SceneGraphGeometry *geom = dynamic_cast<SceneGraphGeometry *>(node);
        if (geom)
        {
            for (SimpleIDSet::iterator it = geom->drawIDs.begin(); it != geom->drawIDs.end(); ++it)
                removeDrawable(*it,changes);
        }
    }
}
    
Drawable *SceneGraphManager::getDrawable(SimpleIdentity drawID)
{
    if (drawAtlas)
        return nil;
    
    BasicDrawable dumbDraw("None");
    dumbDraw.setId(drawID);
    std::set<BasicDrawable *,IdentifiableSorter>::iterator it = drawables.find(&dumbDraw);
    if (it == drawables.end())
        return NULL;
    else
        return *it;
}
    
void SceneGraphManager::removeDrawable(SimpleIdentity drawID,std::vector<ChangeRequest *> &changes)
{
    if (drawAtlas)
    {
        drawAtlas->removeDrawable(drawID, changes);
    } else {
        BasicDrawable dumbDraw("None");
        dumbDraw.setId(drawID);
        std::set<BasicDrawable *,IdentifiableSorter>::iterator it = drawables.find(&dumbDraw);
        if (it != drawables.end())
        {
            Drawable *draw = *it;
            drawables.erase(it);
            changes.push_back(new RemDrawableReq(draw->getId()));
        }
    }
}


void SceneGraphManager::update(WhirlyKitViewState *viewState,ChangeSet &changes)
{
    Point3f localPt = Vector3dToVector3f([viewState eyePos]);
    
    if (drawAtlas)
    {
        // If any drawables got added recently, get them in the altas
        for (std::set<BasicDrawable *,IdentifiableSorter>::iterator it = drawables.begin();
             it != drawables.end(); ++it)
        {
            BasicDrawable *draw = *it;
            drawAtlas->addDrawable(*it, changes);
            drawAtlas->setEnableDrawable(draw->getId(), false);
            delete draw;
        }
        drawables.clear();
    }
    
    SimpleIDSet shouldBeOn;
    
    // Traverse the various top level nodes, gathering drawables that should be on
    for (NodeSet::iterator it = topNodes.begin(); it != topNodes.end(); ++it)
    {
        std::set<SceneGraphNode *> emptySet;
        traverseNode(localPt,*it,emptySet,shouldBeOn);
    }
        
    // Figure out which ones to remove
    SimpleIDSet toRemove;
    std::set_difference(activeDrawIDs.begin(), activeDrawIDs.end(), shouldBeOn.begin(), shouldBeOn.end(),
                        std::inserter(toRemove, toRemove.end()));
    
    for (SimpleIDSet::iterator it = toRemove.begin(); it != toRemove.end(); ++it)
    {
        SimpleIdentity drawId = *it;
        if (drawAtlas)
            drawAtlas->setEnableDrawable(drawId, false);
        else
            changes.push_back(new OnOffChangeRequest(drawId,false));
    }
    
    // And which ones to add
    SimpleIDSet toAdd;
    std::set_difference(shouldBeOn.begin(),shouldBeOn.end(),activeDrawIDs.begin(),activeDrawIDs.end(),
                        std::inserter(toAdd, toAdd.end()));
    for (SimpleIDSet::iterator it = toAdd.begin(); it != toAdd.end(); ++it)
    {
        SimpleIdentity drawId = *it;
        if (drawAtlas)
            drawAtlas->setEnableDrawable(drawId, true);
        else
            changes.push_back(new OnOffChangeRequest(drawId,true));
    }
    
    activeDrawIDs = shouldBeOn;
}
        
void SceneGraphManager::addDrawable(BasicDrawable *draw,ChangeSet &changes)
{
    drawables.insert(draw);
    if (!drawAtlas)
    {
        changes.push_back(new AddDrawableReq(draw));
        changes.push_back(new OnOffChangeRequest(draw->getId(),false));
    }
}
    
void SceneGraphManager::attachSceneFragment(SimpleIdentity attachID,SceneGraphNode *node)
{
    if (attachID != EmptyIdentity)
    {
        SceneGraphGroup groupRep(attachID);
        NodeSet::iterator it = allNodes.find(&groupRep);
        // Find where it's supposed to go
        if (it != allNodes.end())
        {
            (*it)->addChild(node);
        } else {
            // Otherwise, drop it on the floor
            NSLog(@"SceneGraphGenerator: Got orphan node.  Dropping on floor.");
            return;
        }
    } else {
        SceneGraphGroup *group = dynamic_cast<SceneGraphGroup *>(node);
        if (group)
            // Just put it at the top
            topNodes.insert(group);
    }

    // Index all the various IDs (for future attach points)
    traverseAddNodeIDs(node);
}
    
void SceneGraphManager::removeSceneFragment(SimpleIdentity nodeID,ChangeSet &changes)
{
    SceneGraphGroup top(nodeID);
    NodeSet::iterator it = allNodes.find(&top);
    if (it != allNodes.end())
    {
        SceneGraphGroup *group = *it;
        if (group->parent)
            group->parent->removeChild(group);
        
        traverseRemNodeIDs(group,changes);
        topNodes.erase(group);
        delete group;
    }
//    else
//        NSLog(@"SceneGraphGenerator: Got invalid remove node request: %lu",nodeID);
}
    
void SceneGraphManager::dumpStats()
{
    NSLog(@"SceneGraph Generator: %ld top level nodes",topNodes.size());
    NSLog(@"SceneGraph Generator: %ld indexed nodes",allNodes.size());
}

}
