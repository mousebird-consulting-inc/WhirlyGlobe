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

LinearTextBuilder::LinearTextBuilder(ViewStateRef viewState,
                                    unsigned int offi,
                                    const Point2f &frameBufferSize,
                                    LayoutObject *layoutObj)
: viewState(viewState), offi(offi), frameBufferSize(frameBufferSize),
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
    // TODO: Make this a parameter rather than 10px
    screenPts = LineGeneralization(screenPts,10.0,0,screenPts.size());

    if (screenPts.empty())
        return;

    // Run the offsetting
    if (layoutObj->layoutOffset != 0.0) {
        if (isClosed)
            runs = BufferPolygon(screenPts, layoutObj->layoutOffset);
        else
            runs = BufferLoop(screenPts, layoutObj->layoutOffset);
    } else {
        runs.push_back(screenPts);
    }
    
    // Clip to the screen
    std::vector<VectorRing> newRuns;
    for (const auto &run: runs) {
        std::vector<VectorRing> theseRuns;
        ClipLoopToMbr(run, screenMbr, false, theseRuns);
        if (!theseRuns.empty())
            newRuns.insert(newRuns.end(),theseRuns.begin(),theseRuns.end());
    }
    runs = newRuns;

    return;
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

}
