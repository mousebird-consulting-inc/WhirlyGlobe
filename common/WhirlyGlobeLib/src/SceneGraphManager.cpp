/*
 *  SceneGraphManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/24/12.
 *  Copyright 2011-2017 mousebird consulting
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
#import "SceneRenderer.h"
#import "WhirlyKitLog.h"

using namespace Eigen;

namespace WhirlyKit
{
    
void SceneGraphGeometry::traverseNodeDrawables(SceneGraphManager *manage,const Point3f &localPt,const std::set<SceneGraphNode *> &siblingNodes,SimpleIDSet &toDraw)
{
    // Add it to the drawables
    // Note: Need to replace the isOn check
    toDraw.insert(drawIDs.begin(), drawIDs.end());
}

void SceneGraphGeometry::traverseRemNodeIDs(SceneGraphManager *manage,SceneGraphNodeSet &allNodes,ChangeSet &changes)
{
    for (auto drawID: drawIDs)
        manage->removeDrawable(drawID,changes);
}
    
void SceneGraphGroup::traverseNodeDrawables(SceneGraphManager *manage,const Point3f &localPt,const std::set<SceneGraphNode *> &siblingNodes,SimpleIDSet &toDraw)
{
    for (auto node : nodes)
        node->traverseNodeDrawables(manage, localPt, nodes, toDraw);
}

void SceneGraphGroup::traverseAddNodeIDs(SceneGraphManager *manage,SceneGraphNodeSet &allNodes)
{
    allNodes.insert(this);
    for (auto node : nodes)
        node->traverseAddNodeIDs(manage, allNodes);
}

void SceneGraphGroup::traverseRemNodeIDs(SceneGraphManager *manage,SceneGraphNodeSet &allNodes,ChangeSet &changes)
{
    allNodes.erase(this);
    for (auto node : nodes)
        node->traverseRemNodeIDs(manage, allNodes, changes);
}
    
void SceneGraphLOD::traverseNodeDrawables(SceneGraphManager *manage,const Point3f &localPt,const std::set<SceneGraphNode *> &siblingNodes,SimpleIDSet &toDraw)
{
    bool doTraverse = false;
    
    // Basic LOD test
    float dist = (localPt-center).norm();
    if (switchOut < dist && dist < switchIn)
        doTraverse = true;
    else
        doTraverse = false;
    
    // If we're missing children, don't traverse
    if (nodes.size() < numExpectedChildren)
        doTraverse = false;
    else {
        // Only nodes with pageable children are marked with expectedChildren
        if (numExpectedChildren == 0 && dist < switchOut)
        {
            // If a sibling node is missing children, lock this LOD on
            for (std::set<SceneGraphNode *>::iterator siblingIt = siblingNodes.begin();
                 siblingIt != siblingNodes.end(); ++siblingIt)
            {
                SceneGraphLOD *siblingLod = dynamic_cast<SceneGraphLOD *>(*siblingIt);
                if (siblingLod && siblingLod != this &&
                    (siblingLod->nodes.size() < siblingLod->numExpectedChildren))
                    doTraverse = true;
            }
        }
    }
    
    if (doTraverse)
        SceneGraphGroup::traverseNodeDrawables(manage, localPt, nodes, toDraw);
}

SceneGraphManager::SceneGraphManager()
{
}

SceneGraphManager::~SceneGraphManager()
{
    for (auto node : topNodes)
        delete node;
    topNodes.clear();
    allNodes.clear();
    drawables.clear();
}
    
void SceneGraphManager::removeDrawable(SimpleIdentity drawID,std::vector<ChangeRequest *> &changes)
{
    auto it = drawables.find(drawID);
    if (it == drawables.end())
    {
        changes.push_back(new RemDrawableReq(drawID));
        drawables.erase(it);
    }
}


void SceneGraphManager::update(ViewStateRef viewState,ChangeSet &changes)
{
    Point3f localPt = Vector3dToVector3f(viewState->eyePos);
    
    SimpleIDSet shouldBeOn;
    
    // Traverse the various top level nodes, gathering drawables that should be on
    for (auto topNode : topNodes)
    {
        std::set<SceneGraphNode *> emptySet;
        topNode->traverseNodeDrawables(this,localPt,emptySet,shouldBeOn);
    }
        
    // Figure out which ones to remove
    SimpleIDSet toRemove;
    std::set_difference(activeDrawIDs.begin(), activeDrawIDs.end(), shouldBeOn.begin(), shouldBeOn.end(),
                        std::inserter(toRemove, toRemove.end()));
    
    for (SimpleIDSet::iterator it = toRemove.begin(); it != toRemove.end(); ++it)
    {
        SimpleIdentity drawId = *it;
        changes.push_back(new OnOffChangeRequest(drawId,false));
    }
    
    // And which ones to add
    SimpleIDSet toAdd;
    std::set_difference(shouldBeOn.begin(),shouldBeOn.end(),activeDrawIDs.begin(),activeDrawIDs.end(),
                        std::inserter(toAdd, toAdd.end()));
    for (SimpleIDSet::iterator it = toAdd.begin(); it != toAdd.end(); ++it)
    {
        SimpleIdentity drawId = *it;
        changes.push_back(new OnOffChangeRequest(drawId,true));
    }
    
    activeDrawIDs = shouldBeOn;
}
        
void SceneGraphManager::addDrawable(BasicDrawable *draw,ChangeSet &changes)
{
    drawables.insert(draw->getId());
    changes.push_back(new AddDrawableReq(draw));
    changes.push_back(new OnOffChangeRequest(draw->getId(),false));
}
    
void SceneGraphManager::attachSceneFragment(SimpleIdentity attachID,SceneGraphNode *node)
{
    if (attachID != EmptyIdentity)
    {
        SceneGraphGroup groupRep(attachID);
        SceneGraphNodeSet::iterator it = allNodes.find(&groupRep);
        // Find where it's supposed to go
        if (it != allNodes.end())
        {
            (*it)->addChild(node);
        } else {
            // Otherwise, drop it on the floor
            wkLogLevel(Warn,"SceneGraphGenerator: Got orphan node.  Dropping on floor.");
            return;
        }
    } else {
        SceneGraphGroup *group = dynamic_cast<SceneGraphGroup *>(node);
        if (group)
            // Just put it at the top
            topNodes.insert(group);
    }

    // Index all the various IDs (for future attach points)
    node->traverseAddNodeIDs(this, allNodes);
}
    
void SceneGraphManager::removeSceneFragment(SimpleIdentity nodeID,ChangeSet &changes)
{
    SceneGraphGroup top(nodeID);
    SceneGraphNodeSet::iterator it = allNodes.find(&top);
    if (it != allNodes.end())
    {
        SceneGraphGroup *group = *it;
        if (group->parent)
            group->parent->removeChild(group);
        
        group->traverseRemNodeIDs(this,allNodes,changes);
        topNodes.erase(group);
        delete group;
    }
//    else
//        NSLog(@"SceneGraphGenerator: Got invalid remove node request: %lu",nodeID);
}
    
void SceneGraphManager::dumpStats()
{
    wkLogLevel(Debug,"SceneGraph Generator: %ld top level nodes",topNodes.size());
    wkLogLevel(Debug,"SceneGraph Generator: %ld indexed nodes",allNodes.size());
}

}
