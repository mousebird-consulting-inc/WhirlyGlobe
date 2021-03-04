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

namespace WhirlyKit
{

// https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
Point2fVector LineGeneralization(const Point2fVector &screenPts,float eps,unsigned int start,unsigned int end)
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
    if (dMax > eps) {
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
                                    const Mbr &screenMbr,
                                    const Point2f &frameBufferSize,
                                    LayoutObject *layoutObj)
: viewState(viewState), offi(offi), screenMbr(screenMbr), frameBufferSize(frameBufferSize),
layoutObj(layoutObj)
{
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

    // Filter down to 1pixel to clean up duplicates and such
    screenPts = LineGeneralization(screenPts,10.0,0,screenPts.size());

    // Offset if needed.  Might be left/right inside/outside
    if (layoutObj->layoutOffset != 0.0) {
        Point2fVector newScreenPts;

        bool first = true;
        for (int ii=1;ii<screenPts.size()-1;ii++) {
            // Offset the lines and then intersect
            const Point2f &l0 = screenPts[ii-1],&l1 = screenPts[ii], &l2 = screenPts[ii+1];
            if (l0 == l1)
                continue;
            Point2f dir0 = (l1 - l0).normalized(), dir1 = (l2 - l1).normalized();
            dir0 = Point2f(-dir0.y(),dir0.x());  dir1 = Point2f(-dir1.y(),dir1.x());
            Point2f na0 = dir0 * layoutObj->layoutOffset + l0;
            Point2f na1 = dir0 * layoutObj->layoutOffset + l1;
            Point2f nb0 = dir1 * layoutObj->layoutOffset + l1;
            Point2f nb1 = dir1 * layoutObj->layoutOffset + l2;
            Point2f cPt;
            if (first) {
                newScreenPts.push_back(na0);
                first = false;
            }
            if (l1 == l2) {
                newScreenPts.push_back(na1);
            } else {
                bool useIntersection = IntersectLines(na0,na1,nb0,nb1,&cPt);
                // Make sure we didn't get a point too far away
//                if (useIntersection) {
//                    double dist = (cPt - l1).norm();
//                    useIntersection = dist < 3.0*layoutObj->layoutOffset;
//                }
                if (useIntersection) {
                    newScreenPts.push_back(cPt);
                } else
                    newScreenPts.push_back(nb0);
            }
            if (ii==screenPts.size()-1)
                newScreenPts.push_back(nb1);
        }
        screenPts = newScreenPts;
    }
            
    // Filter out anything outside the screen MBR
    {
        std::vector<bool> insidePts;
        insidePts.reserve(screenPts.size());
        for (const auto &pt: screenPts) {
            bool inside = false;
            if (screenMbr.inside(pt))
                inside = true;
            insidePts.push_back(inside);
        }
        
        int startPt = 0,endPt = 0;
        while (startPt < screenPts.size()) {
            // Find the starting point
            for (;startPt<screenPts.size();startPt++) {
                if (insidePts[startPt] ||
                    (startPt < screenPts.size()-1 && insidePts[startPt+1]))
                    break;
            }
            
            // Find the end point
            for (endPt=startPt+1;endPt<screenPts.size();endPt++) {
                if (!insidePts[endPt])
                    break;
            }
            
            // Add this run
            if (startPt < endPt && startPt < screenPts.size()) {
                VectorRing thesePts;
                int copyTo = endPt < screenPts.size() - 1 ? endPt+1 : endPt;
                thesePts.insert(thesePts.end(), screenPts.begin()+startPt, screenPts.begin()+copyTo);
                runs.push_back(thesePts);
            }
                        
            // On to the next one
            startPt = endPt;
        }
    }
    if (screenPts.empty())
        return;
        
    // Ye olde Douglas-Peuker on the line at 3 pixels
//    const float Epsilon = 40.0;
//    screenPts = LineGeneralization(screenPts,Epsilon,0,screenPts.size());
    
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
