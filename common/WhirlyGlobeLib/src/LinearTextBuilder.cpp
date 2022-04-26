/*  LinearTextBuilder.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/3/21.
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

#import "LinearTextBuilder.h"
#import "VectorOffset.h"
#import "GridClipper.h"
#import "WhirlyKitLog.h"

using namespace Eigen;

namespace WhirlyKit
{

// https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
Point2fVector LineGeneralization(const Point2fVector &screenPts,
                                 float eps,
                                 unsigned int start,unsigned int end)
{
    if (screenPts.size() < 3)
        return screenPts;
    
    // Find the point with max distance
    float dMax = 0.0;
    unsigned int maxIdx = 0;
    for (unsigned int ii = start+1;ii<end;ii++) {
        float t;
        Point2f pt = ClosestPointOnLineSegment(screenPts[start], screenPts[end], screenPts[ii], t);
        float dist = (pt-screenPts[ii]).norm();
        if (dist > dMax) {
            maxIdx = ii;
            dMax = dist;
        }
    }
    
    // If max distance is greater than the epsilon, recursively simplify
    Point2fVector pts;
    if (maxIdx != 0 && (dMax > eps)) {
        Point2fVector pts0 = LineGeneralization(screenPts, eps, start, maxIdx);
        Point2fVector pts1 = LineGeneralization(screenPts, eps, maxIdx, end);
        pts.insert(pts.end(),pts0.begin(),pts0.end());
        pts.insert(pts.end(),pts1.begin(),pts1.end());
    } else {
        pts.push_back(screenPts[start]);
        if (start != end-1)
            pts.push_back(screenPts[end-1]);
    }
    
    return pts;
}

LinearWalker::LinearWalker(const VectorRing &pts) :
    pts(pts)
{
    for (unsigned int ii=0;ii<pts.size()-1;ii++)
    {
        totalLength += (pts[ii+1] - pts[ii]).norm();
    }
}

bool LinearWalker::nextPoint(double dist,Point2f *retPt,Point2f *norm,bool savePos)
{
    if (ptSoFar >= pts.size()-1)
        return false;
    
    float travelSoFar = 0.0;
    int newPtSoFar = ptSoFar;
    while (newPtSoFar < pts.size()-1) {
        Point2f dir = pts[newPtSoFar+1] - pts[newPtSoFar];
        float segLen = dir.norm();
        // We're in this segment
        if ((dist-travelSoFar) < (segLen-offsetDist)) {
            float newOffsetDist = offsetDist + (dist-travelSoFar);
            if (retPt)
                *retPt = pts[newPtSoFar] + newOffsetDist/segLen * dir;
            if (norm)
                *norm = Point2f(-dir.y(),dir.x()).normalized();
            if (savePos) {
                offsetDist = newOffsetDist;
                ptSoFar = newPtSoFar;
            }
            return true;
        }
        // Keep going
        travelSoFar += segLen;
        newPtSoFar++;
    }
    
    return false;
}

LinearTextBuilder::LinearTextBuilder(ViewStateRef inViewState,
                                    unsigned int offi,
                                    const Point2f &frameBufferSize,
                                    float generalEps,
                                    LayoutObject *layoutObj) :
    viewState(std::move(inViewState)),
    generalEps(generalEps),
    offi(offi),
    frameBufferSize(frameBufferSize),
    layoutObj(layoutObj)
{
    screenMbr.addPoint(Point2f(0.0,0.0));
    screenMbr.addPoint(Point2f(frameBufferSize.x(),frameBufferSize.y()));
    
    coordAdapt = viewState->coordAdapter;
    coordSys = coordAdapt->getCoordSystem();
    globeViewState = dynamic_cast<WhirlyGlobe::GlobeViewState *>(viewState.get());
    mapViewState = dynamic_cast<Maply::MapViewState *>(viewState.get());
}

void LinearTextBuilder::setPoints(const Point3dVector &inPts)
{
    pts = inPts;
}

void LinearTextBuilder::sortRuns(double minLen)
{
    std::vector<std::pair<VectorRing,double> > newRuns;
    
    // Build up the runs with length
    for (const auto &run: runs) {
        double len = 0.0;
        for (unsigned int ii=0;ii<run.size()-1;ii++) {
            len += (run[ii+1] - run[ii]).norm();
        }
        if (len > minLen)
            newRuns.push_back(std::pair<VectorRing,double>(run,len));
    }
    
    std::sort(newRuns.begin(), newRuns.end(),
              [](const std::pair<VectorRing,double>& a, const std::pair<VectorRing,double>& b) -> bool
    {
        return a.second < b.second;
    });
    
    runs.clear();
    for (auto const &run: newRuns)
        runs.push_back(run.first);
}

void LinearTextBuilder::process()
{
    if (pts.size() == 1)
        return;
    
    Matrix4d modelTrans = viewState->fullMatrices[offi];
    Matrix4d fullNormalMatrix = viewState->fullNormalMatrices[offi];

    {
        std::vector<VectorRing> newRuns;
        VectorRing curRun;
        
        // Project the points and evaluate the individual validity of each one
        std::vector<bool> isValid;  isValid.reserve(pts.size()+1);
        std::vector<bool> isFrontSide;  isFrontSide.reserve(pts.size()+1);
        std::vector<Point2f> projPts;  projPts.reserve(pts.size()+1);
        for (auto pt: pts) {
            Point2f thisObjPt = viewState->pointOnScreenFromDisplay(pt,&modelTrans,frameBufferSize);

            bool testFrontSide = true;
            if (globeViewState)
            {
                // Make sure this one is facing toward the viewer
                if (!(CheckPointAndNormFacing(pt,pt.normalized(),modelTrans,fullNormalMatrix) > 0.0))
                    testFrontSide = false;
            }
            
            bool isInside = screenMbr.inside(thisObjPt);
            isValid.push_back(isInside && testFrontSide);
            isFrontSide.push_back(testFrontSide);
            projPts.push_back(thisObjPt);
        }
        // If it's closed, tack on some end points
        if (pts.front() == pts.back()) {
            isValid.push_back(isValid.front());
            isFrontSide.push_back(isValid.front());
            projPts.push_back(projPts.front());
        }
        
        // Now build runs that cut across the screen MBR
        // It gets tricky.  We have to include the first and last point of a run
        //  and we want to look for spans that can overlap the MBR
        bool includeNext = false;
        for (unsigned int ii=0;ii<projPts.size();ii++) {
            bool include = includeNext || isValid[ii];  // Simplest case
            if (!include) {
                // Previous point is valid, so keep this one
                if (ii>0 && isValid[ii-1])
                    include = true;
                // Next point is valid, so keep this one
                if (ii<projPts.size()-1 && isValid[ii+1])
                    include = true;
            }

            includeNext = false;
            if (!include) {
                // Is this part of a span that overlaps the MBR
                if (ii < projPts.size()-1 && isFrontSide[ii] && isFrontSide[ii+1]) {
                    Mbr testMbr;
                    testMbr.addPoint(projPts[ii]);
                    testMbr.addPoint(projPts[ii+1]);
                    if (screenMbr.overlaps(testMbr)) {
                        include = true;
                        includeNext = true;
                    }
                }
            }

            // Include this point in the run
            if (include)
                curRun.push_back(projPts[ii]);
            else {
                // Cut off the run and add it to the current runs
                if (curRun.size() > 1)
                    newRuns.push_back(curRun);
                curRun.clear();
            }
        }
        
        if (curRun.size() > 1)
            newRuns.push_back(curRun);
        
        runs = newRuns;
        
//        wkLogLevel(Debug,"Found %d runs",runs.size());
//        for (unsigned int ii=0;ii<runs.size();ii++)
//            wkLogLevel(Debug, "  Run %d: %d points",ii,runs[ii].size());
    }
    
    if (runs.empty())
        return;

    {
        std::vector<VectorRing> newRuns;

        for (const auto &run: runs) {
            // Generalize the source line
            auto newRun = LineGeneralization(run,generalEps,0,run.size()-1);
            if (newRun.size() > 1)
                newRuns.push_back(newRun);
        }
                
        runs = newRuns;

//        wkLogLevel(Debug,"Generalize %d runs",runs.size());
//        for (unsigned int ii=0;ii<runs.size();ii++)
//            wkLogLevel(Debug, "  Run %d: %d points",ii,runs[ii].size());
    }

    if (runs.empty())
        return;
        
    // Run the offsetting
    if (layoutObj->layoutOffset != 0.0) {
        std::vector<VectorRing> newRuns;
        for (const auto &run: runs) {
            std::vector<VectorRing> theseRuns;
            if (run.front() == run.back())
                theseRuns = BufferPolygon(run, layoutObj->layoutOffset);
            else {
                theseRuns = BufferLinear(run, layoutObj->layoutOffset);
            }
            newRuns.insert(newRuns.end(),theseRuns.begin(),theseRuns.end());
        }
        runs = newRuns;

//        wkLogLevel(Debug,"Offset %d runs",runs.size());
//        for (unsigned int ii=0;ii<runs.size();ii++)
//            wkLogLevel(Debug, "  Run %d: %d points",ii,runs[ii].size());
    }
        
    // Break up runs at unlikely angles
//    {
//        std::vector<VectorRing> newRuns;
//        for (const auto &run: runs) {
//            std::vector<VectorRing> theseRuns;
//            VectorRing thisRun;
//            thisRun.push_back(run.front());
//            for (unsigned int ii=1;ii<run.size()-1;ii++) {
//                const Point2f &l0 = run[ii-1], &l1 = run[ii], &l2 = run[ii+1];
//                Point2f dir0 = (l1-l0).normalized(), dir1 = (l2-l1).normalized();
//                double ang = acos(dir0.dot(-dir1));
//                if (ang > 90.0 / 180.0 * M_PI)
//                    thisRun.push_back(l1);
//                else {
//                    thisRun.push_back(l1);
//                    theseRuns.push_back(thisRun);
//                    thisRun.clear();
//                    thisRun.push_back(l1);
//                }
//            }
//            thisRun.push_back(run.back());
//            if (thisRun.size() > 1)
//                theseRuns.push_back(thisRun);
//            if (!theseRuns.empty())
//                newRuns.insert(newRuns.end(),theseRuns.begin(),theseRuns.end());
//        }
//        runs = newRuns;
//    }

    return;
}

std::vector<VectorRing> LinearTextBuilder::getScreenVecs() const
{
    return runs;
}

const std::vector<VectorRing> &LinearTextBuilder::getScreenVecsRef() const
{
    return runs;
}


bool LinearTextBuilder::screenToWorld(const Point2f &pt,Point3d &outPt)
{
    if (globeViewState) {
        Point3d modelPt;
        if (globeViewState->pointOnSphereFromScreen(pt, viewState->fullMatrices[offi], frameBufferSize, modelPt, false)) {
            outPt = modelPt;
            return true;
        }
    } else {
        Point3d modelPt;
        if (mapViewState->pointOnPlaneFromScreen(pt, viewState->fullMatrices[offi], frameBufferSize, modelPt, false)) {
            outPt = modelPt;
            return true;
        }
    }
    
    return false;
}

Point2f LinearTextBuilder::worldToScreen(const Point3d &worldPt)
{
    return viewState->pointOnScreenFromDisplay(worldPt, &viewState->fullMatrices[offi], frameBufferSize);
}

ShapeSet LinearTextBuilder::getVisualVecs() const
{
    ShapeSet retShapes;

    for (auto &vec: runs) {
        VectorLinearRef lin = VectorLinear::createLinear();
        
        for (auto &pt: vec) {
            if (globeViewState) {
                Point3d modelPt;
                if (globeViewState->pointOnSphereFromScreen(pt, viewState->fullMatrices[offi], frameBufferSize, modelPt, false)) {
                    GeoCoord geoPt = coordSys->localToGeographic(coordAdapt->displayToLocal(modelPt));
                    lin->pts.push_back(Point2f(geoPt.x(),geoPt.y()));
                }
            } else {
                Point3d modelPt;
                if (mapViewState->pointOnPlaneFromScreen(pt, viewState->fullMatrices[offi], frameBufferSize, modelPt, false)) {
                    GeoCoord geoPt = coordSys->localToGeographic(coordAdapt->displayToLocal(modelPt));
                    lin->pts.push_back(Point2f(geoPt.x(),geoPt.y()));
                }
            }
        }
        
        lin->initGeoMbr();
        retShapes.insert(lin);
    }
    
    return retShapes;
}

double LinearTextBuilder::getViewStateRotation()
{
//    if (globeViewState) {
//        Point3d up = globeViewState->currentUp();
//        double ang = atan2(up.)
//    }
    
//    if (globeViewState)
//        return globeViewState->currentUp();
//    else if (mapViewState)
//        return mapViewState->
    return 0.0;
}


}
