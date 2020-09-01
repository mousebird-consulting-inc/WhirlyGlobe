/*
 *  ScreenImportance.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/11/13.
 *  Copyright 2011-2019 mousebird consulting
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

#import "Platform.h"
#import <math.h>
#import "WhirlyVector.h"
#import "WhirlyKitView.h"
#import "Dictionary.h"
#import "Scene.h"
#import "GlobeMath.h"
#import "QuadTreeNew.h"
#import "SceneRenderer.h"


namespace WhirlyKit
{

/// A solid volume used to describe the display space a tile takes up.
/// We use these for screen space calculations and cache them in the tile
///  idents.
class DisplaySolid : public DelayedDeletable
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    /// Create a display solid, including height.
    DisplaySolid(const QuadTreeIdentifier &nodeIdent,const Mbr &nodeMbr,float minZ,float maxZ,CoordSystem *srcSystem,CoordSystemDisplayAdapter *coordAdapter);
    
    /// Returns true if the given point (in display space) is inside the volume
    bool isInside(const Point3d &pt);
    
    /// Calculate the importance for this display solid given the user's eye position
    double importanceForViewState(ViewState *viewState,const Point2f &frameSize);
    
    /// See if this display solid is current in the viewing frustum
    bool isOnScreenForViewState(ViewState *viewState,const Point2f &frameSize);
    
    /// Set by the constructor
    bool valid;
    
    /// The area sampled into representative polygons
    std::vector<Point3dVector > polys;
    /// Normals for all 5 or 6 planes
    Point3dVector normals;
    /// Normals for the surface.  We use these to make sure the solid is pointing towards us.
    Point3dVector surfNormals;
    /// Bounding box for all the generated polygons
    Point3d bbox0,bbox1;
};
    
typedef std::shared_ptr<DisplaySolid> DisplaySolidRef;

/// Check if any part of the given tile is on screen
bool TileIsOnScreen(WhirlyKit::ViewState *viewState,const WhirlyKit::Point2f &frameSize,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,const WhirlyKit::Mbr &nodeMbr,const QuadTreeIdentifier &nodeIdent,DisplaySolidRef &dispSold);

/// Utility function to calculate importance based on pixel screen size.
/// This would be used by the data source as a default.
double ScreenImportance(WhirlyKit::ViewState *viewState,const WhirlyKit::Point2f &frameSize,const Point3d &notUsed, int pixelsSqare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,const WhirlyKit::Mbr &nodeMbr, const QuadTreeIdentifier &nodeIdent,DisplaySolidRef &dispSold);

/// Utility function to calculate importance based on pixel screen size.
/// This version takes a min/max height and is optimized for volumes.
double ScreenImportance(WhirlyKit::ViewState *viewState,const WhirlyKit::Point2f &frameSize,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,const WhirlyKit::Mbr &nodeMbr, double minZ,double maxZ, const QuadTreeIdentifier &nodeIdent,DisplaySolidRef &dispSold);

}
