/*
 *  VectorOffset.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/4/21.
 *  Copyright 2011-2021 mousebird consulting
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

#import "VectorOffset.h"
#import "clipper.hpp"

using namespace ClipperLib;

namespace WhirlyKit
{

static float PolyScale = 1e6;

std::vector<VectorRing> BufferLinear(const VectorRing &ring, float offset)
{
    // If the offset is negative, just flip the geometry
    VectorRing theRing = ring;
    if (offset < 0) {
        offset *= -1.0;
        std::reverse(theRing.begin(), theRing.end());
    }
    
    Mbr mbr(ring);
    Point2f org = mbr.ll();

    Path path;
    for (const auto &pt: ring)
        path.push_back(IntPoint((pt.x() - org.x()) * PolyScale, (pt.y() - org.y()) * PolyScale));

    ClipperOffset co;
    Paths soln;
    co.AddPath(path, jtMiter, etOpenButt);
    co.Execute(soln, offset * PolyScale);
    
    std::vector<VectorRing> rets;
    for (unsigned int ii=0;ii<soln.size();ii++) {
        Path &outPoly = soln[ii];
        VectorRing outRing;
        for (unsigned jj=0;jj<outPoly.size();jj++) {
            IntPoint &outPt = outPoly[jj];
            outRing.push_back(Point2f(outPt.X/PolyScale + org.x(),outPt.Y/PolyScale + org.y()));
        }
        
        if (outRing.size() > 2)
            rets.push_back(outRing);
    }
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
