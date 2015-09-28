/*
 *  OverlapHelper.mm
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

using namespace Eigen;

namespace WhirlyKit
{

OverlapHelper::OverlapHelper(const Mbr &mbr,int sizeX,int sizeY)
: mbr(mbr), sizeX(sizeX), sizeY(sizeY)
{
    grid.resize(sizeX*sizeY);
    cellSize = Point2f((mbr.ur().x()-mbr.ll().x())/sizeX,(mbr.ur().y()-mbr.ll().y())/sizeY);
}

// Try to add an object.  Might fail (kind of the whole point).
bool OverlapHelper::addObject(const std::vector<Point2d> &pts)
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
    
}
