/*
 *  ElevationChunk.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/24/13.
 *  Copyright 2011-2016 mousebird consulting. All rights reserved.
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

#import <math.h>
#import "WhirlyVector.h"
#import "GlobeMath.h"
#import "Drawable.h"
#import "Texture.h"
#import "Quadtree.h"

namespace WhirlyKit
{
/// Settings needed to turn elevation into drawables
typedef struct
{
    Mbr parentMbr;
    Mbr theMbr;
    int xDim,yDim;
    bool coverPoles;
    bool useTileCenters;
    Point2f texScale,texOffset;
    Point3d dispCenter;
    Eigen::Matrix4d transMat;
    int drawPriority;
    std::vector<WhirlyKit::Texture *> *texs;
    CoordSystemDisplayAdapter *coordAdapter;
    CoordSystem *coordSys;
    Point3d chunkMidDisp;
    bool ignoreEdgeMatching;
    WhirlyKit::Quadtree::Identifier ident;
    int activeTextures;
    int drawOffset;
    float minVis,maxVis;
    bool hasAlpha;
    RGBAColor color;
    SimpleIdentity programId;
    bool includeElev,useElevAsZ;
    bool lineMode;
} ElevationDrawInfo;
    
}

// Note: Porting
///** A protocol for handling elevation data chunks.
// The data itself can be a grid or triangle mesh or what have you.
// The requirement is that you turn it into a set of drawables and interpolate on demand.
// */
//@protocol WhirlyKitElevationChunk<NSObject>
//
///// Return the elevation at an exact location
//- (float)elevationAtX:(int)x y:(int)y;
//
///// Interpolate an elevation at the given location
//- (float)interpolateElevationAtX:(float)x y:(float)y;
//
///// Generate the drawables to represent the elevation
//- (void)generateDrawables:(WhirlyKitElevationDrawInfo *)drawInfo chunk:(BasicDrawable **)draw skirts:(BasicDrawable **)skirtDraw;
//
//@end
//
//
///** Elevation data in grid format.
// These are simple rows and columns of elevation data that can be
// interpolated from or converted into displayable form.
// */
//@interface WhirlyKitElevationGridChunk : NSObject<WhirlyKitElevationChunk>
//
///// Tile size in X
//@property (nonatomic,readonly) int sizeX;
//
///// Tile size in Y
//@property (nonatomic,readonly) int sizeY;
//
///// Assign or get the no data value
//@property (nonatomic,assign) float noDataValue;
//
///// Fills in a chunk with random data values.  For testing.
//+ (WhirlyKitElevationGridChunk *)ElevationChunkWithRandomData;
//
///// Initialize with an NSData full of floats (elevation in meters)
///// SizeX and SizeY are the number of samples in each direction
//- (id)initWithFloatData:(NSData *)data sizeX:(int)sizeX sizeY:(int)sizeY;
//
///// Initialize with shorts of the given size
//- (id)initWithShortData:(NSData *)data sizeX:(int)sizeX sizeY:(int)sizeY;
//
//@end
