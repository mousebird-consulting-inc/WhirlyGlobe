/*
 *  OverlapHelper.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/28/15.
 *  Copyright 2011-2015 mousebird consulting. All rights reserved.
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

#import "OverlapHelper.h"
#import "WhirlyGeometry.h"
#import "VectorData.h"


using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{

OverlapHelper::OverlapHelper(const Mbr &mbr,int sizeX,int sizeY)
	: mbr(mbr), sizeX(sizeX), sizeY(sizeY)
{
    grid.resize(sizeX*sizeY);
    cellSize = Point2f((mbr.ur().x()-mbr.ll().x())/sizeX,(mbr.ur().y()-mbr.ll().y())/sizeY);
}

// Try to add an object.  Might fail (kind of the whole point).
bool OverlapHelper::addObject(const Point2dVector &pts)
{
    Mbr objMbr;
    for (unsigned int ii=0;ii<pts.size();ii++)
        objMbr.addPoint(pts[ii]);
    int sx = floorf((objMbr.ll().x()-mbr.ll().x())/cellSize.x());
    if (sx < 0) sx = 0;
    int sy = floorf((objMbr.ll().y()-mbr.ll().y())/cellSize.y());
    if (sy < 0) sy = 0;
    int ex = ceilf((objMbr.ur().x()-mbr.ll().x())/cellSize.x());
    if (ex >= sizeX)  ex = sizeX-1;
    int ey = ceilf((objMbr.ur().y()-mbr.ll().y())/cellSize.y());
    if (ey >= sizeY)  ey = sizeY-1;
    for (int ix=sx;ix<=ex;ix++)
        for (int iy=sy;iy<=ey;iy++)
        {
            std::vector<int> &objList = grid[iy*sizeX + ix];
            for (unsigned int ii=0;ii<objList.size();ii++)
            {
                BoundedObject &testObj = objects[objList[ii]];
                // Note: This will result in testing the same thing multiple times
                if (ConvexPolyIntersect(testObj.pts,pts))
                    return false;
                }
            }

    // Okay, so it doesn't overlap.  Let's add it where needed.
    objects.resize(objects.size()+1);
    int newId = (int)(objects.size()-1);
    BoundedObject &newObj = objects[newId];
    newObj.pts = pts;
    for (int ix=sx;ix<=ex;ix++)
        for (int iy=sy;iy<=ey;iy++)
        {
            std::vector<int> &objList = grid[iy*sizeX + ix];
            objList.push_back(newId);
        }

    return true;
}

ClusterHelper::ObjectWithBounds::ObjectWithBounds()
{
}

ClusterHelper::SimpleObject::SimpleObject()
	: objEntry(NULL), parentObject(-1)
{
}

ClusterHelper::ClusterObject::ClusterObject()
{
}

ClusterHelper::ClusterHelper(const Mbr &mbr,int sizeX,int sizeY,float resScale,const Point2d &clusterMarkerSize)
	: mbr(mbr), sizeX(sizeX), sizeY(sizeY), resScale(resScale), clusterMarkerSize(clusterMarkerSize)
{
    grid.resize(sizeX*sizeY);
    cellSize = Point2d((mbr.ur().x()-mbr.ll().x())/sizeX,(mbr.ur().y()-mbr.ll().y())/sizeY);
}

void ClusterHelper::calcCells(const Mbr &checkMbr,int &sx,int &sy,int &ex,int &ey)
{
    sx = floorf((checkMbr.ll().x()-mbr.ll().x())/cellSize.x());
    if (sx < 0) sx = 0;
    sy = floorf((checkMbr.ll().y()-mbr.ll().y())/cellSize.y());
    if (sy < 0) sy = 0;
    ex = ceilf((checkMbr.ur().x()-mbr.ll().x())/cellSize.x());
    if (ex >= sizeX)  ex = sizeX-1;
    ey = ceilf((checkMbr.ur().y()-mbr.ll().y())/cellSize.y());
    if (ey >= sizeY)  ey = sizeY-1;
}

void ClusterHelper::addToCells(const Mbr &mbr,int index)
{
    int sx,sy,ex,ey;
    calcCells(mbr,sx,sy,ex,ey);

    // Add the new object to the grid
    for (int ix=sx;ix<=ex;ix++)
        for (int iy=sy;iy<=ey;iy++)
        {
            std::set<int> &objSet = grid[iy*sizeX + ix];
            objSet.insert(index);
        }
}

void ClusterHelper::removeFromCells(const Mbr &mbr,int index)
{
    int sx,sy,ex,ey;
    calcCells(mbr,sx,sy,ex,ey);

    // Add the new object to the grid
    for (int ix=sx;ix<=ex;ix++)
        for (int iy=sy;iy<=ey;iy++)
        {
            std::set<int> &objSet = grid[iy*sizeX + ix];
            objSet.erase(index);
        }
}

void ClusterHelper::findObjectsWithin(const Mbr &mbr,std::set<int> &objSet)
{
    int sx,sy,ex,ey;
    calcCells(mbr,sx,sy,ex,ey);

    for (int ix=sx;ix<=ex;ix++)
    {
        for (int iy=sy;iy<=ey;iy++)
        {
            std::set<int> &thisGridSet = grid[iy*sizeX + ix];
            objSet.insert(thisGridSet.begin(),thisGridSet.end());
        }
    }
}

// Try to add an object.  Might fail (kind of the whole point).
void ClusterHelper::addObject(LayoutObjectEntry *objEntry,const Point2dVector pts)
{
    // We'll add this one way or another
    simpleObjects.resize(simpleObjects.size()+1);
    int newID = (int)(simpleObjects.size()-1);
    SimpleObject &newObj = simpleObjects[newID];
    newObj.objEntry = objEntry;
    newObj.center = CalcCenterOfMass(pts);
    newObj.pts = pts;
    Mbr mbr;  mbr.addPoints(pts);

    // All the things we might overlap
    std::set<int> objSet;
    findObjectsWithin(mbr,objSet);

    // Look for overlaps
    bool found = false;
    for (auto which : objSet)
    {
        ObjectWithBounds *testObj = NULL;
        SimpleObject *simpleObj = NULL;
        ClusterObject *clusterObj = NULL;
        if (which >= 0)
        {
            simpleObj = &simpleObjects[which];
            testObj = simpleObj;
        } else {
            clusterObj = &clusterObjects[-(which+1)];
            testObj = clusterObj;
        }

        if (ConvexPolyIntersect(testObj->pts,newObj.pts))
        {
            int clusterID;

            if (clusterObj)
            {
                Mbr clusterOldMbr;  clusterOldMbr.addPoints(clusterObj->pts);
                removeFromCells(clusterOldMbr, which);

                // Hit a cluster, so merge this new object in
                clusterObj->children.push_back(newID);
                clusterObj->center = (clusterObj->center * (clusterObj->children.size() - 1) + newObj.center)/clusterObj->children.size();
                clusterObj->pts.clear();
                clusterID = -(which+1);
            } else {
                // Hit another test object.  Remove it from the grid
                Mbr testMbr;  testMbr.addPoints(testObj->pts);
                removeFromCells(testMbr, which);

                // Make up a cluster for the two of them.
                clusterID = clusterObjects.size();
                clusterObjects.resize(clusterObjects.size()+1);
                clusterObj = &clusterObjects[clusterID];
                clusterObj->children.push_back(which);
                clusterObj->children.push_back(newID);
                clusterObj->center = (newObj.center + testObj->center)/2.0;

                simpleObj->parentObject = clusterID;
            }

            newObj.parentObject = clusterID;
            clusterObj->pts.reserve(4);
            clusterObj->pts.push_back(clusterObj->center + Point2d(-clusterMarkerSize.x()*resScale/2.0,-clusterMarkerSize.y()*resScale/2.0));
            clusterObj->pts.push_back(clusterObj->center + Point2d(clusterMarkerSize.x()*resScale/2.0,-clusterMarkerSize.y()*resScale/2.0));
            clusterObj->pts.push_back(clusterObj->center + Point2d(clusterMarkerSize.x()*resScale/2.0,clusterMarkerSize.y()*resScale/2.0));
            clusterObj->pts.push_back(clusterObj->center + Point2d(-clusterMarkerSize.x()*resScale/2.0,clusterMarkerSize.y()*resScale/2.0));
            Mbr clusterMbr;  clusterMbr.addPoints(clusterObj->pts);
            addToCells(clusterMbr,-(clusterID+1));

            found = true;
            break;
        }
    }

    // This object stands alone, so add it to the grid
    if (!found)
        addToCells(mbr, newID);
}

void ClusterHelper::resolveClusters()
{
    // Find single objects that overlap existing clusters.
    // We won't move the clusters here to keep it simpler
    for (int so=0;so<simpleObjects.size();so++)
    {
        SimpleObject *simpleObj = &simpleObjects[so];
        if (simpleObj->parentObject < 0)
        {
            Mbr simpleMbr;  simpleMbr.addPoints(simpleObj->pts);
            std::set<int> testObjs;
            findObjectsWithin(simpleMbr, testObjs);
            for (auto which : testObjs)
            {
                // Only care about the clusters
                if (which < 0)
                {
                    ClusterObject *clusterObj = &clusterObjects[-(which+1)];

                    if (ConvexPolyIntersect(simpleObj->pts,clusterObj->pts))
                    {
                        simpleObj->parentObject = -(which + 1);
                        clusterObj->children.push_back(so);
                        break;
                    }
                }
            }
        }
    }

    // Look for clusters that overlap one another
    for (int ci=0;ci<clusterObjects.size();ci++)
    {
        ClusterObject *clusterObj = &clusterObjects[ci];
        if (!clusterObj->children.empty())
        {
            Mbr thisMbr;  thisMbr.addPoints(clusterObj->pts);
            std::set<int> testObjs;
            findObjectsWithin(thisMbr, testObjs);
            for (auto which : testObjs)
            {
                if (which < 0 && ci != -(which + 1))
                {
                    ClusterObject *otherClusterObj = &clusterObjects[-(which+1)];

                    if (!otherClusterObj->children.empty() && ConvexPolyIntersect(clusterObj->pts,otherClusterObj->pts))
                    {
                        clusterObj->children.insert(clusterObj->children.begin(),otherClusterObj->children.begin(), otherClusterObj->children.end());
                        otherClusterObj->children.clear();
                    }
                }
            }
        }
    }
}

void ClusterHelper::objectsForCluster(ClusterObject &cluster,std::vector<LayoutObjectEntry *> &layoutObjs)
{
    for (int child : cluster.children)
        layoutObjs.push_back(simpleObjects[child].objEntry);
}

}
