/*
 *  SceneGraphGenerator.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/24/12.
 *  Copyright 2011-2012 mousebird consulting
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

#import "SceneGraphGenerator.h"
#import "SceneRendererES.h"

using namespace Eigen;

namespace WhirlyKit
{

SceneGraphGenerator::SceneGraphGenerator()
{
    
}

SceneGraphGenerator::~SceneGraphGenerator()
{
    
}
    
void SceneGraphGenerator::traverseNode(WhirlyKitRendererFrameInfo *frameInfo,Point3f localPt,SceneGraphNodeRef node,std::set<SceneGraphNodeRef> &siblingNodes,std::vector<WhirlyKit::DrawableRef> &drawables)
{
    SceneGraphGeometryRef geom = boost::dynamic_pointer_cast<SceneGraphGeometry>(node);
    if (geom.get())
    {
        // Make sure any drawable we return is set up
        // Note: This slows things down (weird!)
//        for (unsigned int ii=0;ii<geom->drawables.size();ii++)
//            geom->drawables[ii]->setupGL(0.0);
        
        // Add it to the drawables
        for (std::vector<WhirlyKit::DrawableRef>::iterator it = geom->drawables.begin();
             it != geom->drawables.end(); ++it)
        {
            if ((*it)->isOn(frameInfo))
                drawables.push_back(*it);
        }
    } else {
        SceneGraphGroupRef group = boost::dynamic_pointer_cast<SceneGraphGroup>(node);
        if (group.get())
        {
            bool doTraverse = true;
            SceneGraphLODRef lod = boost::dynamic_pointer_cast<SceneGraphLOD>(node);
            if (lod.get())
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
                        for (std::set<SceneGraphNodeRef>::iterator siblingIt = siblingNodes.begin();
                             siblingIt != siblingNodes.end(); ++siblingIt)                            
                        {
                            SceneGraphLODRef siblingLod = boost::dynamic_pointer_cast<SceneGraphLOD>(*siblingIt);
                            if (siblingLod.get() && siblingLod != lod &&
                                (siblingLod->nodes.size() < siblingLod->numExpectedChildren))
                                doTraverse = true;
                        }
                    }
                }
            }
            
            // Traverse the children
            if (doTraverse)
            {
                for (std::set<SceneGraphNodeRef>::iterator it = group->nodes.begin();
                     it != group->nodes.end(); ++it)
                {
                    traverseNode(frameInfo, localPt, *it, group->nodes, drawables);
                }
            }
        }
    }
}
    
void SceneGraphGenerator::traverseAddNodeIDs(SceneGraphNodeRef node)
{
    SceneGraphGroupRef group = boost::dynamic_pointer_cast<SceneGraphGroup>(node);
    
    if (group.get())
    {
        allNodes.insert(group);
        for (std::set<SceneGraphNodeRef>::iterator it = group->nodes.begin();
                it != group->nodes.end(); ++it)
            traverseAddNodeIDs(*it);
    }
}
    
void SceneGraphGenerator::traverseRemNodeIDs(SceneGraphNodeRef node)
{
    SceneGraphGroupRef group = boost::dynamic_pointer_cast<SceneGraphGroup>(node);
    
    if (group.get())
    {
        allNodes.erase(group);
        for (std::set<SceneGraphNodeRef>::iterator it = group->nodes.begin();
             it != group->nodes.end(); ++it)
            traverseRemNodeIDs(*it);
    }
}

void SceneGraphGenerator::generateDrawables(WhirlyKitRendererFrameInfo *frameInfo,std::vector<WhirlyKit::DrawableRef> &drawables,std::vector<WhirlyKit::DrawableRef> &screenDrawables)
{
    // Location in the local screen coordinate system
    // Note: Need to clean this up for the non-globe case
    WhirlyGlobeView *globeView = (WhirlyGlobeView *) frameInfo.theView;
    if (![globeView isKindOfClass:[WhirlyGlobeView class]])
        return;
    
//    Point3f localPt = [globeView eyePos];
    Point3f localPt = [globeView currentUp];
    
    // Traverse the various top level nodes, gathering as we go
    for (NodeRefSet::iterator it = topNodes.begin(); it != topNodes.end(); ++it)
    {
        std::set<SceneGraphNodeRef> emptySet;
        traverseNode(frameInfo,localPt,*it,emptySet,drawables);
    }
}
    
void SceneGraphGenerator::attachSceneFragment(SimpleIdentity attachID,SceneGraphNodeRef node)
{
    if (attachID != EmptyIdentity)
    {
        SceneGraphGroupRef groupRep(new SceneGraphGroup(attachID));
        NodeSet::iterator it = allNodes.find(groupRep);
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
        SceneGraphGroupRef group = boost::dynamic_pointer_cast<SceneGraphGroup>(node);
        if (group.get())
            // Just put it at the top
            topNodes.insert(group);
    }

    // Index all the various IDs (for future attach points)
    traverseAddNodeIDs(node);
}
    
void SceneGraphGenerator::removeSceneFragment(SimpleIdentity nodeID)
{
    SceneGraphGroupRef groupRef(new SceneGraphGroup(nodeID));
    NodeSet::iterator it = allNodes.find(groupRef);
    if (it != allNodes.end())
    {
        SceneGraphGroupRef groupRef(*it);
        if (groupRef->parent)
            groupRef->parent->removeChild(groupRef);
        
        traverseRemNodeIDs(groupRef);        
        topNodes.erase(groupRef);
        
    }
//    else
//        NSLog(@"SceneGraphGenerator: Got invalid remove node request: %lu",nodeID);
}
    
void SceneGraphGenerator::dumpStats()
{
    NSLog(@"SceneGraph Generator: %ld top level nodes",topNodes.size());
    NSLog(@"SceneGraph Generator: %ld indexed nodes",allNodes.size());
}
    
SceneGraphGeneratorAddRequest::SceneGraphGeneratorAddRequest(SimpleIdentity genID,SimpleIdentity attachID,SceneGraphNodeRef node)
    : GeneratorChangeRequest(genID), attachID(attachID), node(node)
{
}

void SceneGraphGeneratorAddRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen)
{
    SceneGraphGenerator *sceneGen = (SceneGraphGenerator *)gen;
    
    sceneGen->attachSceneFragment(attachID,node);
}
    
SceneGraphGeneratorRemRequest::SceneGraphGeneratorRemRequest(SimpleIdentity genID,SimpleIdentity nodeID)
    : GeneratorChangeRequest(genID), nodeID(nodeID)
{    
}

void SceneGraphGeneratorRemRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen)
{
    SceneGraphGenerator *sceneGen = (SceneGraphGenerator *)gen;
    
    sceneGen->removeSceneFragment(nodeID);
}

}
