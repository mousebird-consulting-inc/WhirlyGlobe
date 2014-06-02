/*
 *  Cullable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
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

#import "GlobeScene.h"
#import "GlobeMath.h"

namespace WhirlyKit
{
    
CullTree::CullTree(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,Mbr localMbr,int depth,int maxDrawPerNode)
    : coordAdapter(coordAdapter), depth(depth), numCullables(0), maxDrawPerNode(maxDrawPerNode)
{
    topCullable = new Cullable(coordAdapter,localMbr,depth);
}
    
CullTree::~CullTree()
{
    delete topCullable;
}

<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/Cullable.cpp
void CullTree::dumpStats()
{
    // Note: Porting
//    NSLog(@"CullTree: %d nodes",topCullable->countNodes());
=======
   void CullTree::dumpStats()
{
    NSLog(@"CullTree: %d nodes",topCullable->countNodes());
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/Cullable.mm
}
    
Cullable::Cullable(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,Mbr localMbr,int depth)
    : localMbr(localMbr)
{
    height = depth;
    
    for (unsigned int ii=0;ii<4;ii++)
        children[ii] = NULL;
    
    // Put together the extreme points
    Point3f pts[8];
    pts[0] = coordAdapter->localToDisplay(Point3f(localMbr.ll().x(),localMbr.ll().y(),0.0));
    pts[1] = coordAdapter->localToDisplay(Point3f(localMbr.ur().x(),localMbr.ll().y(),0.0));
    pts[2] = coordAdapter->localToDisplay(Point3f(localMbr.ur().x(),localMbr.ur().y(),0.0));
    pts[3] = coordAdapter->localToDisplay(Point3f(localMbr.ll().x(),localMbr.ur().y(),0.0));
    Point2f halfBot = (localMbr.ll() + Point2f(localMbr.ur().x(),localMbr.ll().y()))/2.0;
    pts[4] = coordAdapter->localToDisplay(Point3f(halfBot.x(),halfBot.y(),0.0));
    Point2f halfTop = (Point2f(localMbr.ll().x(),localMbr.ur().y()) + localMbr.ur())/2.0;
    pts[5] = coordAdapter->localToDisplay(Point3f(halfTop.x(),halfTop.y(),0.0));
    Point2f halfLeft = (localMbr.ll() + Point2f(localMbr.ll().x(),localMbr.ur().y()))/2.0;
    pts[6] = coordAdapter->localToDisplay(Point3f(halfLeft.x(),halfLeft.y(),0.0));
    Point2f halfRight = (Point2f(localMbr.ur().x(),localMbr.ll().y()) + localMbr.ur())/2.0;
    pts[7] = coordAdapter->localToDisplay(Point3f(halfRight.x(),halfRight.y(),0.0));
    
    // Now get the bounding box in 3-space
    Point3f minPt,maxPt;
    minPt = maxPt = pts[0];
    for (unsigned int ii=1;ii<8;ii++)
    {
        const Point3f &pt = pts[ii];
        minPt.x() = std::min(minPt.x(),pt.x());
        minPt.y() = std::min(minPt.y(),pt.y());
        minPt.z() = std::min(minPt.z(),pt.z());
        maxPt.x() = std::max(maxPt.x(),pt.x());
        maxPt.y() = std::max(maxPt.y(),pt.y());
        maxPt.z() = std::max(maxPt.z(),pt.z());
    }
//    Point3f midPt = (minPt+maxPt)/2.0;
//    minPt += 0.1 * (minPt - midPt);
//    maxPt += 0.1 * (maxPt - midPt);
    
    // Turn this into a cube for evaluation by the culling logic
    cornerPoints[0] = Point3f(minPt.x(),minPt.y(),minPt.z());
    cornerPoints[1] = Point3f(maxPt.x(),minPt.y(),minPt.z());
    cornerPoints[2] = Point3f(maxPt.x(),maxPt.y(),minPt.z());
    cornerPoints[3] = Point3f(minPt.x(),maxPt.y(),minPt.z());
    cornerPoints[4] = Point3f(minPt.x(),minPt.y(),maxPt.z());
    cornerPoints[5] = Point3f(maxPt.x(),minPt.y(),maxPt.z());
    cornerPoints[6] = Point3f(maxPt.x(),maxPt.y(),maxPt.z());
    cornerPoints[7] = Point3f(minPt.x(),maxPt.y(),maxPt.z());
    
    // Use just 4 of the normals
    for (unsigned int ii=0;ii<4;ii++)
        cornerNorms[ii] = pts[ii];
    
    // Set the child bounding boxes as well
    // We use these to check for overlap without creating a child
    Point2f mid = (localMbr.ur()+localMbr.ll())/2.0;
    childMbr[0] = Mbr(localMbr.ll(),mid);
    childMbr[1] = Mbr(Point2f(mid.x(),localMbr.ll().y()),Point2f(localMbr.ur().x(),mid.y()));
    childMbr[2] = Mbr(Point2f(localMbr.ll().x(),mid.y()),Point2f(mid.x(),localMbr.ur().y()));
    childMbr[3] = Mbr(mid,localMbr.ur());
}
    
Cullable::~Cullable()
{
    for (unsigned int ii=0;ii<4;ii++)
        if (children[ii])
            delete children[ii];
}
    
// Return an existing child if it's there or make a new one
Cullable *Cullable::getOrAddChild(int which, CullTree *cullTree)
{
    if (height == 0)
        return NULL;
    
    if (children[which])
        return children[which];
    
    children[which] = new Cullable(cullTree->coordAdapter,childMbr[which],height-1);
    cullTree->numCullables++;
    
    return children[which];
}
    
// Remove a child if it's empty
void Cullable::possibleRemoveChild(int which, CullTree *cullTree)
{
    if (children[which])
    {
        if (!children[which]->hasChildren() && children[which]->isEmpty())
        {
            delete children[which];
            children[which] = NULL;
            cullTree->numCullables--;
        }
    }
}
    
// Split the existing node into 4 and sort all the drawables
void Cullable::split(CullTree *cullTree)
{
    for (std::set<DrawableRef,IdentifiableRefSorter>::iterator it = drawables.begin();
         it != drawables.end(); ++it)
    {
        DrawableRef draw = *it;
        // Note: This doesn't take the geographic case into account
        Mbr localMbr = draw->getLocalMbr();
        
        addDrawableToChildren(cullTree,localMbr,draw);
    }
    
    drawables.clear();
}
    
void Cullable::addDrawableToChildren(CullTree *cullTree,Mbr drawLocalMbr,DrawableRef draw)
{
    for (unsigned int ii=0;ii<4;ii++)
    {
        if (childMbr[ii].overlaps(drawLocalMbr))
        {
            Cullable *child = getOrAddChild(ii, cullTree);
            child->addDrawable(cullTree,drawLocalMbr,draw);            
        }
    }
}
    
void Cullable::addDrawable(CullTree *cullTree,Mbr drawLocalMbr,DrawableRef draw)
{
    // Will be present in the children (or here)
    childDrawables.insert(draw);
    
    // If it's got a matrix, that can be changed and we have no clue where it might end up
    // Same for drawables without a valid local MBR
    if (draw->getMatrix() || !drawLocalMbr.valid())
    {
        drawables.insert(draw);
        return;
    }

    // Might need to split it
    if (drawables.size() > cullTree->maxDrawPerNode && height > 0)
    {
        drawables.insert(draw);
        split(cullTree);
    }
    
    // The drawable is bigger than this node, so just stick it here
    if (localMbr.contained(drawLocalMbr))
        drawables.insert(draw);
    else {
        // If there are children, sort it into those, maybe
        if (hasChildren())
            addDrawableToChildren(cullTree, drawLocalMbr, draw);
        else
            // This is where it nominally lives
            drawables.insert(draw);
    }
}
    
void Cullable::remDrawable(CullTree *cullTree,Mbr drawLocalMbr,DrawableRef draw)
{
    // Remove it from here
    childDrawables.erase(draw);
    drawables.erase(draw);
    
    // Remove from the appropriate children
    if (height > 0)
    {
        for (unsigned int ii=0;ii<4;ii++)
        {
            if (children[ii] && childMbr[ii].overlaps(drawLocalMbr))
            {
                children[ii]->remDrawable(cullTree,drawLocalMbr,draw);            
                possibleRemoveChild(ii, cullTree);
            }
        }
    }
    
    // Prune the children if they fall below a certain threshold
    if (childDrawables.size() < cullTree->maxDrawPerNode)
    {
        drawables = childDrawables;
        for (unsigned int ii=0;ii<4;ii++)
            if (children[ii])
            {
                delete children[ii];
                children[ii] = NULL;
            }
    }
}

int Cullable::countNodes() const
{
    int count = 1;
    for (unsigned int ii=0;ii<4;ii++)
        if (children[ii])
            count += children[ii]->countNodes();
    
    return count;
}

}
