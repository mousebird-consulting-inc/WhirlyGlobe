/*
 *  QuadTree.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/29/11.
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

#import "Quadtree.h"

namespace WhirlyKit
{
    
bool Quadtree::Identifier::operator<(const Identifier &that) const
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
    
bool Quadtree::NodeInfo::operator<(const NodeInfo &that) const
{
    if (importance == that.importance)
        return (ident < that.ident);
    return importance < that.importance;
}
    
Quadtree::Node::Node(Quadtree *tree)
{
    parent = NULL;
    for (unsigned int ii=0;ii<4;ii++)
        children[ii] = NULL;
    identPos = tree->nodesByIdent.end();
    sizePos = tree->nodesBySize.end();
}
    
void Quadtree::Node::addChild(Quadtree *tree,Node *child)
{
    // Note: The iterators don't appear to be safe
    NodesBySizeType::iterator it = tree->nodesBySize.find(this);
    if (it != tree->nodesBySize.end())
        tree->nodesBySize.erase(it);
    sizePos = tree->nodesBySize.end();

    int which = -1;
    for (unsigned int ii=0;ii<4;ii++)
    {
        if (children[ii] == child)
            return;
        if (!children[ii])
        {
            which = ii;
            break;
        }
    }
    if (which != -1)
        children[which] = child;
}
    
void Quadtree::Node::removeChild(Quadtree *tree, Node *child)
{
    bool hasChildren = false;
    for (unsigned int ii=0;ii<4;ii++)
    {
        if (children[ii] == child)
            children[ii] = NULL;
        hasChildren |= (children[ii] != NULL);
    }
    if (!hasChildren)
        sizePos = tree->nodesBySize.insert(this).first;
}
    
bool Quadtree::Node::hasChildren()
{
    for (unsigned int ii=0;ii<4;ii++)
        if (children[ii])
            return true;
    return false;
}
    
void Quadtree::Node::Print()
{
    // Note: Porting
//    NSLog(@"Node (%d,%d,%d)",nodeInfo.ident.x,nodeInfo.ident.y,nodeInfo.ident.level);
//    if (parent)
//        NSLog(@" Parent = (%d,%d,%d)",parent->nodeInfo.ident.x,parent->nodeInfo.ident.y,parent->nodeInfo.ident.level);
//    for (unsigned int ii=0;ii<4;ii++)
//        if (children[ii])
//            NSLog(@"  Child = (%d,%d,%d)",children[ii]->nodeInfo.ident.x,children[ii]->nodeInfo.ident.y,children[ii]->nodeInfo.ident.level);
}

Quadtree::Quadtree(Mbr mbr,int minLevel,int maxLevel,int maxNodes,float minImportance,QuadTreeImportanceCalculator *importDelegate)
    : mbr(mbr), minLevel(minLevel), maxLevel(maxLevel), maxNodes(maxNodes), minImportance(minImportance), numPhantomNodes(0)
{
    this->importDelegate = importDelegate;
}
    
Quadtree::~Quadtree()
{
    for (NodesByIdentType::iterator it = nodesByIdent.begin();
         it != nodesByIdent.end(); ++it)
        delete *it;
    nodesByIdent.clear();
    nodesBySize.clear();
}
    
bool Quadtree::isTileLoaded(Identifier ident)
{
    Node dummyNode(this);
    dummyNode.nodeInfo.ident = ident;
    
    NodesBySizeType::iterator it = nodesByIdent.find(&dummyNode);
    return it != nodesByIdent.end();
}
    
bool Quadtree::willAcceptTile(const NodeInfo &nodeInfo)
{
    // Reject it out of hand if it's too small
    if (nodeInfo.importance < minImportance)
        return false;
    
    // It must have a parent loaded in, if it's not at the top
    if (nodeInfo.ident.level > minLevel)
    {
        if (!getNode(Identifier(nodeInfo.ident.x / 2, nodeInfo.ident.y / 2, nodeInfo.ident.level - 1)))
            return false;
    }    
    
    // If we're not at the limit, then sure
    if (nodesByIdent.size()-numPhantomNodes < maxNodes)
        return true;
    
    // Otherwise, this one needs to be more important
    NodesBySizeType::iterator it = nodesBySize.begin();
    // Should never happen
    if (it == nodesBySize.end())
        return false;
    Node *compNode = *it;
    
    return compNode->nodeInfo.importance < nodeInfo.importance;
}
    
bool Quadtree::isPhantom(const Identifier &ident)
{
    Node dummyNode(this);
    dummyNode.nodeInfo.ident = ident;
    
    NodesByIdentType::iterator it = nodesByIdent.find(&dummyNode);
    if (it == nodesByIdent.end())
        return false;

    bool phantom = (*it)->nodeInfo.phantom;
    return phantom;
}
    
void Quadtree::setPhantom(const Identifier &ident,bool newPhantom)
{
    Node dummyNode(this);
    dummyNode.nodeInfo.ident = ident;
    
    NodesByIdentType::iterator it = nodesByIdent.find(&dummyNode);
    if (it != nodesByIdent.end())
    {
        bool wasPhantom = (*it)->nodeInfo.phantom;
        (*it)->nodeInfo.phantom = newPhantom;
        if (wasPhantom)
            numPhantomNodes--;
        if (newPhantom)
            numPhantomNodes++;
    }
}

    
void Quadtree::reevaluateNodes()
{
    nodesBySize.clear();
    
    for (NodesByIdentType::iterator it = nodesByIdent.begin();
         it != nodesByIdent.end(); ++it)
    {
        Node *node = *it;
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/Quadtree.cpp
        node->nodeInfo.importance = importDelegate->importanceForTile(node->nodeInfo.ident, node->nodeInfo.mbr, this, &node->nodeInfo.attrs);
=======
        node->nodeInfo.importance = [importDelegate importanceForTile:node->nodeInfo.ident mbr:node->nodeInfo.mbr tree:this attrs:node->nodeInfo.attrs];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/Quadtree.mm
        if (!node->hasChildren())
            nodesBySize.insert(node);
    }
}

void Quadtree::addTile(NodeInfo nodeInfo, std::vector<Identifier> &tilesRemoved)
{
    // Look for the parent
    Node *parent = NULL;
    if (nodeInfo.ident.level > minLevel)
    {
        parent = getNode(Identifier(nodeInfo.ident.x / 2, nodeInfo.ident.y / 2, nodeInfo.ident.level - 1));
        // Note: Should check for a missing parent.  Shouldn't happen.
    }

    // Set up the node first, so we don't remove the parent
    Node *node = new Node(this);
    node->parent = parent;
    node->nodeInfo = nodeInfo;
    if (parent)
        node->parent->addChild(this,node);

    // Need to remove a node
    int numNodes = (int)nodesByIdent.size();
    if (numNodes > maxNodes)
    {
        NodesBySizeType::iterator it = nodesBySize.begin();
        if (it != nodesBySize.end())
        {
            tilesRemoved.push_back((*it)->nodeInfo.ident);
            removeNode(*it);
        }
    }    

    // Add the new node into the lists here, so we don't remove it immediately
    node->identPos = nodesByIdent.insert(node).first;
    node->sizePos = nodesBySize.insert(node).first;

    if (nodeInfo.phantom)
        numPhantomNodes++;
}
    
void Quadtree::removeTile(Identifier ident)
{
    Node *node = getNode(ident);
    if (node)
    {
        if (node->nodeInfo.phantom)
            numPhantomNodes--;
        removeNode(node);
    }
}
    
Quadtree::NodeInfo Quadtree::generateNode(Identifier ident)
{
    NodeInfo nodeInfo;
    nodeInfo.ident = ident;
    nodeInfo.mbr = generateMbrForNode(ident);
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/Quadtree.cpp
    nodeInfo.importance = importDelegate->importanceForTile(nodeInfo.ident, nodeInfo.mbr, this, &nodeInfo.attrs);
=======
    nodeInfo.importance = [importDelegate importanceForTile:nodeInfo.ident mbr:nodeInfo.mbr tree:this attrs:nodeInfo.attrs];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/Quadtree.mm
    
    return nodeInfo;
}
    
Mbr Quadtree::generateMbrForNode(Identifier ident)
{
    Point2f chunkSize(mbr.ur()-mbr.ll());
    chunkSize.x() /= (1<<ident.level);
    chunkSize.y() /= (1<<ident.level);
    
    Mbr outMbr;
    outMbr.ll() = Point2f(chunkSize.x()*ident.x,chunkSize.y()*ident.y) + mbr.ll();
    outMbr.ur() = Point2f(chunkSize.x()*(ident.x+1),chunkSize.y()*(ident.y+1)) + mbr.ll();    
    
    return outMbr;
}
    
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/Quadtree.cpp
bool Quadtree::leastImportantNode(NodeInfo &nodeInfo,bool ignoreImportance,int targetLevel)
=======
bool Quadtree::leastImportantNode(NodeInfo &nodeInfo,bool ignoreImportance)
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/Quadtree.mm
{
    // Look for the first unimportant node without children
    // If the targetLevel is set, we'll avoid unloading things that have parents
    //  that are currently at the right level and phantom
    for (NodesBySizeType::iterator it = nodesBySize.begin();
         it != nodesBySize.end(); ++it)
    {
        Node *node = *it;
        if (ignoreImportance || (node->nodeInfo.importance < minImportance && node->nodeInfo.ident.level > minLevel))
        {
            unsigned int ii;
            for (ii=0;ii<4;ii++)
                if (node->children[ii])
                    break;
            if (ii == 4)
            {
                nodeInfo = node->nodeInfo;
                return true;
            }
        }
    }
    
    return false;
}
    
void Quadtree::unimportantNodes(std::vector<NodeInfo> &nodes,float importance)
{
    for (NodesBySizeType::iterator it = nodesBySize.begin();
         it != nodesBySize.end(); ++it)
    {
        Node *node = *it;
        if (node->nodeInfo.importance < importance && node->nodeInfo.ident.level > minLevel)
        {
            unsigned int ii;
            for (ii=0;ii<4;ii++)
                if (node->children[ii])
                    break;
            if (ii == 4)
                nodes.push_back(node->nodeInfo);
        }
    }
}
    
void Quadtree::generateChildren(Identifier ident, std::vector<NodeInfo> &nodes)
{
    int sx = ident.x * 2;
    int sy = ident.y * 2;
    int level = ident.level + 1;
    
    for (unsigned int ix=0;ix<2;ix++)
        for (unsigned int iy=0;iy<2;iy++)
            nodes.push_back(generateNode(Identifier(sx+ix,sy+iy,level)));
}
    
bool Quadtree::childrenForNode(Quadtree::Identifier ident,std::vector<Quadtree::Identifier> &childIdents)
{
    Node *node = getNode(ident);
    if (!node)
        return false;

    for (unsigned int ii=0;ii<4;ii++)
        if (node->children[ii])
            childIdents.push_back(node->children[ii]->nodeInfo.ident);

    return true;
}

bool Quadtree::hasParent(Quadtree::Identifier ident,Quadtree::Identifier &parentIdent)
{
    if (ident.level == minLevel)
        return false;
    
    Node *node = getNode(Identifier(ident.x/2,ident.y/2,ident.level-1));
    if (!node)
        return false;

    parentIdent = node->nodeInfo.ident;
    return true;
}
    
void Quadtree::Print()
{
    // Note: Porting
//    NSLog(@"***QuadTree Dump***");
//    for (NodesByIdentType::iterator it = nodesByIdent.begin();
//         it != nodesByIdent.end(); ++it)
//    {
//        Node *node = *it;
//        node->Print();
//    }
//    NSLog(@"******");
}

    
Quadtree::Node *Quadtree::getNode(Identifier ident)
{
    Node dummyNode(this);
    dummyNode.nodeInfo.ident = ident;
    NodesByIdentType::iterator it = nodesByIdent.find(&dummyNode);
    if (it == nodesByIdent.end())
        return NULL;
    return *it;
}
    
void Quadtree::removeNode(Node *node)
{
    // Note: Iterators don't seem to be safe
    NodesByIdentType::iterator iit = nodesByIdent.find(node);
    if (iit != nodesByIdent.end())
        nodesByIdent.erase(iit);
    NodesBySizeType::iterator sit = nodesBySize.find(node);
    if (sit != nodesBySize.end())
        nodesBySize.erase(sit);
    
    // Note: Shouldn't happen, but just in case
    for (unsigned int ii=0;ii<4;ii++)
        if (node->children[ii])
            node->children[ii]->parent = NULL;

    // Remove from the parent
    if (node->parent)
        node->parent->removeChild(this, node);
    
    delete node;
}
    
void Quadtree::setMaxNodes(int newMaxNodes)
{
    maxNodes = newMaxNodes;
}

void Quadtree::setMinImportance(float newMinImportance)
{
    minImportance = newMinImportance;
}

}
