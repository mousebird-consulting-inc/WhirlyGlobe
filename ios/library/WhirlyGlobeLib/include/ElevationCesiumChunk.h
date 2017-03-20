/*
 *  ElevationCesiumChunk.h
 *  WhirlyGlobeLib
 *
 *  Created by @jmnavarro on 6/22/15.
 *  Copyright 2011-2015 mousebird consulting. All rights reserved.
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

#import "ElevationChunk.h"
#import "VectorData.h"
#import <vector>

using namespace std;

/** @details An elevation chunk that understands Cesium's terrainf format.
    This converts Cesium data into a form we can interpolate or display.
  */
@interface WhirlyKitElevationCesiumChunk : NSObject<WhirlyKitElevationChunk>

- (id)initWithCesiumData:(NSData *)data sizeX:(int)sizeX sizeY:(int)sizeY;

/// Tile size in X
@property (nonatomic,readonly) int sizeX;

/// Tile size in Y
@property (nonatomic,readonly) int sizeY;

/// Amount ot scale Z by
@property (nonatomic) float scale;

@property (nonatomic, readonly) WhirlyKit::VectorTrianglesRef mesh;

@property (nonatomic, readonly) vector<unsigned int> &westVertices;
@property (nonatomic, readonly) vector<unsigned int> &southVertices;
@property (nonatomic, readonly) vector<unsigned int> &eastVertices;
@property (nonatomic, readonly) vector<unsigned int> &northVertices;

@property (nonatomic, readonly) vector<WhirlyKit::Point3f> &normals;

@end
