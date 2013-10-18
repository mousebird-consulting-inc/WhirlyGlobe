/*
 *  ScreenImportance.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/11/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import <Foundation/Foundation.h>
#import <math.h>
#import "WhirlyVector.h"
#import "TextureGroup.h"
#import "Scene.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "GlobeMath.h"
#import "sqlhelpers.h"
#import "Quadtree.h"
#import "SceneRendererES.h"


namespace WhirlyKit
{

/// Check if any part of the given tile is on screen
bool TileIsOnScreen(WhirlyKitViewState *viewState,WhirlyKit::Point2f frameSize,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,WhirlyKit::Mbr nodeMbr,WhirlyKit::Quadtree::Identifier &nodeIdent,NSMutableDictionary *attrs);
    
/// Utility function to calculate importance based on pixel screen size.
/// This would be used by the data source as a default.
double ScreenImportance(WhirlyKitViewState *viewState,WhirlyKit::Point2f frameSize,const Point3d &notUsed, int pixelsSqare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,WhirlyKit::Mbr nodeMbr, WhirlyKit::Quadtree::Identifier &nodeIdent,NSMutableDictionary *attrs);

/// Utility function to calculate importance based on pixel screen size.
/// This version takes a min/max height and is optimized for volumes.
double ScreenImportance(WhirlyKitViewState *viewState,WhirlyKit::Point2f frameSize,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,WhirlyKit::Mbr nodeMbr, double minZ,double maxZ, WhirlyKit::Quadtree::Identifier &nodeIdent,NSMutableDictionary *attrs);

}

/// A solid volume used to describe the display space a tile takes up.
/// We use these for screen space calculations and cache them in the tile
///  idents.
@interface WhirlyKitDisplaySolid : NSObject

/// The actual polygons for the side (we are lazy)
@property (nonatomic,assign) std::vector<std::vector<WhirlyKit::Point3d> > &polys;
/// Normals for all 5 or 6 planes
@property (nonatomic,assign) std::vector<Eigen::Vector3d> &normals;
/// Normals for the surface.  We use these to make sure the solid is pointing towards us.
@property (nonatomic,assign) std::vector<Eigen::Vector3d> &surfNormals;

/// Create a display solid, including height.
+ (WhirlyKitDisplaySolid *)displaySolidWithNodeIdent:(WhirlyKit::Quadtree::Identifier &)nodeIdent mbr:(WhirlyKit::Mbr)nodeMbr minZ:(float)minZ maxZ:(float)maxZ srcSystem:(WhirlyKit::CoordSystem *)srcSystem adapter:(WhirlyKit::CoordSystemDisplayAdapter *)coordAdapter;

/// Returns true if the given point (in display space) is inside the volume
- (bool)isInside:(WhirlyKit::Point3d)pt;

/// Calculate the importance for this display solid given the user's eye position
- (double)importanceForViewState:(WhirlyKitViewState *)viewState frameSize:(WhirlyKit::Point2f)frameSize;

/// See if this display solid is current in the viewing frustum
- (bool)isOnScreenForViewState:(WhirlyKitViewState *)viewState frameSize:(WhirlyKit::Point2f)frameSize;

@end
