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

/**
 Used to lay text out along a line (with or without offset).
 A very specific implementation of a wacky algorithm.
 */
class LinearTextBuilder {
public:
    LinearTextBuilder(ViewStateRef viewState,
                      unsigned int offi,
                      const Mbr &screenMbr,
                      const Point2f &frameBufferSize,
                      LayoutObject *layoutObj);
    
    // Set the starting point, er points
    void setPoints(const Point3dVector &pts);
    
    // Run our crazy stuff
    void process();
    
    // Visual vectors for debugging
    ShapeSet getVisualVecs();
protected:
    CoordSystemDisplayAdapter *coordAdapt;
    CoordSystem *coordSys;
    ViewStateRef viewState;
    WhirlyGlobe::GlobeViewState *globeViewState;
    Maply::MapViewState *mapViewState;

    unsigned int offi;
    Mbr screenMbr;
    Point2f frameBufferSize;
    LayoutObject *layoutObj;

    Point3dVector pts;
    bool isClosed;
    
    std::vector<VectorRing> runs;
};

}
