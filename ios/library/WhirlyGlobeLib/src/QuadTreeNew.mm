/*
 *  QuadTreeNew.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/26/18.
 *  Copyright 2012-2018 Saildrone Inc
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

#import "QuadTreeNew.h"

namespace WhirlyKit
{
    
bool QuadTreeIdentifier::operator<(const QuadTreeIdentifier &that) const
{
    if (level == that.level)
    {
        if (x == that.x)
        {
            return y < that.y;
        } else {
            return x < that.x;
        }
    }
    
    return level < that.level;
}

bool QuadTreeIdentifier::operator==(const QuadTreeIdentifier &that) const
{
    return level == that.level && x == that.x && y == that.y;
}
    
bool QuadTreeNew::Node::operator<(const WhirlyKit::QuadTreeNew::Node &that) const
{
    if (level == that.level) {
        if (y == that.y) {
            return x < that.x;
        }
        return y < that.y;
    }
    
    return level < that.level;
}
    
bool QuadTreeNew::Node::operator==(const WhirlyKit::QuadTreeNew::Node &that) const
{
    return level == that.level && x == that.x && y == that.y;
}

bool QuadTreeNew::Node::operator!=(const WhirlyKit::QuadTreeNew::Node &that) const
{
    return level != that.level || x != that.x || y != that.y;
}

MbrD QuadTreeNew::generateMbrForNode(const Node &node)
{
    MbrD outMbr;
    Point2d chunkSize = mbr.span();
    chunkSize.x() /= (1<<node.level);
    chunkSize.y() /= (1<<node.level);
    
    outMbr.ll() = Point2d(chunkSize.x()*node.x,chunkSize.y()*node.y) + mbr.ll();
    outMbr.ur() = Point2d(chunkSize.x()*(node.x+1),chunkSize.y()*(node.y+1)) + mbr.ll();
    
    return outMbr;
}
    
bool QuadTreeNew::ImportantNode::operator<(const WhirlyKit::QuadTreeNew::ImportantNode &that) const
{
    if (importance == that.importance) {
        return Node::operator<(that);
    }
    
    return importance < that.importance;
}
    
bool QuadTreeNew::ImportantNode::operator==(const WhirlyKit::QuadTreeNew::ImportantNode &that) const
{
    if (importance != that.importance) {
        return Node::operator==(that);
    }
    
    return true;
}

QuadTreeNew::QuadTreeNew(const MbrD &mbr,int minLevel,int maxLevel)
    : mbr(mbr), minLevel(minLevel), maxLevel(maxLevel)
{
}

QuadTreeNew::~QuadTreeNew()
{
}

QuadTreeNew::ImportantNodeSet QuadTreeNew::calcCoverageImportance(double minImportance,double minImportanceTop,int maxNodes,bool siblingNodes)
{
    ImportantNodeSet sortedNodes;
    
    // Start at the lowest level and work our way to higher resolution
    int numX = 1<<minLevel, numY = 1<<minLevel;
    for (int iy=0;iy<numY;iy++)
        for (int ix=0;ix<numX;ix++)
        {
            ImportantNode node(ix,iy,minLevel);
            evalNodeImportance(node,minImportance,minImportanceTop,sortedNodes);
        }
    
    // Add the most important nodes first until we run out
    ImportantNodeSet retNodes;
    NodeSet testRetNodes;
    for (auto nodeIt = sortedNodes.rbegin();nodeIt != sortedNodes.rend();nodeIt++) {
        if (testRetNodes.find(*nodeIt) == testRetNodes.end())
        {
            retNodes.insert(*nodeIt);
            testRetNodes.insert(*nodeIt);
        }
        // Make sure all the siblings are in there for some modes
        if (siblingNodes) {
            ImportantNode ident = *nodeIt;
            if (ident.level > minLevel && ident.level < maxLevel) {
                Node parentIdent(ident.x/2,ident.y/2,ident.level-1);
                for (int iy=0;iy<2;iy++)
                    for (int ix=0;ix<2;ix++) {
                        ImportantNode childIdent(parentIdent.x*2+ix,parentIdent.y*2+iy,ident.level);
                        childIdent.importance = ident.importance;
                        if (testRetNodes.find(childIdent) == testRetNodes.end()) {
                            retNodes.insert(childIdent);
                            testRetNodes.insert(childIdent);
                        }
                    }
            }
        }
        if (retNodes.size() >= maxNodes || nodeIt->importance < minImportance)
            break;
    }
    
    return retNodes;
}
    
void QuadTreeNew::evalNodeImportance(ImportantNode node,double minImport,double minImportTop,ImportantNodeSet &importSet)
{
    node.importance = importance(node);
    
    if ((node.level == minLevel && node.importance < minImportTop) ||
        (node.level > minLevel && node.importance < minImport) ||
        node.level > maxLevel)
        return;
    
    importSet.insert(node);
    
    if (node.level < maxLevel) {
        // Add the children
        for (int iy=0;iy<2;iy++) {
            int indY = 2*node.y + iy;
            for (int ix=0;ix<2;ix++) {
                int indX = 2*node.x + ix;
                ImportantNode childNode(indX,indY,node.level+1);
                evalNodeImportance(childNode,minImport,minImportTop,importSet);
            }
        }
    }
}
    
void QuadTreeNew::evalNodeVisible(ImportantNode node,double maxLevel,ImportantNodeSet &visibleSet)
{
    if (node.level > maxLevel || !visible(node))
        return;
    // These are used for sorting elsewhere, so let's keep 'em around
    node.importance = importance(node);

    visibleSet.insert(node);
    
    // Test the children
    if (node.level < maxLevel) {
        for (int iy=0;iy<2;iy++) {
            int indY = 2*node.y + iy;
            for (int ix=0;ix<2;ix++) {
                int indX = 2*node.x + ix;
                ImportantNode childNode(indX,indY,node.level+1);
                evalNodeVisible(childNode,maxLevel,visibleSet);
            }
        }
    }
}
    
std::tuple<int,QuadTreeNew::ImportantNodeSet> QuadTreeNew::calcCoverageVisible(double minImportance,double minImportanceTop,int maxNodes,const std::vector<int> &levelLoads)
{
    ImportantNodeSet sortedNodes;

    // Start at the lowest level and work our way to higher resolution
    int numX = 1<<minLevel, numY = 1<<minLevel;
    for (int iy=0;iy<numY;iy++)
        for (int ix=0;ix<numX;ix++)
        {
            ImportantNode node(ix,iy,minLevel);
            evalNodeImportance(node,minImportance,minImportanceTop,sortedNodes);
        }

    // Max level is the one we want to load (or try anyway)
    int targetLevel = -1;
    for (auto node: sortedNodes)
        targetLevel = std::max(targetLevel,node.level);
    
    // Uh, wha?
    if (targetLevel < 0) {
        return {0,ImportantNodeSet()};
    }

    // Get visibility for all the nodes down to our target level
    ImportantNodeSet levelNodes;
    for (int iy=0;iy<numY;iy++)
        for (int ix=0;ix<numX;ix++)
        {
            ImportantNode node(ix,iy,minLevel);
            evalNodeVisible(node,targetLevel,levelNodes);
        }
    
    // Check how many visibile tiles we have at a given level
    // Drop back if it's too many
    int chosenLevel = targetLevel;
    ImportantNodeSet chosenNodes;
    while (chosenLevel >= minLevel) {
        // Resolve the offsets and such if they are there
        std::set<int> levelsToLoad;
        levelsToLoad.insert(minLevel);
        levelsToLoad.insert(chosenLevel);
        for (int level : levelLoads) {
            if (level < 0)
                level = targetLevel + level;
            if (level >= 0 && level < maxLevel)
                levelsToLoad.insert(level);
        }

        chosenNodes.clear();
        for (auto node: levelNodes) {
            if (levelsToLoad.find(node.level) != levelsToLoad.end())
                chosenNodes.insert(node);
        }
        if (chosenNodes.size() <= maxNodes)
            break;
        chosenLevel--;
    }
    
    return {chosenLevel,chosenNodes};
}
    
}
