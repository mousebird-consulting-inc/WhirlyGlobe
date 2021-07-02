/*  QuadTreeNew.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford
 *  Copyright 2012-2021 mousebird consulting
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

#import "QuadTreeNew.h"

static constexpr int maxMaxLevel = 24;

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

MbrD QuadTreeNew::generateMbrForNode(const Node &node) const
{
    Point2d chunkSize = mbr.span();
    chunkSize.x() /= (1<<node.level);
    chunkSize.y() /= (1<<node.level);

    return {Point2d(chunkSize.x()*node.x,chunkSize.y()*node.y) + mbr.ll(),
            Point2d(chunkSize.x()*(node.x+1),chunkSize.y()*(node.y+1)) + mbr.ll()};
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
    : mbr(mbr), minLevel(minLevel), maxLevel(std::min(maxLevel, maxMaxLevel))
{
}

QuadTreeNew::ImportantNodeSet QuadTreeNew::calcCoverageImportance(const std::vector<double> &minImportance,int maxNodes,bool siblingNodes,std::vector<double> &maxRejectedImport)
{
    ImportantNodeSet sortedNodes;
    
    // Start at the lowest level and work our way to higher resolution
    const int numX = 1<<minLevel;
    const int numY = 1<<minLevel;
    for (int iy=0;iy<numY;iy++)
        for (int ix=0;ix<numX;ix++)
        {
            ImportantNode node(ix,iy,minLevel);
            evalNodeImportance(node,minImportance,sortedNodes,maxRejectedImport);
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
        if (retNodes.size() >= maxNodes || nodeIt->importance < minImportance[nodeIt->level])
            break;
    }
    
    return retNodes;
}
    
void QuadTreeNew::evalNodeImportance(ImportantNode node,const std::vector<double> &minImportance,ImportantNodeSet &importSet,std::vector<double> &maxRejectedImport)
{
    node.importance = importance(node);
    
    if (node.level > maxLevel)
        return;
    
    if (node.importance < minImportance[node.level] && minImportance[node.level] != MAXFLOAT) {
        double ratio = 1.0;
        double thisMinImport = minImportance[node.level];
        if (thisMinImport > 0.0 && thisMinImport != MAXFLOAT)
            ratio = node.importance / minImportance[node.level];
        
        maxRejectedImport[node.level] = std::max(ratio,maxRejectedImport[node.level]);
        return;
    }

    if (node.level >= minLevel)
        importSet.insert(node);
    
    if (node.level < maxLevel) {
        // Add the children
        for (int iy=0;iy<2;iy++) {
            int indY = 2*node.y + iy;
            for (int ix=0;ix<2;ix++) {
                int indX = 2*node.x + ix;
                ImportantNode childNode(indX,indY,node.level+1);
                evalNodeImportance(childNode,minImportance,importSet,maxRejectedImport);
            }
        }
    }
}
    
bool QuadTreeNew::evalNodeVisible(ImportantNode node, const std::vector<double> &minImportance, int maxNodes,
                                  const std::set<int> &levelsToLoad, int inMaxLevel, ImportantNodeSet &visibleSet)
{
    if (node.level > inMaxLevel)
        return true;

    // These are used for sorting elsewhere, so let's keep 'em around
    node.importance = importance(node);

    if (node.level == minLevel && node.importance < minImportance[node.level])
        return true;

    // Skip anything we wouldn't have evaluated in the first pass
    if (node.level != minLevel && node.level <= inMaxLevel && node.importance == 0.0)
        return true;

    // Only add to the visible set if we want it
    if (levelsToLoad.find(node.level) != levelsToLoad.end())
        visibleSet.insert(node);

    // Exceeded the number of nodes we can plausibly load.  Fail.
    if (visibleSet.size() > maxNodes)
        return false;
    
    // Test the children
    if (node.level < inMaxLevel) {
        for (int iy=0;iy<2;iy++) {
            int indY = 2*node.y + iy;
            for (int ix=0;ix<2;ix++) {
                int indX = 2*node.x + ix;
                ImportantNode childNode(indX,indY,node.level+1);
                if (!evalNodeVisible(childNode, minImportance, maxNodes, levelsToLoad, inMaxLevel, visibleSet))
                    return false;
            }
        }
    }
    
    return true;
}
    
std::tuple<int,QuadTreeNew::ImportantNodeSet> QuadTreeNew::calcCoverageVisible(
        const std::vector<double> &minImportance,
        int maxNodes,const std::vector<int> &levelLoads,
        bool keepMinLevel,std::vector<double> &maxRejectedImport)
{
    ImportantNodeSet sortedNodes;

    // Start at the lowest level and work our way to higher resolution
    const ImportantNode node(0,0,0);
    evalNodeImportance(node,minImportance,sortedNodes,maxRejectedImport);

    // Max level is the one we want to load (or try anyway)
    int targetLevel = -1;
    for (const auto &inode: sortedNodes)
        targetLevel = std::max(targetLevel,inode.level);
 
    targetLevel = std::max(targetLevel,minLevel);
    
    // Try to load the target level (and anything else we're required to)
    int chosenLevel = targetLevel;
    ImportantNodeSet chosenNodes;
    while (chosenLevel >= minLevel) {
        // Resolve the offsets and such if they are there
        std::set<int> levelsToLoad;
        if (keepMinLevel)
            levelsToLoad.insert(minLevel);
        levelsToLoad.insert(chosenLevel);
        for (int level : levelLoads) {
            if (level < 0)
                level = targetLevel + level;
            if (level >= 0 && level < maxLevel)
                levelsToLoad.insert(level);
        }

        // Get visibility for all the nodes down to our target level
        // Also make sure we're not exceeding our maximum as we go
        // Kept within the limit, so return these nodes
        ImportantNodeSet levelNodes;
        if (evalNodeVisible(node,minImportance,maxNodes,levelsToLoad,chosenLevel,levelNodes)) {
            chosenNodes = levelNodes;
            break;
        }
        chosenLevel--;
    }

    return {chosenLevel,chosenNodes};
}
    
}
