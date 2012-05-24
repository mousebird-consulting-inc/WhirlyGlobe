/*
 *  Cullable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
 *  Copyright 2011 mousebird consulting
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
    
CullTree::CullTree(WhirlyKit::CoordSystem *coordSystem,Mbr localMbr,int depth,int maxDrawPerNode)
    : coordSystem(coordSystem), depth(depth), numCullables(0), maxDrawPerNode(maxDrawPerNode)
{
    topCullable = new Cullable(coordSystem,localMbr,depth);
}
    
CullTree::~CullTree()
{
    delete topCullable;
}
    
Cullable::Cullable(WhirlyKit::CoordSystem *coordSystem,Mbr localMbr,int depth)
    : localMbr(localMbr)
{
    height = depth;
    
    for (unsigned int ii=0;ii<4;ii++)
        children[ii] = NULL;
    
	// Turn the corner points into real world values
    cornerPoints[0] = coordSystem->localToGeocentricish(Point3f(localMbr.ll().x(),localMbr.ll().y(),0.0));
    cornerPoints[1] = coordSystem->localToGeocentricish(Point3f(localMbr.ur().x(),localMbr.ll().y(),0.0));
    cornerPoints[2] = coordSystem->localToGeocentricish(Point3f(localMbr.ur().x(),localMbr.ur().y(),0.0));
    cornerPoints[3] = coordSystem->localToGeocentricish(Point3f(localMbr.ll().x(),localMbr.ur().y(),0.0));

	// Normals happen to be the same
    // Note: This only works for the globe
	for (unsigned int ii=0;ii<4;ii++)
		cornerNorms[ii] = cornerPoints[ii];
    
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
    
    children[which] = new Cullable(cullTree->coordSystem,childMbr[which],height-1);
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
    for (std::set<Drawable *,IdentifiableSorter>::iterator it = drawables.begin();
         it != drawables.end(); ++it)
    {
        Drawable *draw = *it;
        // Note: This doesn't take the geographic case into account
        Mbr localMbr = draw->getLocalMbr();
        
        addDrawableToChildren(cullTree,localMbr,draw);
    }
    
    drawables.clear();
}
    
void Cullable::addDrawableToChildren(CullTree *cullTree,Mbr drawLocalMbr,Drawable *draw)
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
    
void Cullable::addDrawable(CullTree *cullTree,Mbr drawLocalMbr,Drawable *draw)
{
    // Will be present in the children (or here)
    childDrawables.insert(draw);

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
    
void Cullable::remDrawable(CullTree *cullTree,Mbr drawLocalMbr,Drawable *draw)
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

}
