/*  VectorOffset.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/4/21.
 *  Copyright 2011-2022 mousebird consulting
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

#import "VectorOffset.h"
#import "clipper.hpp"
#import "Platform.h"

using namespace ClipperLib;

namespace WhirlyKit
{

static float PolyScale = 1e6;

// Check that the given point is on the positive side of the input ring
static bool isOnRightSide(const VectorRing &ring, float offset, const Point2f &pt)
{
    // Look for the closest line segment
    int found = -1;
    float dist = MAXFLOAT;
    for (int ii=0;ii<ring.size()-1;ii++) {
        float t;
        Point2f closePt = ClosestPointOnLineSegment(ring[ii],ring[ii+1],pt,t);
        float thisDist = (closePt - pt).norm();
        if (thisDist < dist) {
            found = ii;
            dist = thisDist;
        }
    }
    
    if (found >= 0) {
        const auto &p0 = ring[found], &p1 = ring[found+1];
        float sign = (p1.x() - p0.x()) * (pt.y() - p0.y()) -
            (p1.y() - p0.y()) * (pt.x() - p0.x());
        return sign > 0.0;
    }
    
    return false;
}

std::vector<VectorRing> BufferLinear(const VectorRing &ring, float offset)
{
    // If the offset is negative, just flip the geometry
    VectorRing theRing = ring;
    if (offset < 0) {
        offset *= -1.0;
        std::reverse(theRing.begin(), theRing.end());
    }
    
    Mbr mbr(theRing);
    Point2f org = mbr.ll();

    Path path;
    for (const auto &pt: theRing)
        path.push_back(IntPoint((pt.x() - org.x()) * PolyScale, (pt.y() - org.y()) * PolyScale));

    ClipperOffset co;
    Paths soln;
    co.AddPath(path, jtMiter, etOpenButt);
    co.Execute(soln, offset * PolyScale);
    
    std::vector<VectorRing> rets;
    for (unsigned int ii=0;ii<soln.size();ii++) {
        Path &outPoly = soln[ii];
        VectorRing outRing;
        std::vector<bool> validPts;  validPts.reserve(outPoly.size());
        std::vector<Point2f> outPts;  outPts.reserve(outPoly.size());
        for (unsigned jj=0;jj<outPoly.size();jj++) {
            IntPoint &outPt = outPoly[jj];
            
            // So when Clipper buffers a line it buffers *around* the line.  We actually only want
            //  half of these points
            auto theOutPt = Point2f(outPt.X/PolyScale + org.x(),outPt.Y/PolyScale + org.y());
            outPts.push_back(theOutPt);
            validPts.push_back(isOnRightSide(ring, offset, theOutPt));
        }
        
        if (outPoly.size() <= 1)
            continue;
            
        // Find a good starting point
        int startPt = 0;
        for (;startPt<outPts.size();startPt++)
            if (!validPts[startPt])
                break;
        if (startPt == outPts.size())
            startPt = 0;
        
        // Now walk around the outside until we've touched it all once
        for (unsigned int jj=0;jj<outPts.size();jj++,startPt++) {
            if (validPts[startPt % outPoly.size()])
                outRing.push_back(outPts[startPt % outPoly.size()]);
            else {
                if (outRing.size() > 1) {
                    rets.push_back(outRing);
                }
                outRing.clear();
            }
        }
        if (outRing.size() > 1)
            rets.push_back(outRing);
    }

    for (auto &run: rets)
        std::reverse(run.begin(),run.end());

    return rets;
}

/// Offset/buffer a polygon into one or more loops
std::vector<VectorRing> BufferPolygon(const VectorRing &ring, float offset)
{
    Point2f org = ring[0];
    
    Path path;
    for (const auto &pt: ring)
        path.push_back(IntPoint((pt.x() - org.x()) * PolyScale, (pt.y() - org.y()) * PolyScale));
    path.push_back(IntPoint((ring[0].x() - org.x()) * PolyScale, (ring[0].y() - org.y()) * PolyScale));

    ClipperOffset co;
    Paths soln;
    co.AddPath(path, jtMiter, etClosedPolygon);
    co.Execute(soln, offset * PolyScale);
    
    std::vector<VectorRing> rets;
    for (unsigned int ii=0;ii<soln.size();ii++) {
        Path &outPoly = soln[ii];
        VectorRing outRing;
        for (unsigned jj=0;jj<outPoly.size();jj++) {
            IntPoint &outPt = outPoly[jj];
            outRing.push_back(Point2f(outPt.X/PolyScale + org.x(),outPt.Y/PolyScale + org.y()));
        }
        outRing.push_back(Point2f(outPoly[0].X/PolyScale + org.x(),outPoly[0].Y/PolyScale + org.y()));
        
        if (outRing.size() > 2)
            rets.push_back(outRing);
    }
    return rets;
}

}
