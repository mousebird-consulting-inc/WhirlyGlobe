/*
 *  GridClipper.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/16/11.
 *  Copyright 2011-2016 mousebird consulting
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
typedef int OutCode;
    
const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000

OutCode ComputeOutCode(double x, double y, const Mbr &mbr)
{
    OutCode code = INSIDE;
    
    if (x < mbr.ll().x())
        code |= LEFT;
    else if (x > mbr.ur().x())
        code |= RIGHT;
    if (y < mbr.ll().y())
        code |= BOTTOM;
    else if (y > mbr.ur().y())
        code |= TOP;
    
    return code;
}


// Clip the given loop to the given MBR
bool ClipLoopToMbr(const VectorRing &ring,const Mbr &mbr, bool closed,std::vector<VectorRing> &rets)
{
    if(!closed)
    {
        //Cohen-sutherland algorithm based on example implementation from wikipedia
        //https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm#Example_C.2FC.2B.2B_implementation
        VectorRing outRing;
        for (unsigned int ii=1;ii<ring.size();ii++)
        {
            Point2f p0 = ring[ii - 1];
            Point2f p1 = ring[ii];
            OutCode outcode0 = ComputeOutCode(p0.x(), p0.y(), mbr);
            OutCode outcode1 = ComputeOutCode(p1.x(), p1.y(), mbr);
            bool accept = false;
            
            while (true) {
                if (!(outcode0 | outcode1)) { // Both points in MBR
                    accept = true;
                    break;
                } else if (outcode0 & outcode1) { // Both points outside of MBR
                    break;
                } else { //At least one point outside of MBR
                    double x, y;
                    
                    // At least one endpoint is outside the clip rectangle; pick it.
                    OutCode outcodeOut = outcode0 ? outcode0 : outcode1;
                    
                    // Now find the intersection point;
                    // use formulas y = p0.y() + slope * (x - p0.x()), x = p0.x() + (1 / slope) * (y - p0.y())
                    if (outcodeOut & TOP) {           // point is above the clip rectangle
                        x = p0.x() + (p1.x() - p0.x()) * (mbr.ur().y() - p0.y()) / (p1.y() - p0.y());
                        y = mbr.ur().y();
                    } else if (outcodeOut & BOTTOM) { // point is below the clip rectangle
                        x = p0.x() + (p1.x() - p0.x()) * (mbr.ll().y() - p0.y()) / (p1.y() - p0.y());
                        y = mbr.ll().y();
                    } else if (outcodeOut & RIGHT) {  // point is to the right of clip rectangle
                        y = p0.y() + (p1.y() - p0.y()) * (mbr.ur().x() - p0.x()) / (p1.x() - p0.x());
                        x = mbr.ur().x();
                    } else if (outcodeOut & LEFT) {   // point is to the left of clip rectangle
                        y = p0.y() + (p1.y() - p0.y()) * (mbr.ll().x() - p0.x()) / (p1.x() - p0.x());
                        x = mbr.ll().x();
                    }
                    
                    // Now we move outside point to intersection point to clip
                    // and get ready for next pass.
                    if (outcodeOut == outcode0) {
                        p0.x() = x;
                        p0.y() = y;
                        outcode0 = ComputeOutCode(p0.x(), p0.y(), mbr);
                    } else {
                        p1.x() = x;
                        p1.y() = y;
                        outcode1 = ComputeOutCode(p1.x(), p1.y(), mbr);
                    }
                }
            }
            if (accept) {
                if (outRing.size() > 1 && outRing[outRing.size() - 1] != p0)
                {
                    rets.push_back(outRing);
                    outRing = VectorRing();
                }
                if(outRing.size() == 0) {
                    outRing.push_back(p0);
                }
                outRing.push_back(p1);
            }
        }
        
        if (outRing.size() > 1)
            rets.push_back(outRing);
    } else
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
        if (!c.Execute(ctIntersection, solution))
        {
            return false;
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
            
            if (outRing.size() > 2)
                rets.push_back(outRing);
        }
    }
    return true;
}

// Clip the given loop to the given MBR
bool ClipLoopsToMbr(const std::vector<VectorRing> &rings,const Mbr &mbr, bool closed,std::vector<VectorRing> &rets)
{
    Clipper c;
    
    for (const auto &ring: rings)
    {
        Path subject(ring.size());
        for (unsigned int ii=0;ii<ring.size();ii++)
    {
            const Point2f &pt = ring[ii];
            subject[ii] = IntPoint(pt.x()*PolyScale,pt.y()*PolyScale);
        }
        c.AddPath(subject, ptSubject, closed);
    }

    Path clip(4);
    clip[0] = IntPoint(mbr.ll().x()*PolyScale,mbr.ll().y()*PolyScale);
    clip[1] = IntPoint(mbr.ur().x()*PolyScale,mbr.ll().y()*PolyScale);
    clip[2] = IntPoint(mbr.ur().x()*PolyScale,mbr.ur().y()*PolyScale);
    clip[3] = IntPoint(mbr.ll().x()*PolyScale,mbr.ur().y()*PolyScale);
    
    c.AddPath(clip, ptClip, true);
    Paths solution;
        if (!c.Execute(ctIntersection, solution))
    {
            return false;
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
     
        if (outRing.size() > 2)
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

bool ClipLoopsToGrid(const std::vector<VectorRing> &rings,Point2f org,Point2f spacing,std::vector<VectorRing> &rets)
{
    Mbr mbr;
    for (const auto &ring : rings)
        mbr.addPoints(ring);
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
        ClipLoopsToMbr(rings,left, true, leftStrip);
        
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
