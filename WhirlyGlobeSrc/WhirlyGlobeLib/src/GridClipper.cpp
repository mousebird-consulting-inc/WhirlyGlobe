/*
 *  GridClipper.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/16/11.
 *  Copyright 2011-2015 mousebird consulting
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

#import "GridClipper.h"
#import "cpp/clipper.hpp"

namespace WhirlyKit
{
    
using namespace ClipperLib;

static float PolyScale = 1e14;

// Clip the given loop to the given MBR
bool ClipLoopToMbr(const VectorRing &ring,const Mbr &mbr, bool closed,std::vector<VectorRing> &rets)
{
    Path subject(ring.size());
    for (unsigned int ii=0;ii<ring.size();ii++)
    {
        const Point2f &pt = ring[ii];
        subject[ii] = IntPoint(pt.x()*PolyScale,pt.y()*PolyScale);
    }
    Path clip(4);
    clip[0] = IntPoint(mbr.ll().x()*PolyScale,mbr.ll().y()*PolyScale);
    clip[1] = IntPoint(mbr.ur().x()*PolyScale,mbr.ll().y()*PolyScale);
    clip[2] = IntPoint(mbr.ur().x()*PolyScale,mbr.ur().y()*PolyScale);
    clip[3] = IntPoint(mbr.ll().x()*PolyScale,mbr.ur().y()*PolyScale);
    
    Clipper c;
    c.AddPath(subject, ptSubject, closed);
    c.AddPath(clip, ptClip, true);
    Paths solution;
  
    if(!closed)
    {
        PolyTree polyTreeSolution = *new PolyTree();
        c.Execute(ctIntersection, polyTreeSolution, pftEvenOdd, pftEvenOdd);
        PolyTreeToPaths(polyTreeSolution, solution);
    } else
    {
        if (!c.Execute(ctIntersection, solution))
    {
            return false;
        }
    }
    
        for (unsigned int ii=0;ii<solution.size();ii++)
        {
        Path &outPoly = solution[ii];
            VectorRing outRing;
            for (unsigned jj=0;jj<outPoly.size();jj++)
            {
                IntPoint &outPt = outPoly[jj];
                outRing.push_back(Point2f(outPt.X/PolyScale,outPt.Y/PolyScale));
            }
     
        if ((closed && outRing.size() > 2) || (!closed && outRing.size() > 1))
                rets.push_back(outRing);
        }
    return true;
}
    
// Clip the given loop to the given grid (org and spacing)
// Return true on success and the new polygons in the rets
// Note: Not deeply efficient
bool ClipLoopToGrid(const VectorRing &ring,Point2f org,Point2f spacing,std::vector<VectorRing> &rets)
{
    Mbr mbr(ring);
    int startRet = (int)(rets.size());
    
    int ll_ix = (int)std::floor((mbr.ll().x()-org.x())/spacing.x());
    int ll_iy = (int)std::floor((mbr.ll().y()-org.y())/spacing.y());
    int ur_ix = (int)std::ceil((mbr.ur().x()-org.x())/spacing.x());
    int ur_iy = (int)std::ceil((mbr.ur().y()-org.y())/spacing.y());
    
    // Clip in strips from left to right
    for (int ix=ll_ix;ix<=ur_ix;ix++)
    {
        Point2f l0(ix*spacing.x()+org.x(),mbr.ll().y());
        Point2f l1((ix+1)*spacing.x()+org.x(),mbr.ur().y());
        Mbr left(l0,l1);
        
        std::vector<VectorRing> leftStrip;
        ClipLoopToMbr(ring,left, true, leftStrip);
        
        // Now clip the left strip vertically
        for (int iy=ll_iy;iy<=ur_iy;iy++)
        {
            
            Point2f b0(mbr.ll().x(),iy*spacing.y()+org.y());
            Point2f b1(mbr.ur().x(),(iy+1)*spacing.y()+org.y());
            Mbr bot(b0,b1);
            for (unsigned int ic=0;ic<leftStrip.size();ic++)
                ClipLoopToMbr(leftStrip[ic], bot, true, rets);
        }
    }
    
    for (unsigned int ii=startRet;ii<rets.size();ii++)
    {
        VectorRing &theRing = rets[ii];
        std::reverse(theRing.begin(),theRing.end());
    }
    
    return true;
}

}
