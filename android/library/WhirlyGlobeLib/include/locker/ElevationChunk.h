/*
 *  ElevationChunk.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/24/13.
 *  Copyright 2011-2013 mousebird consulting. All rights reserved.
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

@interface WhirlyKitElevationChunk : NSObject

/// Number of elements in X and Y
@property (nonatomic,readonly) int numX,numY;

/// Assign or get the no data value
@property (nonatomic,assign) float noDataValue;

/// Fills in a chunk with random data values.  For testing.
+ (WhirlyKitElevationChunk *)ElevationChunkWithRandomData;

/// Initialize with an NSData full of floats (elevation in meters)
/// SizeX and SizeY are the number of samples in each direction
- (id)initWithFloatData:(NSData *)data sizeX:(int)sizeX sizeY:(int)sizeY;

/// Initialize with shorts of the given size
- (id)initWithShortData:(NSData *)data sizeX:(int)sizeX sizeY:(int)sizeY;

/// Return the elevation at an exact location
- (float)elevationAtX:(int)x y:(int)y;

/// Interpolate an elevation at the given location
- (float)interpolateElevationAtX:(float)x y:(float)y;

@end
