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

QuadTreeNew::ImportantNodeSet QuadTreeNew::calcCoverage(double minImportance,int maxNodes,bool siblingNodes)
{
    ImportantNodeSet sortedNodes;
    
    // Start at the lowest level and work our way to higher resolution
    int numX = 1<<minLevel, numY = 1<<minLevel;
    for (int iy=0;iy<numY;iy++)
        for (int ix=0;ix<numX;ix++)
        {
            ImportantNode node(ix,iy,minLevel);
            evalNode(node,minImportance,sortedNodes);
        }
    
    // Add the most important nodes first until we run out
    ImportantNodeSet retNodes;
    NodeSet testRetNodes;
    for (auto nodeIt = sortedNodes.rbegin();nodeIt != sortedNodes.rend();nodeIt++) {
        retNodes.insert(*nodeIt);
        testRetNodes.insert(*nodeIt);
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
    
void QuadTreeNew::evalNode(ImportantNode node,double minImport,ImportantNodeSet &importSet)
{
    node.importance = importance(node);
    
    if (node.importance < minImport || node.level >= maxLevel)
        return;

    importSet.insert(node);

    // Add the children
    for (int iy=0;iy<2;iy++) {
        int indY = 2*node.y + iy;
        for (int ix=0;ix<2;ix++) {
            int indX = 2*node.x + ix;
            ImportantNode childNode(indX,indY,node.level+1);
            evalNode(childNode,minImport,importSet);
        }
    }
}

//QuadTreeNew::NodeSet QuadTreeNew::calcCoverageToLevel(int loadLevel,int maxNodes)
//{
//}
    
}
