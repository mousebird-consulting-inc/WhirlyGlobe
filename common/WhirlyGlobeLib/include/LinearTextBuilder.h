/*
 *  LinearTextBuilder.h
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

#import "WhirlyVector.h"
#import "WhirlyGeometry.h"
#import "WhirlyKitView.h"
#import "GlobeMath.h"
#import "LayoutManager.h"
#import "VectorData.h"

namespace WhirlyKit {

/// Ye olde Douglas Peucker line generalization algorithm.  Mostly.
Point2fVector LineGeneralization(const Point2fVector &screenPts,
                                 float eps,
                                 unsigned int start,unsigned int end);

/// Used to 'walk' along a linear feature by distance
class LinearWalker {
public:
    LinearWalker(const VectorRing &pts);
    
    /// Return the total length
    float getTotalLength();
    
    /// Calculate the next point along the line given the distance
    /// Or return false if there wasn't anything left
    bool nextPoint(double distance,Point2f *retPt=nullptr,Point2f *norm=nullptr,bool savePos=true);
    
protected:
    VectorRing pts;
    float totalLength;
    int ptSoFar;
    float offsetDist;
};

/**
 Used to lay text out along a line (with or without offset).
 A very specific implementation of a wacky algorithm.
 */
class LinearTextBuilder {
public:
    LinearTextBuilder(ViewStateRef viewState,
                      unsigned int offi,
                      const Point2f &frameBufferSize,
                      float generalEps,
                      LayoutObject *layoutObj);
    
    // Set the starting point, er points
    void setPoints(const Point3dVector &pts);
    
    // Sort the runs by length, toss the ones below the minimum length
    void sortRuns(double minLen);

    // Run our crazy stuff
    void process();
    
    // Return the individual runs to follow
    std::vector<VectorRing> getScreenVecs();
    
    // Visual vectors for debugging
    ShapeSet getVisualVecs();
    
    // Convert point from screen coordinates back to world coordinates
    bool screenToWorld(const Point2f &pt,Point3d &outPt);
    Point2f worldToScreen(const Point3d &worldPt);
    
    // Return the current rotation of the view state
    double getViewStateRotation();

protected:
    CoordSystemDisplayAdapter *coordAdapt;
    CoordSystem *coordSys;
    ViewStateRef viewState;
    WhirlyGlobe::GlobeViewState *globeViewState;
    Maply::MapViewState *mapViewState;

    float generalEps;
    unsigned int offi;
    Mbr screenMbr;
    Point2f frameBufferSize;
    LayoutObject *layoutObj;

    Point3dVector pts;
    bool isClosed;
    
    std::vector<VectorRing> runs;
};

}
