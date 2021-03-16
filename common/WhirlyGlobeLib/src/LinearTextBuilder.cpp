/*
 *  LinearTextBuilder.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/3/21.
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

#import "LinearTextBuilder.h"
#import "VectorOffset.h"
#import "GridClipper.h"

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
    float len = 0.0;
    for (unsigned int ii = start+1;ii<end;ii++) {
        float t;
        len += (screenPts[ii] - screenPts[ii-1]).norm();
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

LinearWalker::LinearWalker(const VectorRing &pts)
: pts(pts), ptSoFar(0), offsetDist(0.0), totalLength(0.0)
{
    for (unsigned int ii=0;ii<pts.size()-1;ii++) {
        totalLength += (pts[ii+1] - pts[ii]).norm();
    }
}

float LinearWalker::getTotalLength()
{
    return totalLength;
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

LinearTextBuilder::LinearTextBuilder(ViewStateRef viewState,
                                    unsigned int offi,
                                    const Point2f &frameBufferSize,
                                    float generalEps,
                                    LayoutObject *layoutObj)
: viewState(viewState), offi(offi), frameBufferSize(frameBufferSize), generalEps(generalEps),
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
    isClosed = pts.front() == pts.back();
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
    
    Eigen::Matrix4d modelTrans = viewState->fullMatrices[offi];
    
    Point2fVector screenPts;
    for (auto pt: pts) {
        Point2f thisObjPt = viewState->pointOnScreenFromDisplay(pt,&modelTrans,frameBufferSize);
        screenPts.push_back(thisObjPt);
    }
    
    // Make sure there's at least some overlap with the screen
    Mbr testMbr(screenPts);
    if (!testMbr.intersect(screenMbr).valid()) {
        return;
    }

    // Generalize the source line
    screenPts = LineGeneralization(screenPts,generalEps,0,screenPts.size());

    if (screenPts.empty())
        return;
    
    // Clip to a larger screen MBR
    {
        std::vector<VectorRing> newRuns;
        Mbr largeScreenMbr = screenMbr;
        largeScreenMbr.expandByFraction(0.1);
        ClipLoopToMbr(screenPts, largeScreenMbr, true, newRuns,1e6);
        runs = newRuns;
    }
    
    // Run the offsetting
    if (layoutObj->layoutOffset != 0.0) {
        std::vector<VectorRing> newRuns;
        for (const auto &run: runs) {
            std::vector<VectorRing> theseRuns;
            if (isClosed)
                theseRuns = BufferPolygon(run, layoutObj->layoutOffset);
            else
                theseRuns = BufferLinear(run, layoutObj->layoutOffset);
            newRuns.insert(newRuns.end(),theseRuns.begin(),theseRuns.end());
        }
        runs = newRuns;
    }
    
    // Clip to the screen
    {
        std::vector<VectorRing> newRuns;
        for (const auto &run: runs) {
            std::vector<VectorRing> theseRuns;
            ClipLoopToMbr(run, screenMbr, false, theseRuns);
            if (!theseRuns.empty())
                newRuns.insert(newRuns.end(),theseRuns.begin(),theseRuns.end());
        }
        runs = newRuns;
    }
    
    // Break up runs at unlikely angles
    {
        std::vector<VectorRing> newRuns;
        for (const auto &run: runs) {
            std::vector<VectorRing> theseRuns;
            VectorRing thisRun;
            thisRun.push_back(run.front());
            for (unsigned int ii=1;ii<run.size()-1;ii++) {
                const Point2f &l0 = run[ii-1], &l1 = run[ii], &l2 = run[ii+1];
                Point2f dir0 = (l1-l0).normalized(), dir1 = (l2-l1).normalized();
                double ang = acos(dir0.dot(-dir1));
                if (ang > 135.0 / 180.0 * M_PI)
                    thisRun.push_back(l1);
                else {
                    thisRun.push_back(l1);
                    theseRuns.push_back(thisRun);
                    thisRun.clear();
                    thisRun.push_back(l1);
                }
            }
            thisRun.push_back(run.back());
            if (thisRun.size() > 1)
                theseRuns.push_back(thisRun);
            if (!theseRuns.empty())
                newRuns.insert(newRuns.end(),theseRuns.begin(),theseRuns.end());
        }
        runs = newRuns;
    }

    return;
}

std::vector<VectorRing> LinearTextBuilder::getScreenVecs()
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

ShapeSet LinearTextBuilder::getVisualVecs()
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
