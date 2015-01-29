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
    
bool Quadtree::Identifier::operator==(const Identifier &that) const
{
    return level == that.level && x == that.x && y == that.y;
}

bool Quadtree::NodeInfo::operator<(const NodeInfo &that) const
{
    if (importance == that.importance)
        return (ident < that.ident);
    return importance < that.importance;
}
    
bool Quadtree::NodeInfo::isFrameLoaded(int theFrame) const
{
    if (theFrame == -1)
        return frameFlags != 0;
    else
        return frameFlags & (1<<theFrame);
}
    
bool Quadtree::NodeInfo::isFrameLoading(int theFrame) const
{
    if (theFrame == -1)
        return frameLoadingFlags != 0;
    else
        return frameLoadingFlags & (1<<theFrame);
}
    
void Quadtree::NodeInfo::setFrameLoading(int theFrame,bool val)
{
    if (theFrame == -1)
        frameLoadingFlags = val;
    else {
        if (val)
            frameLoadingFlags |= (1<<theFrame);
        else
            frameLoadingFlags &= ~(1<<theFrame);
    }
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
    
bool Quadtree::Node::hasNonPhantomParent()
{
    Node *p = parent;
    while (p)
    {
        if (!p->nodeInfo.phantom)
            return true;
        p = p->parent;
    }
    
    return false;
}
    
bool Quadtree::Node::parentLoading()
{
    Node *p = parent;
    while (p)
    {
        if (p->nodeInfo.isFrameLoading(-1))
            return true;
        p = p->parent;
    }
    
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

Quadtree::Quadtree(Mbr mbr,int minLevel,int maxLevel,int maxNodes,float minImportance,NSObject<WhirlyKitQuadTreeImportanceDelegate> *importDelegate)
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
    
bool Quadtree::isTilePresent(const Identifier &ident)
{
    Node dummyNode(this);
    dummyNode.nodeInfo.ident = ident;
    
    NodesBySizeType::iterator it = nodesByIdent.find(&dummyNode);
    return it != nodesByIdent.end();
}
    
bool Quadtree::isFull()
{
    return (nodesByIdent.size()-numPhantomNodes >= maxNodes);
}
    
bool Quadtree::shouldLoadTile(const Identifier &ident,int frame)
{
    Node *node = getNode(ident);
    if (!node)
        return false;
    
    // It's already loading.
    if (node->nodeInfo.isFrameLoading(frame))
        return false;
    
    // Reject it out of hand if it's too small
    if (node->nodeInfo.importance < minImportance)
        return false;
    
    // It must have a parent loaded in, if it's not at the top
    if (node->nodeInfo.ident.level > minLevel)
    {
        if (!getNode(Identifier(node->nodeInfo.ident.x / 2, node->nodeInfo.ident.y / 2, node->nodeInfo.ident.level - 1)))
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
    
    return compNode->nodeInfo.importance < node->nodeInfo.importance;
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
    
    
bool Quadtree::Node::recalcCoverage()
{
    bool newChildCoverage = true;
    for (unsigned int ii=0;ii<4;ii++)
    {
        Node *child = children[ii];
        if (child)
            newChildCoverage &= child->nodeInfo.childCoverage || (!child->nodeInfo.phantom && !child->nodeInfo.isFrameLoading(-1));
        else
            newChildCoverage = false;
    }
    nodeInfo.childCoverage = newChildCoverage;

    return newChildCoverage;
}
    
int Quadtree::getFrameCount(int frame)
{
    if (frame >= frameLoadCounts.size())
        return 0;
    return frameLoadCounts[frame];
}
    
void Quadtree::addFrameLoaded(int frame)
{
    for (int ii=frameLoadCounts.size();ii<=frame;ii++)
        frameLoadCounts.push_back(0);
    frameLoadCounts[frame]++;
}

void Quadtree::clearFlagCounts(int frameFlags)
{
    for (unsigned int ii=0;ii<frameLoadCounts.size();ii++)
        if (frameFlags & (1<<ii))
            frameLoadCounts[ii]--;
}
    
bool Quadtree::frameIsLoaded(int frame,int *tilesLoaded)
{
    int count = getFrameCount(frame);
    if (tilesLoaded)
        *tilesLoaded = count;
    
    // Make sure we don't agree when there's nothing going on
    if (count == 0)
        return false;
    
    bool isLoaded = (count+numPhantomNodes) == nodesByIdent.size();
    
//    if (count+numPhantomNodes > nodesByIdent.size())
//        NSLog(@"Got one");
    
    return isLoaded;
}
    
void Quadtree::updateParentCoverage(const Identifier &ident,std::vector<Identifier> &coveredTiles,std::vector<Identifier> &unCoveredTiles)
{
    Node *node = getNode(ident);

    if (node)
    {
        Node *p = node->parent;
        while (p)
        {
            bool pOldChildCoverage = p->nodeInfo.childCoverage;
            bool pNewChildCoverage = p->recalcCoverage();
            if (pOldChildCoverage == pNewChildCoverage)
            {
                break;
            } else {
                if (pNewChildCoverage)
                    coveredTiles.push_back(p->nodeInfo.ident);
                else
                    unCoveredTiles.push_back(p->nodeInfo.ident);
            }
            
            p = p->parent;
        }
    }
}
    
void Quadtree::recalcCoverage(Node *node)
{
    bool oldChildCoverage = node->nodeInfo.childCoverage;
    bool newChildCoverage = node->recalcCoverage();
    
    // Need to propagate the changes upward
    if (oldChildCoverage != newChildCoverage)
    {
        Node *p = node->parent;
        while (p)
        {
            bool pOldChildCoverage = p->nodeInfo.childCoverage;
            bool pNewChildCoverage = p->recalcCoverage();
            if (pOldChildCoverage == pNewChildCoverage)
                break;
            
            p = p->parent;
        }
    }
}
    
void Quadtree::setPhantom(const Identifier &ident,bool newPhantom)
{
    Node dummyNode(this);
    dummyNode.nodeInfo.ident = ident;
    
    Node *node = NULL;
    NodesByIdentType::iterator it = nodesByIdent.find(&dummyNode);
    if (it != nodesByIdent.end())
    {
        node = *it;
        bool wasPhantom = node->nodeInfo.phantom;
        node->nodeInfo.phantom = newPhantom;
        if (wasPhantom)
            numPhantomNodes--;
        if (newPhantom)
            numPhantomNodes++;
        
        recalcCoverage(node);
    } else
        // Haven't heard of it
        return;

    NodesBySizeType::iterator sit = nodesBySize.find(&dummyNode);
    // Clean it out of the nodes by size if it's a phantom
    if (newPhantom)
    {
        clearFlagCounts(node->nodeInfo.frameFlags);
        node->nodeInfo.frameFlags = 0;
        if (sit != nodesBySize.end())
            nodesBySize.erase(sit);
    } else {
        // Add it in if it's no longer a phantom
        if (sit == nodesBySize.end())
            nodesBySize.insert(*it);
    }
}

bool Quadtree::isLoading(const Identifier &ident,int frame)
{
    Node dummyNode(this);
    dummyNode.nodeInfo.ident = ident;
    
    NodesByIdentType::iterator it = nodesByIdent.find(&dummyNode);
    if (it == nodesByIdent.end())
        return false;
    
    bool loading = (*it)->nodeInfo.isFrameLoading(frame);
    return loading;
}

void Quadtree::setLoading(const Identifier &ident,int frame,bool newLoading)
{
    Node dummyNode(this);
    dummyNode.nodeInfo.ident = ident;
    
    NodesByIdentType::iterator it = nodesByIdent.find(&dummyNode);
    if (it != nodesByIdent.end())
    {
        bool wasLoading = (*it)->nodeInfo.isFrameLoading(frame);
        (*it)->nodeInfo.setFrameLoading(frame,newLoading);
        
        // Let the parents know
        if (wasLoading && !newLoading)
        {
            Node *parent = (*it)->parent;
            while (parent)
            {
                parent->nodeInfo.childrenLoading--;
                parent = parent->parent;
            }
        } else if (!wasLoading && newLoading)
        {
            Node *parent = (*it)->parent;
            while (parent)
            {
                parent->nodeInfo.childrenLoading++;
                parent = parent->parent;
            }
        }
    } else
        // Haven't heard of it
        return;
}
    
void Quadtree::didLoad(const Identifier &tileIdent,int frame)
{
    Node *node = getNode(tileIdent);
    if (!node)
        return;
    
    // Note: Could make these more efficient
    setLoading(tileIdent, frame, false);
    setFailed(tileIdent, false);
    
    if (frame != -1)
    {
        if (!(node->nodeInfo.frameFlags & 1<<frame))
        {
            node->nodeInfo.frameFlags |= 1<<frame;
            addFrameLoaded(frame);
        }
    }
}
    
bool Quadtree::isEvaluating(const Identifier &ident)
{
    Node dummyNode(this);
    dummyNode.nodeInfo.ident = ident;
    
    NodesByIdentType::iterator it = nodesByIdent.find(&dummyNode);
    if (it == nodesByIdent.end())
        return false;
    
    bool eval = (*it)->nodeInfo.eval;
    return eval;
}

void Quadtree::setEvaluating(const Identifier &ident,bool newEval)
{
    Node dummyNode(this);
    dummyNode.nodeInfo.ident = ident;
    
    NodesByIdentType::iterator it = nodesByIdent.find(&dummyNode);
    if (it != nodesByIdent.end())
    {
        bool wasEval = (*it)->nodeInfo.eval;
        Node *node = *it;
        node->nodeInfo.eval = newEval;
        
        // Let the parents know
        if (wasEval && !newEval)
        {
            Node *parent = node->parent;
            while (parent)
            {
                parent->nodeInfo.childrenEval--;
                parent = parent->parent;
            }
            
            evalNodes.erase(node->evalPos);
            node->evalPos = evalNodes.end();
        } else if (!wasEval && newEval)
        {
            Node *parent = node->parent;
            while (parent)
            {
                parent->nodeInfo.childrenEval++;
                parent = parent->parent;
            }
            
            node->evalPos = evalNodes.insert(node).first;
        }
    } else
        // Haven't heard of it
        return;
}
    
bool Quadtree::didFail(const Quadtree::Identifier &ident)
{
    Node *node = getNode(ident);
    
    if (node)
        return node->nodeInfo.failed;
    
    return false;
}
    
void Quadtree::setFailed(const Identifier &ident,bool newFail)
{
    Node dummyNode(this);
    dummyNode.nodeInfo.ident = ident;
    
    NodesByIdentType::iterator it = nodesByIdent.find(&dummyNode);
    if (it != nodesByIdent.end())
    {
        (*it)->nodeInfo.failed = newFail;
    }
}

bool Quadtree::childFailed(const Identifier &ident)
{
    for (unsigned int ix=0;ix<2;ix++)
        for (unsigned int iy=0;iy<2;iy++)
        {
            Node *child = getNode(Identifier(ident.x*2+ix,ident.y*2+iy,ident.level+1));
            if (child && child->nodeInfo.failed)
                return true;
        }
    
    return false;
}
    
int Quadtree::numEvals()
{
    return evalNodes.size();
}
    
void Quadtree::clearEvals()
{
    for (NodesByIdentType::iterator it = nodesByIdent.begin();it != nodesByIdent.end(); ++it)
    {
        Node *node = *it;
        node->evalPos = evalNodes.end();
        node->nodeInfo.eval = false;
        node->nodeInfo.childrenEval = 0;
//        node->nodeInfo.loading = false;
        node->nodeInfo.childrenLoading = 0;
        node->nodeInfo.childrenEval = 0;
        node->nodeInfo.failed = false;
    }
    
    evalNodes.clear();
}
    
void Quadtree::clearFails()
{
    for (NodesByIdentType::iterator it = nodesByIdent.begin();it != nodesByIdent.end(); ++it)
    {
        Node *node = *it;
        node->nodeInfo.failed = false;
    }
    }
    
bool Quadtree::popLastEval(NodeInfo &retNodeInfo)
{
    if (evalNodes.empty())
        return false;
    NodesBySizeType::iterator it = evalNodes.end();
    it--;
    Node *node = *it;
    evalNodes.erase(it);
    node->nodeInfo.eval = false;
    
    // Remove children eval
    Node *parent = node->parent;
    while (parent)
    {
        parent->nodeInfo.childrenEval--;
        parent = parent->parent;
    }
    
    retNodeInfo = node->nodeInfo;
    return true;
}
    
bool Quadtree::childrenLoading(const Identifier &ident)
{
    Node dummyNode(this);
    dummyNode.nodeInfo.ident = ident;
    
    NodesByIdentType::iterator it = nodesByIdent.find(&dummyNode);
    if (it != nodesByIdent.end())
    {
        return (*it)->nodeInfo.childrenLoading;
    } else
        return false;
}

bool Quadtree::childrenEvaluating(const Identifier &ident)
{
    Node dummyNode(this);
    dummyNode.nodeInfo.ident = ident;
    
    NodesByIdentType::iterator it = nodesByIdent.find(&dummyNode);
    if (it != nodesByIdent.end())
    {
        int numEval = (*it)->nodeInfo.childrenEval;
        return numEval;
    } else
        return false;
}
    
void Quadtree::reevaluateNodes()
{
    nodesBySize.clear();
    evalNodes.clear();
    
    for (NodesByIdentType::iterator it = nodesByIdent.begin();
         it != nodesByIdent.end(); ++it)
    {
        Node *node = *it;
        node->nodeInfo.importance = [importDelegate importanceForTile:node->nodeInfo.ident mbr:node->nodeInfo.mbr tree:this attrs:node->nodeInfo.attrs];
        if (!node->hasChildren())
            node->sizePos = nodesBySize.insert(node).first;
        node->evalPos = evalNodes.insert(node).first;
    }
}

const Quadtree::NodeInfo *Quadtree::addTile(const Identifier &ident,bool newEval,bool checkImportance)
{
    bool oldEval = false;
    bool oldLoading = false;
    Node *node = getNode(ident);

    // Make up a new node
    if (!node)
{
    // Look for the parent
    Node *parent = NULL;
        if (ident.level > minLevel)
    {
            parent = getNode(Identifier(ident.x / 2, ident.y / 2, ident.level - 1));
        // Note: Should check for a missing parent.  Shouldn't happen.
    }

        // Check that the importance is more than our minimum before adding the tile
        NodeInfo nodeInfo = generateNode(ident);
        if (checkImportance && nodeInfo.importance < minImportance)
            return NULL;
        
    // Set up the node first, so we don't remove the parent
        node = new Node(this);
    node->nodeInfo = nodeInfo;
    node->parent = parent;
        node->nodeInfo.phantom = true;
        node->nodeInfo.frameLoadingFlags = 0;
        node->nodeInfo.eval = newEval;
    if (parent)
        node->parent->addChild(this,node);
        if (node->nodeInfo.phantom)
            numPhantomNodes++;
    } else {
        oldEval = node->nodeInfo.eval;
        oldLoading = node->nodeInfo.isFrameLoading(-1);
        node->nodeInfo.eval = newEval;
    }

    // Add the new node into the lists here, so we don't remove it immediately
    node->identPos = nodesByIdent.insert(node).first;
    if (!node->nodeInfo.phantom)
        node->sizePos = nodesBySize.insert(node).first;

    // Let the parents know
    if (!oldEval && newEval)
    {
        Node *parent = node->parent;
        while (parent)
    {
            parent->nodeInfo.childrenEval++;
            parent = parent->parent;
        }
        node->evalPos = evalNodes.insert(node).first;
    } else if (oldEval && !newEval)
    {
        Node *parent = node->parent;
        while (parent)
        {
            parent->nodeInfo.childrenEval--;
            parent = parent->parent;
        }
        NodesBySizeType::iterator it = evalNodes.find(node);
        if (it != evalNodes.end())
            evalNodes.erase(it);
    }    

    if (!oldLoading && node->nodeInfo.isFrameLoading(-1))
    {
        Node *parent = node->parent;
        while (parent)
        {
            parent->nodeInfo.childrenLoading++;
            parent = parent->parent;
        }
    } else if (oldLoading && !node->nodeInfo.isFrameLoading(-1))
    {
        Node *parent = node->parent;
        while (parent)
        {
            parent->nodeInfo.childrenLoading--;
            parent = parent->parent;
        }
    }

    return &node->nodeInfo;
}
    
void Quadtree::removeTile(const Identifier &ident)
{
    Node *node = getNode(ident);
    
    if (!node)
        return;
    
    // Can't remove nodes that have children
    if (node->hasChildren())
        return;
    
    if (node)
        removeNode(node);
    }
    
const Quadtree::NodeInfo *Quadtree::getNodeInfo(const Identifier &ident)
{
    Node *node = getNode(ident);
    if (!node)
        return NULL;
    
    return &node->nodeInfo;
}
    
Quadtree::NodeInfo Quadtree::generateNode(const Identifier &ident)
{
    NodeInfo nodeInfo;
    nodeInfo.ident = ident;
    nodeInfo.mbr = generateMbrForNode(ident);
    nodeInfo.importance = [importDelegate importanceForTile:nodeInfo.ident mbr:nodeInfo.mbr tree:this attrs:nodeInfo.attrs];
    
    return nodeInfo;
}
    
Mbr Quadtree::generateMbrForNode(const Identifier &ident)
{
    Point2f chunkSize(mbr.ur()-mbr.ll());
    chunkSize.x() /= (1<<ident.level);
    chunkSize.y() /= (1<<ident.level);
    
    Mbr outMbr;
    outMbr.ll() = Point2f(chunkSize.x()*ident.x,chunkSize.y()*ident.y) + mbr.ll();
    outMbr.ur() = Point2f(chunkSize.x()*(ident.x+1),chunkSize.y()*(ident.y+1)) + mbr.ll();    
    
    return outMbr;
}
    
void Quadtree::generateMbrForNode(const Identifier &ident,Point2d &ll,Point2d &ur)
{
    Point2d chunkSize(mbr.ur().x()-mbr.ll().x(),mbr.ur().y()-mbr.ll().y());
    chunkSize.x() /= (1<<ident.level);
    chunkSize.y() /= (1<<ident.level);
    
    ll = Point2d(chunkSize.x()*ident.x,chunkSize.y()*ident.y) + Point2d(mbr.ll().x(),mbr.ll().y());
    ur = Point2d(chunkSize.x()*(ident.x+1),chunkSize.y()*(ident.y+1)) + Point2d(mbr.ll().x(),mbr.ll().y());
}
    
bool Quadtree::leastImportantNode(NodeInfo &nodeInfo,bool force)
{
    // Look for the most unimportant node that isn't therwise engaged
    for (NodesBySizeType::iterator it = nodesBySize.begin();
         it != nodesBySize.end(); ++it)
    {
        Node *node = *it;
        if (force || node->nodeInfo.importance == 0.0 || ((node->nodeInfo.importance < minImportance && node->nodeInfo.ident.level > minLevel) &&
                                 !node->parentLoading() && node->nodeInfo.childrenLoading == 0 && node->hasNonPhantomParent()))
        {
            unsigned int ii;
            for (ii=0;ii<4;ii++)
                if (node->children[ii])
                    break;
            if (ii == 4 && node->nodeInfo.childrenLoading == 0 && node->nodeInfo.childrenEval == 0 && !node->parentLoading())
            {
                nodeInfo = node->nodeInfo;
                return true;
            }
        }
    }
    
    return false;
}
    
void Quadtree::childrenForNode(const Quadtree::Identifier &ident,std::vector<Quadtree::Identifier> &childIdents)
        {
    for (unsigned int ix=0;ix<2;ix++)
        for (unsigned int iy=0;iy<2;iy++)
            childIdents.push_back(Identifier(2*ident.x+ix,2*ident.y+iy,ident.level+1));
}
    
bool Quadtree::parentIsLoading(const Identifier &ident)
{
    Node *node = getNode(ident);
    if (!node)
        return false;
    Node *p = node->parent;
    while (p)
    {
        if (p->nodeInfo.isFrameLoading(-1))
    return true;
        p = node->parent;
}

    return false;
}

bool Quadtree::hasParent(const Quadtree::Identifier &ident,Quadtree::Identifier &parentIdent)
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
    NSLog(@"***QuadTree Dump***");
    for (NodesByIdentType::iterator it = nodesByIdent.begin();
         it != nodesByIdent.end(); ++it)
    {
        Node *node = *it;
        node->Print();
    }
    NSLog(@"******");
}

Quadtree::Node *Quadtree::getNode(const Identifier &ident)
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
    clearFlagCounts(node->nodeInfo.frameFlags);

    if (node->nodeInfo.phantom)
        numPhantomNodes--;

    // Let the parents know
    if (node->nodeInfo.isFrameLoading(-1))
    {
        Node *parent = node->parent;
        while (parent)
        {
            parent->nodeInfo.childrenLoading--;
            parent = parent->parent;
        }
    }
    if (node->nodeInfo.eval)
    {
        Node *parent = node->parent;
        while (parent)
        {
            parent->nodeInfo.childrenEval--;
            parent = parent->parent;
        }
    }
    
    // Note: Iterators don't seem to be safe
    NodesByIdentType::iterator iit = nodesByIdent.find(node);
    if (iit != nodesByIdent.end())
        nodesByIdent.erase(iit);
    NodesBySizeType::iterator sit = nodesBySize.find(node);
    if (sit != nodesBySize.end())
        nodesBySize.erase(sit);
    NodesBySizeType::iterator eit = evalNodes.find(node);
    if (eit != evalNodes.end())
        evalNodes.erase(eit);
    
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
